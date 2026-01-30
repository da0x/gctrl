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

#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <cstring>
#include "controls/network/udp.hpp"

namespace panel {

    // Per-config panel UDP sender
    struct sender {
        std::string panel_uuid;
        std::string send_ip;
        int send_port;
        std::unique_ptr<network::udp::udp_sender> udp_sender;

        sender() = default;

        ~sender() = default;

        // Non-copyable
        sender(const sender&) = delete;
        sender& operator=(const sender&) = delete;

        void configure(const std::string& uuid, const std::string& ip, int port) {
            panel_uuid = uuid;
            send_ip = ip;
            send_port = port;
            udp_sender = std::make_unique<network::udp::udp_sender>(ip, static_cast<uint16_t>(port));
        }

        void send(uint64_t signal_id, float value) {
            if (!udp_sender) return;

            std::vector<uint8_t> data(sizeof(float));
            std::memcpy(data.data(), &value, sizeof(float));
            udp_sender->send(signal_id, data);
        }

        bool is_configured() const {
            return udp_sender != nullptr;
        }
    };

    // Global manager for all config panel senders
    inline std::map<std::string, std::unique_ptr<sender>> senders;
    inline std::mutex senders_mutex;

    inline void configure_sender(const std::string& uuid, const std::string& ip, int port) {
        std::lock_guard<std::mutex> lock(senders_mutex);

        auto it = senders.find(uuid);
        if (it == senders.end()) {
            senders[uuid] = std::make_unique<sender>();
        }
        senders[uuid]->configure(uuid, ip, port);
    }

    inline void remove_sender(const std::string& uuid) {
        std::lock_guard<std::mutex> lock(senders_mutex);
        senders.erase(uuid);
    }

    inline void remove_all_senders() {
        std::lock_guard<std::mutex> lock(senders_mutex);
        senders.clear();
    }

    inline sender* get_sender(const std::string& uuid) {
        std::lock_guard<std::mutex> lock(senders_mutex);
        auto it = senders.find(uuid);
        if (it != senders.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    inline void send_value(const std::string& uuid, uint64_t signal_id, float value) {
        std::lock_guard<std::mutex> lock(senders_mutex);
        auto it = senders.find(uuid);
        if (it != senders.end() && it->second) {
            it->second->send(signal_id, value);
        }
    }

} // namespace panel
