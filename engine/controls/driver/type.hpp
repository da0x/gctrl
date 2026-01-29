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
#include "ui/icons.hpp"

namespace driver {

    enum class type {
        udp,    // Machine-to-machine and machine-to-GCtrl communication
        config  // Configuration file read/write, state persistence
    };

    inline std::string type_to_string(type t) {
        switch (t) {
            case type::udp:    return "udp";
            case type::config: return "config";
            default:           return "udp";
        }
    }

    inline type string_to_type(const std::string& s) {
        if (s == "config") return type::config;
        return type::udp;  // default to udp for backward compatibility
    }

    inline std::string type_to_description(type t) {
        switch (t) {
            case type::udp:    return "UDP communication driver for machine-to-machine and machine-to-GCtrl data exchange";
            case type::config: return "Configuration file driver for persistent settings and state";
            default:           return "";
        }
    }

    inline const char* type_to_icon(type t) {
        switch (t) {
            case type::udp:    return ICON_FA_NETWORK_WIRED;
            case type::config: return ICON_FA_FILE_LINES;
            default:           return ui::icon::driver;
        }
    }

    inline std::string type_to_display_name(type t) {
        switch (t) {
            case type::udp:    return "UDP";
            case type::config: return "Config";
            default:           return "UDP";
        }
    }

} // namespace driver
