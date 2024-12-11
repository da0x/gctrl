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

#include "imgui.h"

namespace ui {
    namespace theme {
        namespace vs2022 {
            constexpr ImVec4 picker_to_vec4(uint32_t hex_color, uint32_t hex_alpha = 0xFF) {
                float red = ((hex_color >> 16) & 0xFF) / 255.0f;
                float green = ((hex_color >> 8) & 0xFF) / 255.0f;
                float blue = (hex_color & 0xFF) / 255.0f;
                float alpha = (hex_alpha / 255.0f);
                return ImVec4(red, green, blue, alpha);
            }

            constexpr ImVec4 background = ImVec4(0.13f, 0.13f, 0.14f, 1.00f);
            constexpr ImVec4 frame_background = ImVec4(0.25f, 0.25f, 0.27f, 0.75f);
            constexpr ImVec4 frame_background_hovered = ImVec4(0.30f, 0.30f, 0.33f, 1.00f);
            constexpr ImVec4 frame_background_active = ImVec4(0.41f, 0.41f, 0.43f, 1.00f);
            constexpr ImVec4 border = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
            constexpr ImVec4 button = ImVec4(0.25f, 0.25f, 0.28f, 1.00f);
            constexpr ImVec4 button_hovered = ImVec4(0.37f, 0.37f, 0.39f, 1.00f);
            constexpr ImVec4 button_active = ImVec4(0.45f, 0.45f, 0.47f, 1.00f);
            constexpr ImVec4 text = ImVec4(0.86f, 0.93f, 0.89f, 1.00f);
            constexpr ImVec4 text_disabled = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
            constexpr ImVec4 bright_yellow = ImVec4(0.94f, 0.89f, 0.38f, 1.00f);
            constexpr ImVec4 bright_cyan = ImVec4(0.33f, 0.84f, 0.89f, 1.00f);
            constexpr ImVec4 bright_purple = ImVec4(0.69f, 0.40f, 0.89f, 1.00f);
            constexpr ImVec4 bright_blue = ImVec4(0.39f, 0.62f, 0.97f, 1.00f);
            constexpr ImVec4 olive_oil = ImVec4(0.663f, 0.545f, 0.176f, 1.0f);
            constexpr ImVec4 olive_oil_dark = ImVec4(olive_oil.x * 0.5f, olive_oil.y * 0.7f, olive_oil.z * 0.4f, olive_oil.w);
            constexpr ImVec4 dark_yellow = ImVec4(0.8f, 0.796f, 0.58f, 1.0f);
            constexpr ImVec4 bright_teal = ImVec4(0.27f, 0.81f, 0.74f, 1.00f);
            constexpr ImVec4 code_type = ImVec4(0.85f, 0.58f, 0.27f, 1.00f);
            constexpr ImVec4 code_keyword = picker_to_vec4(0x3981cb);
            constexpr ImVec4 code_instance = picker_to_vec4(0xfbffc5);
            constexpr ImVec4 code_typename = picker_to_vec4(0x4ec9b0);
            constexpr ImVec4 code_number = picker_to_vec4(0xa9cda6);
            constexpr ImVec4 highlight = ImVec4(0.29f, 0.59f, 0.82f, 1.00f);
            constexpr ImVec4 symbol = ImVec4(0.85f, 0.27f, 0.27f, 1.00f);
            constexpr ImVec4 element_background = picker_to_vec4(0x3187CE, 255 / 3);
            constexpr ImVec4 plug_background = picker_to_vec4(0xCE3187, 255 / 3);
            constexpr ImVec4 socket_background = picker_to_vec4(0x87CE31, 255 / 3);

            void apply() {
                ImGuiStyle& style = ImGui::GetStyle();
                ImVec4* colors = style.Colors;

                colors[ImGuiCol_Text] = text;
                colors[ImGuiCol_TextDisabled] = text_disabled;
                colors[ImGuiCol_WindowBg] = background;
                colors[ImGuiCol_ChildBg] = background;
                colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.92f);
                colors[ImGuiCol_Border] = border;
                colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                colors[ImGuiCol_FrameBg] = frame_background;
                colors[ImGuiCol_FrameBgHovered] = frame_background_hovered;
                colors[ImGuiCol_FrameBgActive] = frame_background_active;
                colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
                colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
                colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
                colors[ImGuiCol_MenuBarBg] = background;
                colors[ImGuiCol_ScrollbarBg] = background;
                colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
                colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
                colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
                colors[ImGuiCol_CheckMark] = highlight;
                colors[ImGuiCol_SliderGrab] = highlight;
                colors[ImGuiCol_SliderGrabActive] = ImVec4(0.39f, 0.70f, 0.90f, 1.00f);
                colors[ImGuiCol_Button] = button;
                colors[ImGuiCol_ButtonHovered] = button_hovered;
                colors[ImGuiCol_ButtonActive] = button_active;
                colors[ImGuiCol_Header] = frame_background_hovered;
                colors[ImGuiCol_HeaderHovered] = ImVec4(0.34f, 0.34f, 0.37f, 1.00f);
                colors[ImGuiCol_HeaderActive] = frame_background_active;
                colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
                colors[ImGuiCol_SeparatorHovered] = ImVec4(0.47f, 0.47f, 0.54f, 0.78f);
                colors[ImGuiCol_SeparatorActive] = ImVec4(0.56f, 0.56f, 0.61f, 1.00f);
                colors[ImGuiCol_ResizeGrip] = ImVec4(0.29f, 0.59f, 0.82f, 0.20f);
                colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.59f, 0.82f, 0.67f);
                colors[ImGuiCol_ResizeGripActive] = ImVec4(0.29f, 0.59f, 0.82f, 0.95f);
                colors[ImGuiCol_Tab] = frame_background;
                colors[ImGuiCol_TabHovered] = frame_background_hovered;
                colors[ImGuiCol_TabActive] = frame_background_hovered;
                colors[ImGuiCol_TabUnfocused] = frame_background;
                colors[ImGuiCol_TabUnfocusedActive] = frame_background_hovered;
                colors[ImGuiCol_DockingPreview] = ImVec4(0.29f, 0.59f, 0.82f, 0.70f);
                colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
                colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.90f, 0.45f, 0.00f, 1.00f);
                colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.77f, 0.00f, 1.00f);
                colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
                colors[ImGuiCol_TextSelectedBg] = ImVec4(0.29f, 0.59f, 0.82f, 0.35f);
                colors[ImGuiCol_DragDropTarget] = ImVec4(0.29f, 0.59f, 0.82f, 0.90f);
                colors[ImGuiCol_NavHighlight] = highlight;
                colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
                colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
                colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

                style.FrameRounding = 4.0f;
                style.GrabRounding = 4.0f;
                style.WindowRounding = 4.0f;
                style.ScrollbarRounding = 4.0f;
                style.TabRounding = 4.0f;
            }
        }
    }
}
