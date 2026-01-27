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

#include "platform/font.hpp"
#include <Windows.h>

namespace platform {
namespace font {

    std::string get_system_ui_font() {
        return "C:\\Windows\\Fonts\\segoeui.ttf";
    }

    std::string get_system_monospace_font() {
        return "C:\\Windows\\Fonts\\consola.ttf";
    }

    std::optional<std::vector<uint8_t>> load_font_resource(resource_id id) {
        HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(id), RT_FONT);
        if (!hRes) return std::nullopt;

        HGLOBAL hMem = LoadResource(NULL, hRes);
        if (!hMem) return std::nullopt;

        void* pFontData = LockResource(hMem);
        DWORD font_size = SizeofResource(NULL, hRes);
        if (!pFontData || font_size == 0) return std::nullopt;

        std::vector<uint8_t> data(font_size);
        memcpy(data.data(), pFontData, font_size);
        return data;
    }

    std::vector<std::string> get_font_search_paths() {
        return {
            "C:\\Windows\\Fonts"
        };
    }

} // namespace font
} // namespace platform
