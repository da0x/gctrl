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

#include "controls/controller/object.hpp"
#include "controls/element/instance.hpp"
#include "controls/port/instance.hpp"

#include "ui/ui.hpp"
#include "ui/graph.hpp"
#include "ui/focus.hpp"
#include "ui/hierarchy.hpp"
#include <nlohmann/json.hpp>
#include <list>
#include <string>
#include <cstdint>
#include <functional>

namespace ed = ax::NodeEditor;

namespace controller {

    using drill_down_callback = std::function<void(const ui::focus::focus_entry&)>;
    inline drill_down_callback on_drill_down;

    struct selectable {
        element::instance* element = nullptr;
        port::plug::instance* plug = nullptr;
        port::socket::instance* socket = nullptr;
    };

    void render_controller_properties(controller::object& active_controller, selectable& selected) {
        ui::begin("Controller Properties");
        active_controller.editor();
        ui::graph::render_list(ui::icon::element, "Elements", active_controller.elements, selected.element);
        ui::graph::render_list(ui::icon::plug, "Plugs", active_controller.plugs, selected.plug);
        ui::graph::render_list(ui::icon::socket, "Sockets", active_controller.sockets, selected.socket);
        ui::end();
    }

    void render_plug_node(port::plug::instance& instance, controller::object& active_controller) {
        ed::PushStyleColor(ed::StyleColor_NodeBg, ui::theme::vs2022::plug_background);

        const signal::instance::list& left_group = (instance.direction == ui::direction::ltr) ? instance.inputs() : instance.outputs();
        const signal::instance::list& right_group = (instance.direction == ui::direction::ltr) ? instance.outputs() : instance.inputs();

        ui::graph::render_node(ui::icon::plug, instance, left_group, right_group, active_controller, ui::connector::shape::triangle);
        ed::PopStyleColor();
    }

    void render_socket_node(port::socket::instance& instance, controller::object& active_controller) {
        ed::PushStyleColor(ed::StyleColor_NodeBg, ui::theme::vs2022::socket_background);
        const signal::instance::list& left_group = (instance.direction == ui::direction::ltr) ? instance.inputs() : instance.outputs();
        const signal::instance::list& right_group = (instance.direction == ui::direction::ltr) ? instance.outputs() : instance.inputs();

        ui::graph::render_node(ui::icon::socket, instance, left_group, right_group, active_controller, ui::connector::shape::triangle);
        ed::PopStyleColor();
    }

    void render_element_node(element::instance& instance, controller::object& active_controller) {
        ed::PushStyleColor(ed::StyleColor_NodeBg, ui::theme::vs2022::element_background);

        const signal::instance::list& left_group = (instance.direction == ui::direction::ltr) ? instance.inputs() : instance.outputs();
        const signal::instance::list& right_group = (instance.direction == ui::direction::ltr) ? instance.outputs() : instance.inputs();

        ui::graph::render_node(ui::icon::element, instance, left_group, right_group, active_controller, ui::connector::shape::triangle);
        ed::PopStyleColor();
    }

    uint64_t query_selected_node() {
        ed::NodeId selected_nodes[1];
        int selected_node_count = ed::GetSelectedNodes(selected_nodes, 1);

        if (selected_node_count > 0) {
            return selected_nodes[0].Get();
        }
        return 0;
    }

    void render_node_editor(controller::object& active_controller, controller::selectable& selected, const element::object::list& element_db, const port::object::list& port_db) {
        ui::begin("Controller Editor", 0);
        ui::node::context_begin(active_controller.id());
        ui::columns(2);
        ui::separator("Insert");

        if (ui::button(ICON_FA_PLUS " Insert")) {
            ui::popup::open("InsertMenu");
        }

        if (ui::popup::begin("InsertMenu")) {
            if (ui::menu::begin("Plug")) {
                for (const auto& port_object : port_db) {
                    if (ui::menu::item(port_object.display_name().c_str())) {
                        active_controller.plugs.push_back(port::plug::instance(port_object));
                    }
                }
                ui::menu::end();
            }
            if (ui::menu::begin("Socket")) {
                for (const auto& port_object : port_db) {
                    if (ui::menu::item(port_object.display_name().c_str())) {
                        active_controller.sockets.push_back(port::socket::instance(port_object));
                    }
                }
                ui::menu::end();
            }
            if (ui::menu::begin("Element")) {
                for (const auto& element_object : element_db) {
                    if (ui::menu::item(element_object.display_name().c_str())) {
                        active_controller.elements.push_back(element::instance(element_object));
                    }
                }
                ui::menu::end();
            }
            ui::popup::end();
        }

        ui::same_line();
        bool node_selected = selected.element != nullptr || selected.plug != nullptr || selected.socket != nullptr;

        ed::LinkId selected_links[1000];
        ed::NodeId selected_nodes[1000];
        int selected_link_count = ed::GetSelectedLinks(selected_links, IM_ARRAYSIZE(selected_links));
        int selected_node_count = ed::GetSelectedNodes(selected_nodes, IM_ARRAYSIZE(selected_nodes));
        bool link_selected = selected_link_count > 0;
        bool multi_node_selected = selected_node_count > 0;

        if (multi_node_selected || link_selected) {
            if (ui::button(ICON_FA_TRASH " Delete")) {
                if (multi_node_selected) {
                    for (int i = 0; i < selected_node_count; ++i) {
                        uint64_t node_id = selected_nodes[i].Get();
                        // Remove associated links before deleting the node
                        active_controller.remove_links_to_node(node_id);
                        ui::graph::delete_nodes_by_id(node_id,
                            active_controller.elements,
                            active_controller.plugs,
                            active_controller.sockets);
                    }
                    selected.element = nullptr;
                    selected.plug = nullptr;
                    selected.socket = nullptr;
                }

                if (link_selected) {
                    for (int i = 0; i < selected_link_count; ++i) {
                        active_controller.remove_link(selected_links[i].Get());
                    }
                }
            }
        }
        else {
            ui::disabled::begin();
            ui::button(ICON_FA_TRASH " Delete");
            ui::disabled::end();
        }

        ui::next_column();
        ui::separator("Compiler");

        if (ui::button("Save & Compile")) {
            ui::cout << "Not implemented ..." << ui::endl;
//            code::delete_gctrl_directory();
//            active_controller.generate();
//            system::compile();
        }

        ui::separator();
        ui::columns(1);

        ui::node::begin(active_controller.uuid);

        // Apply pending selection from hierarchy
        if (uint64_t pending_id = ui::hierarchy::consume_pending_selection()) {
            ed::SelectNode(pending_id, false);
            ed::NavigateToSelection();
        }

        ui::font::push(ui::font::type::code);
        for (auto& element_instance : active_controller.elements) {
            render_element_node(element_instance, active_controller);
        }
        for (auto& plug_instance : active_controller.plugs) {
            render_plug_node(plug_instance, active_controller);
        }
        for (auto& socket_instance : active_controller.sockets) {
            render_socket_node(socket_instance, active_controller);
        }
        ui::graph::handle_link_creation(active_controller);
        ui::graph::render_existing_links(active_controller);
        ui::font::pop();
        ui::node::end();

        // Handle click on empty canvas to deselect
        if (ed::GetBackgroundClickButtonIndex() == 0) {
            ed::ClearSelection();
        }

        uint64_t selected_node_id = query_selected_node();

        // Update hierarchy with current canvas selection
        ui::hierarchy::set_canvas_selection(selected_node_id);

        if (selected_node_id != 0) {
            selected.element = ui::graph::find_selected_in(active_controller.elements, selected_node_id);
            selected.plug = ui::graph::find_selected_in(active_controller.plugs, selected_node_id);
            selected.socket = ui::graph::find_selected_in(active_controller.sockets, selected_node_id);
        }

        // Handle double-click drill-down (elements can be edited)
        ed::NodeId double_clicked_node = ed::GetDoubleClickedNode();
        if (double_clicked_node && on_drill_down) {
            uint64_t id = double_clicked_node.Get();
            ui::graph::try_drilldown_prototype(active_controller.elements, id, ui::focus::level::element, on_drill_down);
        }

        ui::node::context_end();
        ui::end();
    }

    void render_editor(controller::object& active_controller, const element::object::list& element_db, const port::object::list& port_db) {
        selectable selected;
        render_controller_properties(active_controller, selected);
        render_node_editor(active_controller, selected, element_db, port_db);


        if (selected.element) {
            ui::begin("Element Properties");
            selected.element->editor();

            ui::graph::render_list(ui::icon::signal, "Inputs", selected.element->inputs());
            ui::graph::render_list(ui::icon::signal, "Outputs", selected.element->outputs());

            ui::end();
        }
        else if (selected.plug) {
            ui::begin("Plug Properties");
            selected.plug->editor();
            ui::graph::render_list(ui::icon::signal, "Inputs", selected.plug->inputs());
            ui::graph::render_list(ui::icon::signal, "Outputs", selected.plug->outputs());
            ui::end();
        }
        else if (selected.socket) {
            ui::begin("Socket Properties");
            selected.socket->editor();
            ui::graph::render_list(ui::icon::signal, "Inputs", selected.socket->inputs());
            ui::graph::render_list(ui::icon::signal, "Outputs", selected.socket->outputs());
            ui::end();
        }
    }

} // namespace controller
