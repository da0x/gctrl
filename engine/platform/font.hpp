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
#include <optional>
#include <cstdint>

namespace platform {
namespace font {

    // Font resource IDs (matches Windows resource.h)
    enum resource_id {
        CASCADIA_CODE = 160,
        FONT_AWESOME = 161,
        CONSOLA = 162
    };

    // Get the path to the system UI font
    std::string get_system_ui_font();

    // Get the path to a system monospace font
    std::string get_system_monospace_font();

    // Load font data from embedded resource (Windows) or bundled file (Linux)
    // Returns the font data buffer, or empty optional on failure
    std::optional<std::vector<uint8_t>> load_font_resource(resource_id id);

    // Get font search paths for the platform
    std::vector<std::string> get_font_search_paths();

} // namespace font
} // namespace platform
