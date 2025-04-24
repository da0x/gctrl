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

#include "ui/ui.hpp"
#include <numbers>

namespace ui {
    namespace connector {

        enum class shape {
            triangle, square, circle
        };

        void triangle(ui::direction dir, bool is_connected) {
            float size = 16.0f;
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            ImVec2 p1, p2, p3;

            if (dir == ui::direction::ltr) {
                p1 = ImVec2(pos.x, pos.y);                      // Left point
                p2 = ImVec2(pos.x + size, pos.y + size / 2);    // Right-middle point
                p3 = ImVec2(pos.x, pos.y + size);               // Bottom-left point
            }
            else {
                p1 = ImVec2(pos.x + size, pos.y);               // Right point
                p2 = ImVec2(pos.x, pos.y + size / 2);           // Left-middle point
                p3 = ImVec2(pos.x + size, pos.y + size);        // Bottom-right point
            }

            ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
            if (is_connected) {
                draw_list->AddTriangleFilled(p1, p2, p3, color);
            }
            else {
                draw_list->AddTriangle(p1, p2, p3, color, 1.5f);
            }

            ui::dummy(size, size);
        }


        void square(bool is_connected) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float size = 16.0f;

            ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
            if (is_connected) {
                draw_list->AddRectFilled(pos, ImVec2(pos.x + size, pos.y + size), color);
            }
            else {
                draw_list->AddRect(pos, ImVec2(pos.x + size, pos.y + size), color, 0.0f, 0, 2.0f);
            }

            ui::dummy(size, size);
        }

        void diamond(bool is_connected) {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            float size = 12.0f;
            float half_size = size / 2;

            ImVec2 p1 = ImVec2(pos.x + half_size, pos.y);
            ImVec2 p2 = ImVec2(pos.x + size, pos.y + half_size);
            ImVec2 p3 = ImVec2(pos.x + half_size, pos.y + size);
            ImVec2 p4 = ImVec2(pos.x, pos.y + half_size);

            ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
            if (is_connected) {
                draw_list->AddQuadFilled(p1, p2, p3, p4, color);
            }
            else {
                draw_list->AddQuad(p1, p2, p3, p4, color, 1.5f);
            }

            ui::dummy(size, size);
        }

        void pentagon(bool is_connected) {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            float size = 12.0f;
            float radius = size / 2;

            ImVec2 points[5] = {
                ImVec2(pos.x + radius, radius + pos.y),
                ImVec2(pos.x + size, pos.y + radius + radius * 0.6f),
                ImVec2(pos.x + radius + radius * 0.8f, pos.y + radius + size),
                ImVec2(pos.x + radius * 0.2f, pos.y + radius + size),
                ImVec2(pos.x, pos.y + radius + radius * 0.6f)
            };

            ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
            if (is_connected) {
                draw_list->AddConvexPolyFilled(points, 5, color);
            }
            else {
                draw_list->AddPolyline(points, 5, color, true, 1.5f);
            }

            ui::dummy(size, size);
        }

        void hexagon(bool is_connected) {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            float size = 12.0f;
            float radius = size / 2;

            float half_width = radius * 0.866f;
            float height_offset = radius * 0.5f;

            ImVec2 points[6] = {
                ImVec2(pos.x + radius, pos.y),
                ImVec2(pos.x + radius + half_width, pos.y + height_offset),
                ImVec2(pos.x + radius + half_width, pos.y + size - height_offset),
                ImVec2(pos.x + radius, pos.y + size),
                ImVec2(pos.x + radius - half_width, pos.y + size - height_offset),
                ImVec2(pos.x + radius - half_width, pos.y + height_offset)
            };

            ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
            if (is_connected) {
                draw_list->AddConvexPolyFilled(points, 6, color);
            }
            else {
                draw_list->AddPolyline(points, 6, color, true, 1.5f);
            }

            ui::dummy(size, size);
        }

        void and_gate(bool is_connected) {
            float size = 12.0f;
            float width = size / 2.0f;
            float height = size;
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            ImVec2 p1 = ImVec2(pos.x, pos.y);
            ImVec2 p2 = ImVec2(pos.x + width * 0.5f, pos.y);
            ImVec2 p3 = ImVec2(pos.x + width, pos.y + height / 2);
            ImVec2 p4 = ImVec2(pos.x + width * 0.5f, pos.y + height);
            ImVec2 p5 = ImVec2(pos.x, pos.y + height);

            draw_list->PathLineTo(p1);
            draw_list->PathLineTo(p2);

            draw_list->PathArcTo(p3, height / 2, (float)-std::numbers::pi / 2, (float)std::numbers::pi / 2, 20);

            draw_list->PathLineTo(p4);
            draw_list->PathLineTo(p5);

            ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
            if (is_connected) {
                draw_list->PathFillConvex(color);
            }
            else {
                draw_list->PathStroke(color, true, 2.0f);
            }

            ui::dummy(size, size);
        }

        void circle(bool is_connected) {
            float size = 12.0f;
            float radius = size / 2.0f;
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);

            ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
            if (is_connected) {
                draw_list->AddCircleFilled(center, radius, color);
            }
            else {
                draw_list->AddCircle(center, radius, color, 0, 2.0f);
            }

            ui::dummy(size, size);
        }

    } // namespace connector
} // namespace ui