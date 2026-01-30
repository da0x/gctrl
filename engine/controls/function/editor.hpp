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
#include <cstring>
#include "imgui.h"
#include "ui/code.hpp"
#include "controls/signal/editor.hpp"
#include "controls/function/instance.hpp"

namespace function {

    void render_function_table(const char* section_name, function::instance::list& functions, const function::object::list& function_db) {
        if (ImGui::BeginTable(section_name, 4, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("##Icon", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Function");
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("##Actions", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();

            for (auto it = functions.begin(); it != functions.end();) {
                auto& func = *it;
                ui::id::push(static_cast<int>(func.id()));
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ui::text(ui::icon::function);

                ImGui::TableSetColumnIndex(1);
                ui::input::text("##function-name", func.name);

                ImGui::TableSetColumnIndex(2);
                ui::text(func.prototype_typename());

                ImGui::TableSetColumnIndex(3);
                if (ui::button(ui::icon::trash)) {
                    it = functions.erase(it);
                    ui::id::pop();
                    ImGui::EndTable();
                    return;
                }
                else {
                    ++it;
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Delete this function");
                }

                ui::id::pop();
            }

            ImGui::EndTable();
        }

        if (ui::button("Insert Function")) {
            ui::popup::open("Select Function");
        }

        if (ui::popup::begin("Select Function")) {
            for (const auto& f : function_db) {
                if (ui::selectable(f.display_name())) {
                    functions.emplace_back(function::instance(f));
                }
            }
            ui::popup::end();
        }
    }

    void render_editor(function::object& func, const function::object::list& function_db, const signal::object::list& signal_db) {
        ui::begin("Function Editor");
        ui::columns(2);
        ui::id::push(func.uuid.c_str());
        func.editor();
        ui::separator("Signals");
        if (ImGui::CollapsingHeader("Inputs", ImGuiTreeNodeFlags_DefaultOpen)) {
            signal::editor(func.inputs, "Inputs", signal_db);
        }
        if (ImGui::CollapsingHeader("Outputs", ImGuiTreeNodeFlags_DefaultOpen)) {
            signal::editor(func.outputs, "Outputs", signal_db);
        }
        if (ImGui::CollapsingHeader("Memory", ImGuiTreeNodeFlags_DefaultOpen)) {
            signal::editor(func.memory, "Memory", signal_db);
        }
        ui::next_column();
        ui::separator("C++");
        ui::code::code_editor(func.code_block);
        ui::id::pop();
        ui::end();
    }

}  // namespace function
