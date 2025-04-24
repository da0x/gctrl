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

#include "network/udp.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <mutex>

#pragma comment(lib, "Ws2_32.lib")

namespace network {
    namespace udp {

        udp_listener::udp_listener(const std::string& address, uint16_t port)
            : stop_(false), socket_fd_(INVALID_SOCKET) {
            initialize_socket(address, port);
        }

        udp_listener::~udp_listener() {
            stop();
        }

        void udp_listener::start(signal_callback on_receive) {
            listener_thread_ = std::thread([this, on_receive]() { listen_for_packets(on_receive); });
        }

        void udp_listener::stop() {
            if (!stop_) {
                stop_ = true;
                closesocket(socket_fd_);
                WSACleanup();
                if (listener_thread_.joinable()) {
                    listener_thread_.join();
                }
            }
        }

        void udp_listener::initialize_socket(const std::string& address, uint16_t port) {
            WSADATA wsa_data;
            if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
                throw std::runtime_error("WSAStartup failed");
            }
            socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
            if (socket_fd_ == INVALID_SOCKET) {
                throw std::runtime_error("Socket creation failed");
            }
            sockaddr_in server_addr{};
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
            server_addr.sin_addr.s_addr = INADDR_ANY;
            if (bind(socket_fd_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == SOCKET_ERROR) {
                throw std::runtime_error("Socket bind failed");
            }
        }

        void udp_listener::listen_for_packets(signal_callback on_receive) {
            stop_ = false;
            while (!stop_) {
                std::vector<uint8_t> buffer(1024);
                sockaddr_in sender_addr{};
                int sender_addr_len = sizeof(sender_addr);
                int received_bytes = recvfrom(socket_fd_, reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()), 0,
                    reinterpret_cast<sockaddr*>(&sender_addr), &sender_addr_len);
                if (received_bytes > 0) {
                    buffer.resize(received_bytes);
                    if (buffer.size() >= sizeof(uint64_t)) {
                        uint64_t id;
                        std::memcpy(&id, buffer.data(), sizeof(uint64_t));
                        on_receive(id, { buffer.begin() + sizeof(uint64_t), buffer.end() });
                    }
                }
            }
        }

        udp_sender::udp_sender(const std::string& address, uint16_t port)
            : address_(address), port_(port), socket_fd_(INVALID_SOCKET) {
            initialize_socket();
        }

        void udp_sender::initialize_socket() {
            WSADATA wsa_data;
            if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
                throw std::runtime_error("WSAStartup failed");
            }
            socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
            if (socket_fd_ == INVALID_SOCKET) {
                throw std::runtime_error("Socket creation failed");
            }
        }

        void udp_sender::send(uint64_t id, const std::vector<uint8_t>& data) {
            std::vector<uint8_t> packet(sizeof(id) + data.size());
            std::memcpy(packet.data(), &id, sizeof(id));
            std::memcpy(packet.data() + sizeof(id), data.data(), data.size());

            sockaddr_in server_addr{};
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port_);
            InetPtonA(AF_INET, address_.c_str(), &server_addr.sin_addr);

            sendto(socket_fd_, reinterpret_cast<const char*>(packet.data()), static_cast<int>(packet.size()), 0,
                reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
        }

        udp_sender::~udp_sender() {
            if (socket_fd_ != INVALID_SOCKET) {
                closesocket(socket_fd_);
                WSACleanup();
            }
        }
    }
}
