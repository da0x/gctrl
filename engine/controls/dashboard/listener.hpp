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
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include "controls/network/udp.hpp"

namespace dashboard {

    // Per-dashboard UDP listener
    struct listener {
        std::string dashboard_uuid;
        std::map<uint64_t, std::vector<float>> signal_data;
        std::mutex data_mutex;
        std::atomic<bool> keep_listening{false};
        std::thread thread;
        std::string listen_ip;
        int listen_port;

        listener() = default;

        ~listener() {
            stop();
        }

        // Non-copyable
        listener(const listener&) = delete;
        listener& operator=(const listener&) = delete;

        // Movable
        listener(listener&& other) noexcept
            : dashboard_uuid(std::move(other.dashboard_uuid))
            , signal_data(std::move(other.signal_data))
            , keep_listening(other.keep_listening.load())
            , thread(std::move(other.thread))
            , listen_ip(std::move(other.listen_ip))
            , listen_port(other.listen_port) {
            other.keep_listening = false;
        }

        void add_value(uint64_t id, float value) {
            std::lock_guard<std::mutex> lock(data_mutex);
            auto& values = signal_data[id];
            if (values.size() > 1000) {
                values.erase(values.begin());
            }
            values.push_back(value);
        }

        void start(const std::string& uuid, const std::string& ip, int port) {
            if (keep_listening.load()) {
                return; // Already running
            }

            dashboard_uuid = uuid;
            listen_ip = ip;
            listen_port = port;
            keep_listening.store(true);

            thread = std::thread([this]() {
                try {
                    network::udp::udp_listener udp_listener(listen_ip, static_cast<uint16_t>(listen_port));

                    udp_listener.start([this](uint64_t id, const std::vector<uint8_t>& data) {
                        if (data.size() < sizeof(float)) {
                            return;
                        }

                        float value;
                        std::memcpy(&value, data.data(), sizeof(float));
                        add_value(id, value);
                    });

                    while (keep_listening.load()) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }

                    udp_listener.stop();
                } catch (const std::exception& e) {
                    std::cerr << "Dashboard listener failed: " << e.what() << "\n";
                }
            });
        }

        void stop() {
            keep_listening.store(false);
            if (thread.joinable()) {
                thread.join();
            }
        }

        void clear_data() {
            std::lock_guard<std::mutex> lock(data_mutex);
            signal_data.clear();
        }

        std::map<uint64_t, std::vector<float>> get_data() {
            std::lock_guard<std::mutex> lock(data_mutex);
            return signal_data;
        }
    };

    // Global manager for all dashboard listeners
    inline std::map<std::string, std::unique_ptr<listener>> listeners;
    inline std::mutex listeners_mutex;

    inline void start_listener(const std::string& uuid, const std::string& ip, int port) {
        std::lock_guard<std::mutex> lock(listeners_mutex);

        auto it = listeners.find(uuid);
        if (it != listeners.end()) {
            // Reuse existing listener (preserves data), just restart if not running
            if (!it->second->keep_listening.load()) {
                it->second->start(uuid, ip, port);
            }
            return;
        }

        // Create and start new listener
        auto new_listener = std::make_unique<listener>();
        new_listener->start(uuid, ip, port);
        listeners[uuid] = std::move(new_listener);
    }

    // Pause listener but retain data (for machine stop)
    inline void pause_listener(const std::string& uuid) {
        std::lock_guard<std::mutex> lock(listeners_mutex);
        auto it = listeners.find(uuid);
        if (it != listeners.end()) {
            it->second->stop();
            // Don't erase - keeps data
        }
    }

    // Fully stop and remove listener (for disconnect/delete)
    inline void stop_listener(const std::string& uuid) {
        std::lock_guard<std::mutex> lock(listeners_mutex);
        auto it = listeners.find(uuid);
        if (it != listeners.end()) {
            it->second->stop();
            listeners.erase(it);
        }
    }

    inline void stop_all_listeners() {
        std::lock_guard<std::mutex> lock(listeners_mutex);
        for (auto& [uuid, listener_ptr] : listeners) {
            listener_ptr->stop();
        }
        listeners.clear();
    }

    inline listener* get_listener(const std::string& uuid) {
        std::lock_guard<std::mutex> lock(listeners_mutex);
        auto it = listeners.find(uuid);
        if (it != listeners.end()) {
            return it->second.get();
        }
        return nullptr;
    }

} // namespace dashboard
