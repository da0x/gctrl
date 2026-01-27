//
// This file is part of gctrl.
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

#include <cmath>
#include <numbers>
#include <random>
#include <chrono>
#include <thread>
#include <map>
#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>
#include <string>
#include <complex>
#include <algorithm>


namespace gctrl {
    constexpr float e = static_cast<float>(std::numbers::e);
    constexpr float log2e = static_cast<float>(std::numbers::log2e);
    constexpr float log10e = static_cast<float>(std::numbers::log10e);
    constexpr float pi = static_cast<float>(std::numbers::pi);
    constexpr float inv_pi = static_cast<float>(std::numbers::inv_pi);
    constexpr float inv_sqrtpi = static_cast<float>(std::numbers::inv_sqrtpi);
    constexpr float ln2 = static_cast<float>(std::numbers::ln2);
    constexpr float ln10 = static_cast<float>(std::numbers::ln10);
    constexpr float sqrt2 = static_cast<float>(std::numbers::sqrt2);
    constexpr float sqrt3 = static_cast<float>(std::numbers::sqrt3);
    constexpr float inv_sqrt3 = static_cast<float>(std::numbers::inv_sqrt3);
    constexpr float egamma = static_cast<float>(std::numbers::egamma);
    constexpr float phi = static_cast<float>(std::numbers::phi);

    typedef std::complex<float> complex;
    using std::clamp;

    class timestep {
    public:
        timestep() : last_time_(std::chrono::steady_clock::now()), delta_seconds_(0.0f) {}

        void update(std::chrono::steady_clock::time_point now) {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - last_time_);
            delta_seconds_ = duration.count() / 1'000'000.0f;
        }

        void reset() {
            last_time_ = std::chrono::steady_clock::now();
            delta_seconds_ = 0.0;
        }

        float seconds() const { return delta_seconds_; }
        float milliseconds() const { return delta_seconds_ * 1000.0f; }
        float microseconds() const { return delta_seconds_ * 1'000'000.0f; }

    private:
        std::chrono::steady_clock::time_point last_time_;
        float delta_seconds_;
    };

    class timer {
    public:
        timer() : now_(std::chrono::steady_clock::now()) {}

        void update() {
            now_ = std::chrono::steady_clock::now();
            timestep_1hz.update(now_);
            timestep_10hz.update(now_);
            timestep_100hz.update(now_);
            timestep_1khz.update(now_);
            timestep_10khz.update(now_);
        }

        timestep timestep_1hz;
        timestep timestep_10hz;
        timestep timestep_100hz;
        timestep timestep_1khz;
        timestep timestep_10khz;

    private:
        std::chrono::steady_clock::time_point now_;
    };
}

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

namespace gctrl {

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

        class listener {
        public:
            listener(const std::string& address, uint16_t port, signal_registry& registry);
            ~listener();
            void start();
            void stop();

        private:
            signal_registry& registry_;
            SOCKET socket_fd_;
            bool stop_;
            std::thread listener_thread_;
            void initialize_socket(const std::string& address, uint16_t port);
            void listen_for_packets();
            void close_socket();
        };

        class sender {
        public:
            sender(const std::string& address, uint16_t port);
            ~sender();
            void send(uint64_t id, const std::vector<uint8_t>& data);

        private:
            std::string address_;
            uint16_t port_;
            SOCKET socket_fd_;
            void initialize_socket();
            void close_socket();
        };

    }

    namespace binary {

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
} // namespace gctrl

#include <csignal>
#include <atomic>
#include <iostream>

namespace gctrl {
    inline std::atomic<bool> terminate_requested = false;

    inline void handle_signal(int signal) {
        if (signal == SIGTERM || signal == SIGINT) {
            terminate_requested = true;
            std::cerr << "Signal received. Terminating process..." << std::endl;
        }
    }
}
