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
#include <SDL2/SDL.h>
#include <string>
#include <numbers>
#include <iostream>

namespace ui {
    namespace main_menu {

        inline void render() {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("New")) { /* Handle new action */ }
                    if (ImGui::MenuItem("Open")) { /* Handle open action */ }
                    if (ImGui::MenuItem("Close")) { /* Handle close action */ }
                    if (ImGui::MenuItem("Exit")) { /* Handle exit action */ }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit")) {
                    if (ImGui::MenuItem("Cut")) { /* Handle cut action */ }
                    if (ImGui::MenuItem("Copy")) { /* Handle copy action */ }
                    if (ImGui::MenuItem("Paste")) { /* Handle paste action */ }
                    if (ImGui::MenuItem("Delete")) { /* Handle delete action */ }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
        }

    } // namespace main_menu
} // namespace ui
