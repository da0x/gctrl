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

#include "imgui.h"

namespace ui {
    namespace chart {
        void draw_square_chart_with_ball(float ball_x, float ball_y)
        {
            return;
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            ImVec2 p = ImGui::GetCursorScreenPos();
            float square_size = 200.0f;
            ImVec2 top_left = ImVec2(p.x, p.y);
            ImVec2 bottom_right = ImVec2(p.x + square_size, p.y + square_size);

            draw_list->AddRect(top_left, bottom_right, IM_COL32(255, 255, 255, 255));

            float normalized_x = top_left.x + ball_x * square_size;
            float normalized_y = top_left.y + ball_y * square_size;
            float ball_radius = 5.0f;
            draw_list->AddCircleFilled(ImVec2(normalized_x, normalized_y), ball_radius, IM_COL32(255, 0, 0, 255));

            ImGui::Dummy(ImVec2(square_size, square_size));
        }

        void show_my_window()
        {
            return;
            ImGui::Begin("Square Chart with Ball");
            float ball_x = 0.5f;
            float ball_y = 0.75f;
            draw_square_chart_with_ball(ball_x, ball_y);

            ImGui::End();
        }
    }
}