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
#include "ui/icons.hpp"
#include "ui/theme.hpp"
#include "ui/focus.hpp"
#include "controls/network/object.hpp"
#include "controls/machine/object.hpp"
#include "controls/machine/editor.hpp"
#include "controls/runtime/state.hpp"
#include <functional>

namespace network {
    namespace ed = ax::NodeEditor;

    using drill_down_callback = std::function<void(const ui::focus::focus_entry&)>;
    inline drill_down_callback on_drill_down;
    inline machine::object* selected_machine = nullptr;

    // Unique context ID for network editor (using a large value to avoid collision)
    inline constexpr uint64_t network_context_id = 0xFFFFFFFF00000001;

    inline uint64_t query_selected_node() {
        ed::NodeId selected_nodes[1];
        int count = ed::GetSelectedNodes(selected_nodes, 1);
        if (count > 0) {
            return selected_nodes[0].Get();
        }
        return 0;
    }

    inline machine::object* find_machine_by_node_id(machine::object::list& machines, uint64_t node_id) {
        for (auto& mach : machines) {
            if (mach.id() == node_id) return &mach;
        }
        return nullptr;
    }

    inline void render_machine_node(machine::object& mach, runtime::tracker& runtime_tracker) {
        auto status = runtime_tracker.get_status(mach.id());

        // Set node colors based on status
        ImVec4 border_color = ui::theme::vs2022::highlight;
        switch (status) {
            case runtime::status::running:
                border_color = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);
                break;
            case runtime::status::building:
                border_color = ImVec4(1.0f, 0.7f, 0.0f, 1.0f);
                break;
            case runtime::status::error:
                border_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                break;
            default:
                break;
        }

        ed::PushStyleColor(ed::StyleColor_NodeBg, ui::theme::vs2022::frame_background);
        ed::PushStyleColor(ed::StyleColor_NodeBorder, border_color);

        ui::node::begin_node(mach.id());
        ui::id::push(mach.id());

        ui::dummy(0.0f, 10.0f);

        // Icon and type
        float padding = 40.0f;
        std::string type_text = "Machine";
        float type_width = ui::text_size(type_text).x;
        float name_width = ui::text_size(mach.display_name()).x;
        float total_width = std::max(type_width, name_width) + 2 * padding;
        float type_padding = (total_width - type_width) * 0.5f;
        float name_padding = (total_width - name_width) * 0.5f;

        // Machine icon and type
        ui::dummy(type_padding, 0.0f);
        ui::same_line();
        ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_typename);
        ui::text(std::string(ui::icon::machine) + " " + type_text);
        ui::style::color::pop();

        // Machine name
        ui::dummy(name_padding, 0.0f);
        ui::same_line();
        ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_instance);
        ui::text(mach.display_name());
        ui::style::color::pop();

        ui::dummy(0.0f, 10.0f);

        // Summary info
        size_t controller_count = mach.controllers.size();
        size_t driver_count = mach.drivers.size();
        std::string summary = std::to_string(controller_count) + " controller" + (controller_count != 1 ? "s" : "") +
                              ", " + std::to_string(driver_count) + " driver" + (driver_count != 1 ? "s" : "");
        float summary_width = ui::text_size(summary).x;
        float summary_padding = (total_width - summary_width) * 0.5f;
        ui::dummy(summary_padding, 0.0f);
        ui::same_line();
        ImGui::TextDisabled("%s", summary.c_str());

        ui::dummy(0.0f, 10.0f);

        // Status indicator
        const char* status_icon;
        const char* status_text;
        ImVec4 status_color;

        switch (status) {
            case runtime::status::stopped:
                status_icon = ICON_FA_STOP;
                status_text = "Stopped";
                status_color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                break;
            case runtime::status::building:
                status_icon = ICON_FA_HAMMER;
                status_text = "Building...";
                status_color = ImVec4(1.0f, 0.7f, 0.0f, 1.0f);
                break;
            case runtime::status::running:
                status_icon = ICON_FA_PLAY;
                status_text = "Running";
                status_color = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);
                break;
            case runtime::status::error:
                status_icon = ICON_FA_TRIANGLE_EXCLAMATION;
                status_text = "Error";
                status_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                break;
        }

        std::string status_str = std::string(status_icon) + " " + status_text;
        float status_width = ui::text_size(status_str).x;
        float status_padding = (total_width - status_width) * 0.5f;
        ui::dummy(status_padding, 0.0f);
        ui::same_line();
        ImGui::PushStyleColor(ImGuiCol_Text, status_color);
        ui::text(status_str);
        ImGui::PopStyleColor();

        ui::dummy(0.0f, 10.0f);

        ui::id::pop();
        ui::node::end_node();

        ed::PopStyleColor(2);
    }

    inline void render_editor(network::object& net, runtime::tracker& runtime_tracker) {
        ui::begin("Network Editor", 0);
        ui::node::context_begin(network_context_id);

        // Toolbar - show controls for selected machine
        ui::separator("Runtime");
        if (selected_machine) {
            machine::render_runtime_toolbar(*selected_machine, runtime_tracker);
        } else {
            ImGui::TextDisabled("Select a machine to control");
        }

        ui::separator("Toolbar");
        if (ui::button("Insert Machine")) {
            machine::object new_machine;
            new_machine.name = "new_machine";
            net.machines.push_back(new_machine);
        }

        ui::same_line();

        // Delete selected machine
        bool has_selection = selected_machine != nullptr;
        if (!has_selection) {
            ui::disabled::begin();
        }
        if (ui::button(ICON_FA_TRASH " Delete")) {
            if (selected_machine) {
                net.machines.remove_if([](const machine::object& m) {
                    return &m == selected_machine;
                });
                selected_machine = nullptr;
            }
        }
        if (!has_selection) {
            ui::disabled::end();
        }

        ui::separator();

        ui::node::begin(std::string("network_") + std::to_string(network_context_id));

        ui::font::push(ui::font::type::code);
        for (auto& mach : net.machines) {
            render_machine_node(mach, runtime_tracker);
        }
        ui::font::pop();

        ui::node::end();

        // Handle click on empty canvas to deselect
        if (ed::GetBackgroundClickButtonIndex() == 0) {
            ed::ClearSelection();
            selected_machine = nullptr;
        }

        // Handle selection
        uint64_t selected_id = query_selected_node();
        if (selected_id != 0) {
            selected_machine = find_machine_by_node_id(net.machines, selected_id);
        }

        // Handle double-click drill-down
        ed::NodeId double_clicked_node = ed::GetDoubleClickedNode();
        if (double_clicked_node && on_drill_down) {
            uint64_t id = double_clicked_node.Get();
            auto* mach = find_machine_by_node_id(net.machines, id);
            if (mach) {
                on_drill_down({ui::focus::level::machine, mach->uuid, mach->display_name()});
            }
        }

        ui::node::context_end();
        ui::end();
    }

} // namespace network
