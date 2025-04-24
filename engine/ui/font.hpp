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

#include <unordered_map>
#include <string>
#include "imgui.h"
#include <Windows.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

namespace ui {
    namespace font {

        enum class type {
            ui,
            ui_large,
            code,
            symbols,
            console
        };

        inline std::unordered_map<type, ImFont*> fonts;

        void load(type font_type, const char* file_name, float font_size_pixels = 16.0f, bool merge = false, const ImWchar* glyph_ranges = nullptr) {
            ImGuiIO& io = ImGui::GetIO();
            ImFontConfig font_config;
            font_config.OversampleH = 2;
            font_config.OversampleV = 2;
            font_config.PixelSnapH = true;

            if (merge) {
                font_config.MergeMode = true;
            }

            // Define default glyph ranges if not provided (including Unicode characters like π, √, γ, Φ)
            static const ImWchar default_glyph_ranges[] = {
                0x0020, 0x00FF,  // Basic Latin + Latin Supplement
                0x03A0, 0x03C9,  // Greek letters (including Pi and other symbols)
                0x2200, 0x22FF,  // Mathematical Operators (√, ∞, etc.)
                0   // Null-terminated list
            };

            const ImWchar* ranges = glyph_ranges ? glyph_ranges : default_glyph_ranges;

            ImFont* font = io.Fonts->AddFontFromFileTTF(file_name, font_size_pixels, &font_config, ranges);
            if (font && !merge) {
                fonts[font_type] = font;
            }
        }

        ImFont* load_from_resource(type font_type, int resource_id, float font_size_pixels = 16.0f, bool merge = false, const ImWchar* glyph_ranges = nullptr) {

            HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resource_id), RT_FONT);
            if (!hRes) return nullptr;

            HGLOBAL hMem = LoadResource(NULL, hRes);
            if (!hMem) return nullptr;

            void* pFontData = LockResource(hMem);
            DWORD font_size = SizeofResource(NULL, hRes);
            if (!pFontData || font_size == 0) return nullptr;

            void* font_data_copy = malloc(font_size);
            if (!font_data_copy) return nullptr;
            memcpy(font_data_copy, pFontData, font_size);

            ImGuiIO& io = ImGui::GetIO();
            ImFontConfig font_config;
            font_config.OversampleH = 2;
            font_config.OversampleV = 2;
            font_config.PixelSnapH = true;

            if (merge) {
                font_config.MergeMode = true;
            }

            static const ImWchar default_glyph_ranges[] = {
                0x0020, 0x00FF,  // Basic Latin + Latin Supplement
                0x03A0, 0x03C9,  // Greek letters (including Pi and other symbols)
                0x2200, 0x22FF,  // Mathematical Operators (√, ∞, etc.)
                0
            };

            const ImWchar* ranges_to_use = glyph_ranges ? glyph_ranges : default_glyph_ranges;

            ImFont* font = io.Fonts->AddFontFromMemoryTTF(font_data_copy, font_size, font_size_pixels, &font_config, ranges_to_use);
            if (font && !merge) {
                fonts[font_type] = font;
            }

            return font;
        }

        void push(type font_type) {
            if (fonts.find(font_type) != fonts.end()) {
                ImGui::PushFont(fonts[font_type]);
            }
        }

        void pop() {
            ImGui::PopFont();
        }

    } // namespace font
} // namespace ui
