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
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <array>
#include "ui/ui.hpp"

namespace ui {

    // Helper function to convert ImVec4 to JSON
    nlohmann::json color_to_json(const ImVec4& color) {
        return { color.x, color.y, color.z, color.w };
    }

    // Helper function to convert JSON to ImVec4
    ImVec4 json_to_color(const nlohmann::json& j) {
        return ImVec4(j[0], j[1], j[2], j[3]);
    }

    // Nord color palette definition (RGB without alpha)
    const std::array<const char*, 8> nord_color_names = {
        "Polar Night 1", "Polar Night 2", "Polar Night 3", "Polar Night 4",
        "Snow Storm 1", "Snow Storm 2", "Aurora Green", "Aurora Red"
    };

    const std::array<ImVec4, 8> nord_colors = {
        ImVec4(0.18f, 0.20f, 0.25f, 1.0f),  // Polar Night 1
        ImVec4(0.19f, 0.23f, 0.29f, 1.0f),  // Polar Night 2
        ImVec4(0.21f, 0.27f, 0.31f, 1.0f),  // Polar Night 3
        ImVec4(0.23f, 0.29f, 0.33f, 1.0f),  // Polar Night 4
        ImVec4(0.92f, 0.94f, 0.95f, 1.0f),  // Snow Storm 1
        ImVec4(0.90f, 0.91f, 0.93f, 1.0f),  // Snow Storm 2
        ImVec4(0.67f, 0.84f, 0.47f, 1.0f),  // Aurora Green
        ImVec4(0.90f, 0.44f, 0.44f, 1.0f)   // Aurora Red
    };

    class ThemeEditor {
    public:
        bool show_window = true;  // Toggle to show/hide the editor

        // Render the theme editor window
        void render(bool dark_mode) {
            if (!show_window) return;

            if (ImGui::Begin("Theme Editor", &show_window, ImGuiWindowFlags_MenuBar)) {
                ImGuiStyle& style = ImGui::GetStyle();
                ImVec4* colors = style.Colors;

                // Menu Bar
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("File")) {
                        if (ImGui::MenuItem("Save Theme to JSON")) {
                            save_theme_to_json(dark_mode ? "theme_dark.json" : "theme_light.json");
                        }
                        if (ImGui::MenuItem("Load Theme from JSON")) {
                            load_theme_from_json(dark_mode ? "theme_dark.json" : "theme_light.json");
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Editable color controls with Nord color dropdowns
                for (int i = 0; i < ImGuiCol_COUNT; ++i) {
                    ImGui::PushID(i);  // Ensure unique ID for each color item

                    // Layout: Color square, color name, dropdown with colors and alpha slider
                    ImGui::Columns(4, nullptr, false);  // 4 columns: color square, name, dropdown, alpha slider

                    // 1. Display color square
                    ImGui::ColorButton("##color", colors[i], ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoAlpha, ImVec2(20, 20));
                    ImGui::NextColumn();

                    // 2. Display the name of the color
                    ImGui::Text("%s", ImGui::GetStyleColorName(i));
                    ImGui::NextColumn();

                    // 3. Dropdown for Nord colors
                    static int selected_color_index[ImGuiCol_COUNT] = { 0 };  // Store selected index for each color
                    if (ImGui::BeginCombo("##nord_combo", nord_color_names[selected_color_index[i]])) {
                        for (int n = 0; n < nord_color_names.size(); ++n) {
                            ImGui::PushID(n);
                            // Display a small color square next to the Nord color name in the dropdown
                            ImGui::ColorButton("##color_square", nord_colors[n], ImGuiColorEditFlags_NoAlpha, ImVec2(10, 10));
                            ImGui::SameLine();
                            bool is_selected = (selected_color_index[i] == n);
                            if (ImGui::Selectable(nord_color_names[n], is_selected)) {
                                selected_color_index[i] = n;
                                colors[i] = nord_colors[n];  // Set the selected Nord color
                            }
                            if (is_selected) ImGui::SetItemDefaultFocus();
                            ImGui::PopID();
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::NextColumn();

                    // 4. Alpha slider
                    ImGui::SliderFloat("##alpha", &colors[i].w, 0.0f, 1.0f, "Alpha: %.2f");
                    ImGui::NextColumn();

                    ImGui::PopID();  // Restore ID stack
                }

                ImGui::Columns(1);  // End the column layout

                ImGui::End();
            }
        }

        // Save the current theme to a JSON file
        void save_theme_to_json(const std::string& filename) {
            ImGuiStyle& style = ImGui::GetStyle();
            ImVec4* colors = style.Colors;

            nlohmann::json json_theme;
            for (int i = 0; i < ImGuiCol_COUNT; ++i) {
                json_theme[ImGui::GetStyleColorName(i)] = color_to_json(colors[i]);
            }

            std::ofstream file(filename);
            if (file.is_open()) {
                file << json_theme.dump(4);
                std::cout << "Theme saved to " << filename << std::endl;
            }
        }

        // Load a theme from a JSON file
        void load_theme_from_json(const std::string& filename) {
            ImGuiStyle& style = ImGui::GetStyle();
            ImVec4* colors = style.Colors;

            std::ifstream file(filename);
            if (file.is_open()) {
                nlohmann::json json_theme;
                file >> json_theme;

                for (int i = 0; i < ImGuiCol_COUNT; ++i) {
                    const char* color_name = ImGui::GetStyleColorName(i);
                    if (json_theme.contains(color_name)) {
                        colors[i] = json_to_color(json_theme[color_name]);
                    }
                }
                std::cout << "Theme loaded from " << filename << std::endl;
            }
        }
    };

    // Global instance of the theme editor
    ThemeEditor g_ThemeEditor;

    // Call this in your main application loop to render the editor
    void render_theme_editor(bool dark_mode) {
        g_ThemeEditor.render(dark_mode);
    }
}
