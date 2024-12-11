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
#include <vector>

namespace exec {
    using std::string;
    using std::vector;

    enum class frequency {
        _1_hz = 1,
        _10_hz = 10,
        _100_hz = 100,
        _1_khz = 1000,
        _10_khz = 10000
    };

    inline vector<frequency> frequencies = {
        frequency::_1_hz,
        frequency::_10_hz,
        frequency::_100_hz,
        frequency::_1_khz,
        frequency::_10_khz
    };

    inline string frequency_to_string(frequency freq) {
        switch (freq) {
        case frequency::_1_hz: return "1 Hz";
        case frequency::_10_hz: return "10 Hz";
        case frequency::_100_hz: return "100 Hz";
        case frequency::_1_khz: return "1 kHz";
        case frequency::_10_khz: return "10 kHz";
        default: return "Unknown Frequency";
        }
    }

    std::map<int, string> frequency_map = {
        {static_cast<int>(frequency::_1_hz), "1 Hz"},
        {static_cast<int>(frequency::_10_hz), "10 Hz"},
        {static_cast<int>(frequency::_100_hz), "100 Hz"},
        {static_cast<int>(frequency::_1_khz), "1 kHz"},
        {static_cast<int>(frequency::_10_khz), "10 kHz"}
    };

    namespace code {
        const string declare(frequency freq) {
            switch (freq) {
            case frequency::_1_hz:
                return "at_1hz(float timestep)";
            case frequency::_10_hz:
                return "at_10hz(float timestep)";
            case frequency::_100_hz:
                return "at_100hz(float timestep)";
            case frequency::_1_khz:
                return "at_1khz(float timestep)";
            case frequency::_10_khz:
                return "at_10khz(float timestep)";
            }
            return "unknown_frequency";
        }

        constexpr const char* call(frequency freq) {
            switch (freq) {
            case frequency::_1_hz:
                return "at_1hz(timestep)";
            case frequency::_10_hz:
                return "at_10hz(timestep)";
            case frequency::_100_hz:
                return "at_100hz(timestep)";
            case frequency::_1_khz:
                return "at_1khz(timestep)";
            case frequency::_10_khz:
                return "at_10khz(timestep)";
            }
            return "unknown_frequency";
        }
    }
}
