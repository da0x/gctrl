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

#include <imgui.h>
#include "ui/focus.hpp"
#include "ui/icons.hpp"
#include "ui/theme.hpp"

namespace ui {
namespace breadcrumb {

    inline const char* icon_for_level(focus::level level) {
        switch (level) {
            case focus::level::machine: return ui::icon::machine;
            case focus::level::controller: return ui::icon::controller;
            case focus::level::driver: return ui::icon::driver;
            case focus::level::element: return ui::icon::element;
            case focus::level::plug: return ui::icon::plug;
            case focus::level::socket: return ui::icon::socket;
            default: return "";
        }
    }

    // Returns the depth (1-based index) of the clicked breadcrumb segment, or -1 if no click
    inline int render(const focus::path& path) {
        if (path.empty()) return -1;

        int clicked_depth = -1;
        const auto& entries = path.all();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ui::theme::vs2022::highlight);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ui::theme::vs2022::button_active);

        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& entry = entries[i];

            if (i > 0) {
                ImGui::SameLine();
                ImGui::TextDisabled(">");
                ImGui::SameLine();
            }

            std::string label = std::string(icon_for_level(entry.type)) + " " + entry.name;

            // Last item is not clickable (current location)
            if (i == entries.size() - 1) {
                ImGui::Text("%s", label.c_str());
            } else {
                ImGui::PushID(static_cast<int>(i));
                if (ImGui::SmallButton(label.c_str())) {
                    clicked_depth = static_cast<int>(i + 1);
                }
                ImGui::PopID();
            }
        }

        ImGui::PopStyleColor(3);

        return clicked_depth;
    }

} // namespace breadcrumb
} // namespace ui
