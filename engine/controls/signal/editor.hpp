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

#include <da0x/uuid.hpp>
#include <list>
#include <string>
#include <algorithm>
#include "imgui.h"
#include "ui/code.hpp"

namespace signal {

    void editor(signal::instance::list& properties, const char* section_name, const signal::object::list& signals) {
        if (ImGui::BeginTable(section_name, 6, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {

            ImGui::TableSetupColumn("##Icon", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Value");
            ImGui::TableSetupColumn("Comment");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("##Actions", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();

            for (auto it = properties.begin(); it != properties.end();) {
                auto& prop = *it;

                ImGui::PushID(static_cast<int>(prop.id()));
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::SetNextItemWidth(-FLT_MIN);
                ui::text(ui::icon::signal);

                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(-FLT_MIN);
                ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_instance);
                ui::input::text("##Name", prop.name);
                ui::style::color::pop();

                ImGui::TableSetColumnIndex(2);
                ImGui::SetNextItemWidth(-FLT_MIN);
                ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_number);
                ui::input::text("##Default", prop.value);
                ui::style::color::pop();

                ImGui::TableSetColumnIndex(3);
                ImGui::SetNextItemWidth(-FLT_MIN);
                ui::input::text("##Comment", prop.description);

                ImGui::TableSetColumnIndex(4);
                ImGui::SetNextItemWidth(-FLT_MIN);
                ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_keyword);
                ui::text(prop.prototype_typename());
                ui::style::color::pop();

                ImGui::TableSetColumnIndex(5);
                if (ui::button(ui::icon::trash)) {
                    it = properties.erase(it);
                    ImGui::PopID();
                    continue;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Delete this signal");
                }
                ImGui::PopID();
                ++it;
            }
            ImGui::EndTable();
        }

        ui::id::push(section_name);
        if (ui::button("Add Signal")) {
            ui::popup::open("AddSignalPopup");
        }

        if (ui::popup::begin("AddSignalPopup")) {
            ui::separator("Signals");
            for (const auto& signal : signals) {
                if (ui::selectable(signal.description)) {
                    properties.emplace_back(signal::instance(signal));
                    ui::popup::close();
                }
            }
            ui::popup::end();
        }
        ui::id::pop();
    }

}  // namespace signal
