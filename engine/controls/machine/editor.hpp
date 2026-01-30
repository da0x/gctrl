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
#include "ui/graph.hpp"
#include "ui/theme.hpp"
#include "ui/focus.hpp"
#include "ui/selection.hpp"
#include "controls/machine/object.hpp"
#include "controls/driver/instance.hpp"
#include "controls/driver/type.hpp"
#include "controls/driver/editor.hpp"
#include "controls/controller/editor.hpp"
#include "controls/runtime/state.hpp"
#include "controls/runtime/build.hpp"
#include <numbers>
#include <algorithm>
#include <functional>

#undef max


namespace machine {
    namespace ed = ax::NodeEditor;

    using drill_down_callback = std::function<void(const ui::focus::focus_entry&)>;
    inline drill_down_callback on_drill_down;

    // Find selected controller/driver from shared selection
    inline controller::instance* find_selected_controller(machine::object& machine, ui::selection& sel) {
        if (!sel.has_selection()) return nullptr;
        for (auto& ctrl : machine.controllers) {
            if (ctrl.id() == sel.selected_id) return &ctrl;
        }
        return nullptr;
    }

    inline driver::instance* find_selected_driver(machine::object& machine, ui::selection& sel) {
        if (!sel.has_selection()) return nullptr;
        for (auto& drv : machine.drivers) {
            if (drv.id() == sel.selected_id) return &drv;
        }
        return nullptr;
    }

    void render_machine_properties(machine::object& active_machine, ui::selection& selection) {
        controller::instance* selected_controller = find_selected_controller(active_machine, selection);
        driver::instance* selected_driver = find_selected_driver(active_machine, selection);

        ui::begin("Machine Properties");
        active_machine.editor();
        ui::graph::render_list(ui::icon::controller, "Controllers", active_machine.controllers, selected_controller);
        ui::graph::render_list(ui::icon::driver, "Drivers", active_machine.drivers, selected_driver);
        ui::end();
    }

    template <typename T>
    void render_node(T& instance, machine::object& active_machine) {
        ui::node::begin_node(instance.id());
        ui::id::push(instance.id());
        ui::dummy(0.0f, 10.0f);

        const auto& left_group = (instance.direction == ui::direction::ltr) ? instance.inputs() : instance.outputs();
        const auto& right_group = (instance.direction == ui::direction::ltr) ? instance.outputs() : instance.inputs();

        float title_width = ui::text_size(instance.prototype_typename()).x;
        float subtitle_width = ui::text_size(instance.instance_name()).x;
        float max_left_width = 0.0f;
        float max_right_width = 0.0f;

        for (const auto& signal : left_group) {
            float signal_width = ui::text_size(signal.name).x;
            max_left_width = std::max(signal_width, max_left_width);
        }

        for (const auto& signal : right_group) {
            float signal_width = ui::text_size(signal.name).x;
            max_right_width = std::max(signal_width, max_right_width);
        }

        float connector_size = 15.0f;
        float spacing = 20.0f;
        float padding = 40.0f;
        float min_total_width = std::max(title_width, subtitle_width) + 2 * padding;
        float total_width = std::max(min_total_width, max_left_width + max_right_width + 2 * connector_size + spacing);
        float title_padding = (total_width - title_width) * 0.5f;
        float subtitle_padding = (total_width - subtitle_width) * 0.5f;
        title_padding = std::max(title_padding, padding);
        subtitle_padding = std::max(subtitle_padding, padding);

        ui::dummy(title_padding, 0.0f);
        ui::same_line();
        ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_typename);
        ui::text(instance.prototype_typename());
        ui::style::color::pop();

        ui::dummy(subtitle_padding, 0.0f);
        ui::same_line();
        ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_instance);
        ui::text(instance.instance_name());
        ui::style::color::pop();

        ui::dummy(0.0f, 10.0f);

        ui::group::begin();
        for (const auto& signal : left_group) {
            uint64_t pin_id = instance.id() | signal.id();
            ui::pin::begin(pin_id, ui::pin::input);
            ui::connector::triangle(instance.direction, active_machine.is_connected(pin_id));
            ui::pin::end();
            ui::same_line();
            ui::text(signal.name);
            if (ImGui::IsItemHovered()) {
                ImVec2 offset(30.0f, -30.0f);
                ImVec2 mouse_pos = ImGui::GetMousePos();
                ImVec2 canvas_pos = ax::NodeEditor::CanvasToScreen(ImVec2(mouse_pos.x + offset.x, mouse_pos.y + offset.y));

                ImGui::SetNextWindowPos(canvas_pos);
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(signal.prototype_typename().c_str());
                ImGui::EndTooltip();
            }
        }
        ui::group::end();

        float available_space = total_width - (max_left_width + max_right_width + 2 * connector_size);
        float spacing_between_io = available_space > 0.0f ? available_space : spacing;

        ui::same_line();
        ui::dummy(spacing_between_io, 0.0f);
        ui::same_line();

        ui::group::begin();
        for (const auto& signal : right_group) {
            uint64_t pin_id = instance.id() | signal.id();
            ui::dummy(max_right_width - ImGui::CalcTextSize(signal.name.c_str()).x, 0.0f);
            ui::same_line();
            ui::text(signal.name);
            if (ImGui::IsItemHovered()) {
                ImVec2 offset(30.0f, -30.0f);
                ImVec2 mouse_pos = ImGui::GetMousePos();
                ImVec2 canvas_pos = ax::NodeEditor::CanvasToScreen(ImVec2(mouse_pos.x + offset.x, mouse_pos.y + offset.y));

                ImGui::SetNextWindowPos(canvas_pos);
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(signal.prototype_typename().c_str());
                ImGui::EndTooltip();
            }
            ui::same_line();
            ui::pin::begin(pin_id, ui::pin::output);
            ui::connector::triangle(instance.direction, active_machine.is_connected(pin_id));
            ui::pin::end();
        }
        ui::group::end();


        ui::dummy(0.0f, 10.0f);
        ui::group::begin();
        if (ui::button(ui::icon::swap)) {
            instance.direction = (instance.direction == ui::direction::ltr) ? ui::direction::rtl : ui::direction::ltr;
        }

        if (ImGui::IsItemHovered()) {
            ImVec2 offset(30.0f, -30.0f);
            ImVec2 mouse_pos = ImGui::GetMousePos();
            ImVec2 canvas_pos = ax::NodeEditor::CanvasToScreen(ImVec2(mouse_pos.x + offset.x, mouse_pos.y + offset.y));

            ImGui::SetNextWindowPos(canvas_pos);
            ImGui::BeginTooltip();
            ImGui::TextUnformatted("Swap Direction");
            ImGui::EndTooltip();
        }
        ui::group::end();

        ui::id::pop();
        ui::node::end_node();
    }


    void render_controller_node(controller::instance& controller_instance, machine::object& active_machine) {
        ed::PushStyleColor(ed::StyleColor_NodeBg, ui::theme::vs2022::frame_background);
        ed::PushStyleColor(ed::StyleColor_NodeBorder, ui::theme::vs2022::highlight);
        auto& prototype = *dynamic_cast<const controller::object*>(&controller_instance.prototype);
        if (controller_instance.direction == ui::direction::ltr) {
            ui::graph::render_node(ui::icon::controller, controller_instance, controller_instance.inputs(), controller_instance.outputs(), active_machine, ui::connector::shape::circle);
        }
        else {
            ui::graph::render_node(ui::icon::controller, controller_instance, controller_instance.outputs(), controller_instance.inputs(), active_machine, ui::connector::shape::circle);
        }
        ed::PopStyleColor(2);
    }

    void render_driver_node(driver::instance& driver_obj, machine::object& active_machine) {
        ed::PushStyleColor(ed::StyleColor_NodeBg, ui::theme::vs2022::frame_background);
        ed::PushStyleColor(ed::StyleColor_NodeBorder, ui::theme::vs2022::code_keyword);
        if (driver_obj.direction == ui::direction::ltr) {
            ui::graph::render_node(driver::type_to_icon(driver_obj.backend_type), driver_obj, driver_obj.inputs(), driver_obj.outputs(), active_machine, ui::connector::shape::circle);
        }
        else {
            ui::graph::render_node(driver::type_to_icon(driver_obj.backend_type), driver_obj, driver_obj.outputs(), driver_obj.inputs(), active_machine, ui::connector::shape::circle);
        }
        ed::PopStyleColor(2);
    }

    void render_runtime_toolbar(machine::object& machine, runtime::tracker& runtime_tracker) {
        auto& state = runtime_tracker.state(machine.id());

        // Status indicator
        ImVec4 status_color;
        const char* status_icon;
        const char* status_text;

        switch (state.current_status) {
            case runtime::status::stopped:
                status_color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                status_icon = ui::icon::stop;
                status_text = "Stopped";
                break;
            case runtime::status::building:
                status_color = ImVec4(1.0f, 0.7f, 0.0f, 1.0f);
                status_icon = ui::icon::hammer;
                status_text = "Building...";
                break;
            case runtime::status::running:
                status_color = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);
                status_icon = ui::icon::play;
                status_text = "Running";
                break;
            case runtime::status::error:
                status_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                status_icon = ui::icon::warning;
                status_text = "Error";
                break;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, status_color);
        ImGui::Text("%s %s", status_icon, status_text);
        ImGui::PopStyleColor();

        if (state.has_error() && ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted(state.error_message.c_str());
            ImGui::EndTooltip();
        }

        ui::same_line();

        // Build button
        ImGui::BeginDisabled(state.is_building() || state.is_running());
        if (ui::button(ui::icon::build)) {
            runtime::build_machine(machine, runtime_tracker);
        }
        ImGui::EndDisabled();

        ui::same_line();

        // Run/Stop button
        if (state.is_stopped() || state.has_error()) {
            if (ui::button(ui::icon::run)) {
                runtime::run_machine(machine, runtime_tracker);
            }
        } else if (state.is_running()) {
            if (ui::button(ui::icon::halt)) {
                runtime::stop_machine(machine, runtime_tracker);
            }
        } else if (state.is_building()) {
            ImGui::BeginDisabled(true);
            ui::button(ui::icon::run);
            ImGui::EndDisabled();
        }
    }

    void render_node_editor(machine::object& active_machine, const controller::object::list& controller_db, const port::object::list& port_db, runtime::tracker& runtime_tracker, ui::selection& selection) {
        ui::begin("Machine Editor", 0);
        ui::node::context_begin(active_machine.id());

        // Runtime and Toolbar side by side
        ui::columns(2);

        // Left column: Runtime
        ui::separator("Runtime");
        render_runtime_toolbar(active_machine, runtime_tracker);

        // Right column: Toolbar
        ui::next_column();
        ui::separator("Toolbar");
        if (ui::button("Insert")) {
            ui::popup::open("InsertMenu");
        }
        if (ui::popup::begin("InsertMenu")) {
            if (ui::menu::begin("Controller")) {
                for (const auto& object : controller_db) {
                    if (ui::menu::item(object.display_name())) {
                        active_machine.controllers.push_back(controller::instance(object));
                    }
                }
                ui::menu::end();
            }
            if (ui::menu::begin("Driver")) {
                ui::font::push(ui::font::type::code);
                if (ui::menu::item(std::string(driver::type_to_icon(driver::type::udp)) + "  " + driver::type_to_typename(driver::type::udp))) {
                    driver::instance new_driver(driver::type::udp);
                    new_driver.name = "udp_driver";
                    active_machine.drivers.push_back(new_driver);
                }
                if (ui::menu::item(std::string(driver::type_to_icon(driver::type::config)) + "  " + driver::type_to_typename(driver::type::config))) {
                    driver::instance new_driver(driver::type::config);
                    new_driver.name = "config_driver";
                    active_machine.drivers.push_back(new_driver);
                }
                ui::font::pop();
                ImGui::EndMenu();
            }
            ui::popup::end();
        }
        ui::same_line();

        ed::LinkId selected_links[1000];
        ed::NodeId selected_nodes[1000];
        int selected_link_count = ed::GetSelectedLinks(selected_links, IM_ARRAYSIZE(selected_links));
        int selected_node_count = ed::GetSelectedNodes(selected_nodes, IM_ARRAYSIZE(selected_nodes));
        bool link_selected = selected_link_count > 0;
        bool multi_node_selected = selected_node_count > 0;

        if (multi_node_selected || link_selected) {
            if (ui::button(ui::icon::remove)) {
                if (multi_node_selected) {
                    for (int i = 0; i < selected_node_count; ++i) {
                        uint64_t node_id = selected_nodes[i].Get();
                        active_machine.remove_links_to_node(node_id);
                        ui::graph::delete_nodes_by_id(node_id,
                            active_machine.controllers,
                            active_machine.drivers);
                    }
                    selection.clear();
                }

                if (link_selected) {
                    for (int i = 0; i < selected_link_count; ++i) {
                        active_machine.remove_link(selected_links[i].Get());
                    }
                }
            }
        }
        else {
            ui::disabled::begin();
            ui::button(ui::icon::remove);
            ui::disabled::end();
        }

        ui::separator();
        ui::columns(1);

        ui::node::begin(active_machine.uuid);

        // Sync node editor with shared selection - selection is the single source of truth
        if (selection.has_selection()) {
            bool is_in_this_machine = find_selected_controller(active_machine, selection) || find_selected_driver(active_machine, selection);
            if (is_in_this_machine) {
                ed::SelectNode(ed::NodeId(selection.selected_id), false);
            } else {
                ed::ClearSelection();
            }
        } else {
            ed::ClearSelection();
        }

        ui::font::push(ui::font::type::code);
        for (auto& controller_instance : active_machine.controllers) {
            render_controller_node(controller_instance, active_machine);
        }
        for (auto& driver_instance : active_machine.drivers) {
            render_driver_node(driver_instance, active_machine);
        }
        ui::graph::handle_link_creation(active_machine);
        ui::graph::render_existing_links(active_machine);
        ui::font::pop();

        // Handle click on empty canvas to deselect
        if (ed::GetBackgroundClickButtonIndex() == 0) {
            selection.clear();
        }

        // Handle double-click drill-down (controllers have a canvas to navigate into)
        // Clear selection when drilling down to a new container
        ed::NodeId double_clicked_node = ed::GetDoubleClickedNode();
        if (double_clicked_node && on_drill_down) {
            uint64_t id = double_clicked_node.Get();
            selection.clear();
            ui::graph::try_drilldown_prototype(active_machine.controllers, id, ui::focus::level::controller, on_drill_down);
        }

        // Handle single click on node - update shared selection
        ed::NodeId clicked_node = ed::GetClickedNode();
        if (clicked_node) {
            selection.select(clicked_node.Get());
        }

        ui::node::end();
        ui::node::context_end();
        ui::end();
    }

    void render_editor(machine::object& active_machine, const controller::object::list& controller_db, const element::object::list& element_db, const port::object::list& port_db, const function::object::list& function_db, runtime::tracker& runtime_tracker, ui::selection& selection) {
        render_machine_properties(active_machine, selection);
        render_node_editor(active_machine, controller_db, port_db, runtime_tracker, selection);

        // Derive selected items from shared selection
        controller::instance* selected_controller = find_selected_controller(active_machine, selection);
        driver::instance* selected_driver = find_selected_driver(active_machine, selection);

        if (selected_controller != nullptr) {
            ui::begin("Controller Instance");
            selected_controller->editor();

            ui::graph::render_list(ui::icon::plug, "Inputs", selected_controller->inputs());
            ui::graph::render_list(ui::icon::socket, "Outputs", selected_controller->outputs());
            ui::end();
        }
        if (selected_driver != nullptr) {
            ui::begin("Driver");
            driver::render_editor(*selected_driver, port_db);
            ui::end();
        }
    }

}  // namespace machine
