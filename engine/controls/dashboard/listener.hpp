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
#include <set>
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

    // Shared UDP listener for one ip:port endpoint.
    // Multiple dashboards can subscribe to the same endpoint.
    struct shared_listener {
        std::map<uint64_t, std::vector<float>> signal_data;
        std::mutex data_mutex;
        std::atomic<bool> keep_listening{false};
        std::thread thread;
        std::string listen_ip;
        int listen_port = 0;
        int ref_count = 0;

        shared_listener() = default;

        ~shared_listener() {
            stop();
        }

        // Non-copyable
        shared_listener(const shared_listener&) = delete;
        shared_listener& operator=(const shared_listener&) = delete;

        void add_value(uint64_t id, float value) {
            std::lock_guard<std::mutex> lock(data_mutex);
            auto& values = signal_data[id];
            if (values.size() > 1000) {
                values.erase(values.begin());
            }
            values.push_back(value);
        }

        void start(const std::string& ip, int port) {
            if (keep_listening.load()) {
                return; // Already running
            }

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

    // Shared listeners keyed by "ip:port"
    inline std::map<std::string, std::unique_ptr<shared_listener>> shared_listeners;
    // Dashboard UUID -> "ip:port" subscription
    inline std::map<std::string, std::string> subscriptions;
    inline std::mutex listeners_mutex;

    inline std::string endpoint_key(const std::string& ip, int port) {
        return ip + ":" + std::to_string(port);
    }

    inline void start_listener(const std::string& uuid, const std::string& ip, int port) {
        std::lock_guard<std::mutex> lock(listeners_mutex);
        std::string key = endpoint_key(ip, port);

        // Check if already subscribed to this endpoint
        auto sub_it = subscriptions.find(uuid);
        if (sub_it != subscriptions.end()) {
            if (sub_it->second == key) {
                // Already subscribed, just ensure it's running
                auto& sl = shared_listeners[key];
                if (sl && !sl->keep_listening.load()) {
                    sl->start(ip, port);
                }
                return;
            }
            // Different endpoint - unsubscribe from old
            auto old_it = shared_listeners.find(sub_it->second);
            if (old_it != shared_listeners.end()) {
                old_it->second->ref_count--;
                if (old_it->second->ref_count <= 0) {
                    old_it->second->stop();
                    shared_listeners.erase(old_it);
                }
            }
        }

        // Create shared listener if needed
        auto& sl = shared_listeners[key];
        if (!sl) {
            sl = std::make_unique<shared_listener>();
        }

        // Start if not running
        if (!sl->keep_listening.load()) {
            sl->start(ip, port);
        }

        // Subscribe
        subscriptions[uuid] = key;
        sl->ref_count++;
    }

    // Pause listener but retain data (for machine stop)
    inline void pause_listener(const std::string& uuid) {
        std::lock_guard<std::mutex> lock(listeners_mutex);
        auto sub_it = subscriptions.find(uuid);
        if (sub_it == subscriptions.end()) return;

        auto sl_it = shared_listeners.find(sub_it->second);
        if (sl_it != shared_listeners.end()) {
            sl_it->second->stop(); // Stop thread, keep data and object
        }
        // Don't erase subscription - keeps link for retained data access
    }

    // Fully stop and remove listener (for disconnect/delete)
    inline void stop_listener(const std::string& uuid) {
        std::lock_guard<std::mutex> lock(listeners_mutex);
        auto sub_it = subscriptions.find(uuid);
        if (sub_it == subscriptions.end()) return;

        std::string key = sub_it->second;
        auto sl_it = shared_listeners.find(key);
        if (sl_it != shared_listeners.end()) {
            sl_it->second->ref_count--;
            if (sl_it->second->ref_count <= 0) {
                sl_it->second->stop();
                shared_listeners.erase(sl_it);
            }
        }
        subscriptions.erase(sub_it);
    }

    inline void stop_all_listeners() {
        std::lock_guard<std::mutex> lock(listeners_mutex);
        for (auto& [key, sl] : shared_listeners) {
            sl->stop();
        }
        shared_listeners.clear();
        subscriptions.clear();
    }

    inline shared_listener* get_listener(const std::string& uuid) {
        std::lock_guard<std::mutex> lock(listeners_mutex);
        auto sub_it = subscriptions.find(uuid);
        if (sub_it == subscriptions.end()) return nullptr;

        auto sl_it = shared_listeners.find(sub_it->second);
        if (sl_it != shared_listeners.end()) {
            return sl_it->second.get();
        }
        return nullptr;
    }

} // namespace dashboard
