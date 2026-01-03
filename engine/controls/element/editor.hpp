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
#include "controls/function/editor.hpp"

namespace element {

    void render_editor(element::object& elem, const function::object::list& function_db, const signal::object::list& signal_db) {
        ImGui::Begin("Element Editor");
        ui::columns(2);
        ui::id::push(elem.uuid.c_str());

        elem.editor();

        ui::separator("Signals");

        if (ImGui::CollapsingHeader("Inputs", ImGuiTreeNodeFlags_DefaultOpen)) {
            signal::editor(elem.inputs, "Inputs", signal_db);
        }

        if (ImGui::CollapsingHeader("Outputs", ImGuiTreeNodeFlags_DefaultOpen)) {
            signal::editor(elem.outputs, "Outputs", signal_db);
        }

        if (ImGui::CollapsingHeader("Memory", ImGuiTreeNodeFlags_DefaultOpen)) {
            signal::editor(elem.memory, "Memory", signal_db);
        }

        if (ImGui::CollapsingHeader("Functions", ImGuiTreeNodeFlags_DefaultOpen)) {
            function::render_function_table("Functions", elem.functions, function_db);
        }

        ui::separator("Behavior");

        elem.frequency = static_cast<exec::frequency>(ui::combo("Element Frequency", static_cast<int>(elem.frequency), exec::frequency_map));

        ui::next_column();
        ui::separator("C++");
        ui::code::code_editor(elem.code_block);
        ui::id::pop();
        ImGui::End();
    }

}  // namespace element_editor
