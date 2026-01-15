//
// General Controls Engine
//
// Copyright (C) 2024 Daher Alfawares
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//
// Contact Information: www.gctrl.org
//

#pragma once

#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include "controls/network/udp.hpp"
#include <thread>
#include <mutex>
#include <atomic>

namespace controls {
    namespace debug {
        struct State {
            std::map<uint64_t, std::vector<float>> signal_data;
            std::mutex data_mutex;
            std::atomic<bool> keep_listening{ true };
            std::thread listener;
        };

        inline State& get_state() {
            static State state;
            return state;
        }

        inline void add_signal_value(uint64_t id, float value) {
            auto& state = get_state();
            std::lock_guard<std::mutex> lock(state.data_mutex);
            if (state.signal_data[id].size() > 1000) {
                state.signal_data[id].erase(state.signal_data[id].begin());
            }
            state.signal_data[id].push_back(value);
        }

        inline void listener_thread() {
            auto& state = get_state();
            try {
                network::udp::udp_listener listener("127.0.0.1", 8080);

                listener.start([](uint64_t id, const std::vector<uint8_t>& data) {
                    if (data.size() < sizeof(float)) {
                        std::cerr << "Received data is too small: " << data.size() << " bytes\n";
                        return;
                    }

                    float value;
                    std::memcpy(&value, data.data(), sizeof(float));

                    std::cerr << "Received Signal ID: " << id << ", Value: " << value << "\n";

                    add_signal_value(id, value);
                });

                while (state.keep_listening.load()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                listener.stop();
            } catch (const std::exception& e) {
                std::cerr << "Debug listener failed: " << e.what() << "\n";
            }
        }

        inline void begin() {
            auto& state = get_state();
            if (state.listener.joinable()) {
                throw std::runtime_error("Listener thread already running!");
            }
            state.keep_listening.store(true);
            state.listener = std::thread(listener_thread);
        }

        inline void end() {
            auto& state = get_state();
            state.keep_listening.store(false);
            if (state.listener.joinable()) {
                state.listener.join();
            }
        }
    } // namespace debug
} // namespace controls
