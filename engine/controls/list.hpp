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

#include "controls/record.hpp"
#include "ui/ui.hpp"
#include "ui/navigation.hpp"
#include <list>

namespace controls {


    template <typename T>
    void list(const char* object_name, std::list<T>& records, ui::navigation::history& navigation_history, ui::navigation::type type) {
        ui::begin(object_name);

        if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
            navigation_history.push(ui::navigation::entry{ static_cast<int>(type), -1 });
        }

        if (ui::button("+ Add")) {
            records.emplace_back();
            navigation_history.push(ui::navigation::entry{ static_cast<int>(type), static_cast<int>(std::distance(records.begin(), std::prev(records.end()))) });
        }

        const ui::navigation::entry& current_entry = navigation_history.current();

        int index = 0;
        for (auto& record : records) {
            ImGui::PushID(record.uuid.c_str());
            bool is_selected = current_entry.type == static_cast<int>(type) && current_entry.index == index;

            ui::style::color::push(ui::colors::text, ui::theme::vs2022::bright_yellow);
            if (ui::selectable(record.display_name().c_str(), is_selected)) {
                navigation_history.push(ui::navigation::entry{ static_cast<int>(type), index });
            }
            ui::style::color::pop();

            ImGui::PopID();
            ++index;
        }

        ui::end();
    }

    template <typename T>
    void render_list(const char* icon, const char* label, const char* add_label, std::list<T>& records, ui::navigation::type type) {
        ui::begin(label);

        if (ui::button(add_label)) {
            records.emplace_back();
            ui::navigation::push(ui::navigation::entry{ static_cast<int>(type), static_cast<int>(std::distance(records.begin(), std::prev(records.end()))) });
        }

        auto current_entry = ui::navigation::current_entry();
        bool item_selected = current_entry.type == static_cast<int>(type) && current_entry.index >= 0 && current_entry.index < static_cast<int>(records.size());

        ui::same_line();
        if (item_selected) {
            if (ui::button(ui::icon::remove)) {
                int deleted_index = current_entry.index;

                auto it = records.begin();
                std::advance(it, deleted_index);
                records.erase(it);

                ui::navigation::delete_entry(static_cast<int>(type), deleted_index, current_entry.mode);
                ui::navigation::push(ui::navigation::entry{ static_cast<int>(type), -1 });
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Delete the selected item");
            }
        }
        else {
            ImGui::BeginDisabled();
            ui::button(ui::icon::remove);
            ImGui::EndDisabled();
        }

        ui::separator();
        ui::font::push(ui::font::type::code);

        int index = 0;
        for (auto& record : records) {
            ImGui::PushID(record.uuid.c_str());
            bool is_selected = current_entry.type == static_cast<int>(type) && current_entry.index == index;

            ui::text(icon);
            ui::same_line();
            ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_typename);
            if (ui::selectable(record.display_name().c_str(), is_selected)) {
                ui::navigation::push(ui::navigation::entry{ static_cast<int>(type), index });
            }
            ui::style::color::pop();

            ImGui::PopID();
            ++index;
        }

        ui::font::pop();
        ui::end();
    }
}
