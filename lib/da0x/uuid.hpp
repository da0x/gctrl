/*

MIT License

Copyright (c) 2024 Daher Alfawares

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#ifndef UUID_HPP
#define UUID_HPP

#include <random>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdint>

namespace uuid {
    namespace v4 {
        // Generate a true version 4 UUID (random-based)
        static std::string generate() {
            std::random_device rd;
            std::mt19937_64 gen(rd());
            std::uniform_int_distribution<uint64_t> dis(0, (uint64_t)-1);

            uint64_t data1 = dis(gen);
            uint64_t data2 = dis(gen);

            // Set the version (4) and variant (RFC 4122)
            data1 = (data1 & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL; // Version 4
            data2 = (data2 & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL; // Variant 1

            std::ostringstream oss;
            oss << std::hex << std::setfill('0')
                << std::setw(8) << (data1 >> 32) << '-'
                << std::setw(4) << ((data1 >> 16) & 0xFFFF) << '-'
                << std::setw(4) << (data1 & 0xFFFF) << '-'
                << std::setw(4) << (data2 >> 48) << '-'
                << std::setw(12) << (data2 & 0xFFFFFFFFFFFFULL);

            return oss.str();
        }
    }

    // Function to convert a UUID string into a uint64_t by taking half of the UUID (lower 64 bits)
    static uint64_t uuid_to_uint64(const std::string& uuid_str) {
        // Extract the last 12 characters from the UUID string
        std::string last_part = uuid_str.substr(uuid_str.length() - 12);

        // Convert the last part of the UUID string to uint64_t
        return std::stoull(last_part, nullptr, 16);
    }
}

#endif // UUID_HPP
