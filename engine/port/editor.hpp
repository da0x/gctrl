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
#include <vector>
#include <string>
#include <cstring>
#include "imgui.h"
#include <algorithm>
namespace ed = ax::NodeEditor;

namespace port {

    void render_editor(port::object& port_obj, const signal::object::list& signal_db) {
        ImGui::Begin("Port Editor");
        ImGui::PushID(port_obj.uuid.c_str());
        port_obj.editor();
        ui::separator("Port Data");
        if (ui::button("Reverse Direction")) {
            std::swap(port_obj.incoming, port_obj.outgoing);
        }
        ui::columns(2);
        ui::separator("Socket Data");
        if (ImGui::CollapsingHeader("Socket Signals", ImGuiTreeNodeFlags_DefaultOpen)) {
            signal::editor(port_obj.incoming, "Socket Signals", signal_db);
        }
        ui::next_column();
        ui::separator("Plug Data");
        if (ImGui::CollapsingHeader("Plug Signals", ImGuiTreeNodeFlags_DefaultOpen)) {
            signal::editor(port_obj.outgoing, "Plug Signals", signal_db);
        }

        ImGui::PopID();
        ImGui::End();
    }

}  // namespace port_editor
