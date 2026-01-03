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

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

namespace network {
    namespace udp {

        class signal_registry {
        public:
            using signal_callback = std::function<void(const std::vector<uint8_t>&)>;
            void subscribe(uint64_t id, signal_callback callback);
            void notify(uint64_t id, const std::vector<uint8_t>& data);

        private:
            std::unordered_map<uint64_t, signal_callback> registry_;
            std::mutex mutex_;
        };

        class udp_listener {
        public:
            using signal_callback = std::function<void(uint64_t id, const std::vector<uint8_t>&)>;

            udp_listener(const std::string& address, uint16_t port);
            ~udp_listener();

            void start(signal_callback on_receive);
            void stop();

        private:
            SOCKET socket_fd_;
            bool stop_ = false;
            std::thread listener_thread_;
            void initialize_socket(const std::string& address, uint16_t port);
            void listen_for_packets(signal_callback on_receive);
            void close_socket();
        };

        class udp_sender {
        public:
            udp_sender(const std::string& address, uint16_t port);
            ~udp_sender();

            void send(uint64_t id, const std::vector<uint8_t>& data);

        private:
            std::string address_;
            uint16_t port_;
            SOCKET socket_fd_;
            void initialize_socket();
            void close_socket();
        };

        template <typename T>
        std::vector<uint8_t> serialize(const T& value) {
            std::vector<uint8_t> buffer(sizeof(T));
            std::memcpy(buffer.data(), &value, sizeof(T));
            return buffer;
        }

        template <typename T>
        T deserialize(const std::vector<uint8_t>& buffer) {
            T value;
            std::memcpy(&value, buffer.data(), sizeof(T));
            return value;
        }
    }
}

namespace debug {

    inline void send(uint64_t id, float value) {
        static network::udp::udp_sender sender("127.0.0.1", 8081);

        // Serialize the float value
        auto serialized_data = network::udp::serialize(value);

        // Send the data with the specified signal ID
        sender.send(id, serialized_data);
    }

}
