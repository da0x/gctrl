#include "gctrl.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdexcept>
#include <cstring>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

namespace gctrl {
    namespace udp {
        void signal_registry::subscribe(uint64_t id, signal_callback callback) {
            std::lock_guard<std::mutex> lock(mutex_);
            registry_[id] = callback;
        }

        void signal_registry::notify(uint64_t id, const std::vector<uint8_t>& data) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (registry_.count(id)) {
                registry_[id](data);
            }
        }

        listener::listener(const std::string& address, uint16_t port, signal_registry& registry)
            : registry_(registry), stop_(false), socket_fd_(INVALID_SOCKET) {
            initialize_socket(address, port);
        }

        listener::~listener() {
            stop();
        }

        void listener::start() {
            listener_thread_ = std::thread([this]() { listen_for_packets(); });
        }

        void listener::stop() {
            if (!stop_) {
                stop_ = true;
                closesocket(socket_fd_);
                WSACleanup();
                if (listener_thread_.joinable()) {
                    listener_thread_.join();
                }
            }
        }

        void listener::initialize_socket(const std::string& address, uint16_t port) {
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

        void listener::listen_for_packets() {
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
                        registry_.notify(id, { buffer.begin() + sizeof(uint64_t), buffer.end() });
                    }
                }
            }
        }

        sender::sender(const std::string& address, uint16_t port)
            : address_(address), port_(port), socket_fd_(INVALID_SOCKET) {
            initialize_socket();
        }

        void sender::initialize_socket() {
            WSADATA wsa_data;
            if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
                throw std::runtime_error("WSAStartup failed");
            }
            socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
            if (socket_fd_ == INVALID_SOCKET) {
                throw std::runtime_error("Socket creation failed");
            }
        }

        void sender::send(uint64_t id, const std::vector<uint8_t>& data) {
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

        sender::~sender() {
            if (socket_fd_ != INVALID_SOCKET) {
                closesocket(socket_fd_);
                WSACleanup();
            }
        }
    }
}