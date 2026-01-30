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
#include "ui/selection.hpp"
#include <nlohmann/json.hpp>
#include <list>
#include <string>
#include <cstdint>
#include <functional>

namespace ed = ax::NodeEditor;

namespace controller {

    using drill_down_callback = std::function<void(const ui::focus::focus_entry&)>;
    inline drill_down_callback on_drill_down;

    // Legacy struct for viewer compatibility (not used in main selection flow)
    struct selectable {
        element::instance* element = nullptr;
        port::plug::instance* plug = nullptr;
        port::socket::instance* socket = nullptr;
    };

    // Find selected element/plug/socket from shared selection
    inline element::instance* find_selected_element(controller::object& controller, ui::selection& selection) {
        if (!selection.has_selection()) return nullptr;
        for (auto& element : controller.elements) {
            if (element.id() == selection.selected_id) return &element;
        }
        return nullptr;
    }

    inline port::plug::instance* find_selected_plug(controller::object& controller, ui::selection& selection) {
        if (!selection.has_selection()) return nullptr;
        for (auto& plug : controller.plugs) {
            if (plug.id() == selection.selected_id) return &plug;
        }
        return nullptr;
    }

    inline port::socket::instance* find_selected_socket(controller::object& controller, ui::selection& selection) {
        if (!selection.has_selection()) return nullptr;
        for (auto& socket : controller.sockets) {
            if (socket.id() == selection.selected_id) return &socket;
        }
        return nullptr;
    }

    void render_controller_properties(controller::object& active_controller, ui::selection& selection) {
        element::instance* selected_element = find_selected_element(active_controller, selection);
        port::plug::instance* selected_plug = find_selected_plug(active_controller, selection);
        port::socket::instance* selected_socket = find_selected_socket(active_controller, selection);

        ui::begin("Controller Properties");
        active_controller.editor();
        ui::graph::render_list(ui::icon::element, "Elements", active_controller.elements, selected_element);
        ui::graph::render_list(ui::icon::plug, "Plugs", active_controller.plugs, selected_plug);
        ui::graph::render_list(ui::icon::socket, "Sockets", active_controller.sockets, selected_socket);
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

    void render_node_editor(controller::object& active_controller, const element::object::list& element_db, const port::object::list& port_db, ui::selection& selection) {
        ui::begin("Controller Editor", 0);
        ui::node::context_begin(active_controller.id());
        ui::columns(2);
        ui::separator("Insert");

        if (ui::button(ui::icon::insert)) {
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
                        active_controller.remove_links_to_node(node_id);
                        ui::graph::delete_nodes_by_id(node_id,
                            active_controller.elements,
                            active_controller.plugs,
                            active_controller.sockets);
                    }
                    selection.clear();
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
            ui::button(ui::icon::remove);
            ui::disabled::end();
        }

        ui::next_column();
        ui::separator("Compiler");

        if (ui::button("Save & Compile")) {
            ui::cout << "Not implemented ..." << ui::endl;
        }

        ui::separator();
        ui::columns(1);

        ui::node::begin(active_controller.uuid);

        // Sync node editor with shared selection - selection is the single source of truth
        if (selection.has_selection()) {
            bool is_in_this_controller = find_selected_element(active_controller, selection) ||
                                         find_selected_plug(active_controller, selection) ||
                                         find_selected_socket(active_controller, selection);
            if (is_in_this_controller) {
                ed::SelectNode(ed::NodeId(selection.selected_id), false);
            } else {
                ed::ClearSelection();
            }
        } else {
            ed::ClearSelection();
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

        // Handle click on empty canvas to deselect
        if (ed::GetBackgroundClickButtonIndex() == 0) {
            selection.clear();
        }

        // Handle double-click drill-down (elements can be edited)
        // Clear selection when drilling down to a new container
        ed::NodeId double_clicked_node = ed::GetDoubleClickedNode();
        if (double_clicked_node && on_drill_down) {
            uint64_t id = double_clicked_node.Get();
            selection.clear();
            ui::graph::try_drilldown_prototype(active_controller.elements, id, ui::focus::level::element, on_drill_down);
        }

        // Handle single click on node - update shared selection
        // Only update if user clicked (not just reading stale state)
        ed::NodeId clicked_node = ed::GetClickedNode();
        if (clicked_node) {
            uint64_t node_id = clicked_node.Get();
            selection.select(node_id);
        }

        ui::node::end();
        ui::node::context_end();
        ui::end();
    }

    void render_editor(controller::object& active_controller, const element::object::list& element_db, const port::object::list& port_db, ui::selection& selection) {
        render_controller_properties(active_controller, selection);
        render_node_editor(active_controller, element_db, port_db, selection);

        // Derive selected items from shared selection
        element::instance* selected_element = find_selected_element(active_controller, selection);
        port::plug::instance* selected_plug = find_selected_plug(active_controller, selection);
        port::socket::instance* selected_socket = find_selected_socket(active_controller, selection);

        if (selected_element) {
            ui::begin("Element Properties");
            selected_element->editor();

            ui::graph::render_list(ui::icon::signal, "Inputs", selected_element->inputs());
            ui::graph::render_list(ui::icon::signal, "Outputs", selected_element->outputs());

            ui::end();
        }
        else if (selected_plug) {
            ui::begin("Plug Properties");
            selected_plug->editor();
            ui::graph::render_list(ui::icon::signal, "Inputs", selected_plug->inputs());
            ui::graph::render_list(ui::icon::signal, "Outputs", selected_plug->outputs());
            ui::end();
        }
        else if (selected_socket) {
            ui::begin("Socket Properties");
            selected_socket->editor();
            ui::graph::render_list(ui::icon::signal, "Inputs", selected_socket->inputs());
            ui::graph::render_list(ui::icon::signal, "Outputs", selected_socket->outputs());
            ui::end();
        }
    }

} // namespace controller
