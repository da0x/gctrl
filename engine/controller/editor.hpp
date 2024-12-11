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

#include "controller/object.hpp"
#include "element/instance.hpp"
#include "port/instance.hpp"

#include "ui/ui.hpp"
#include "ui/graph.hpp"
#include <nlohmann/json.hpp>
#include <list>
#include <string>
#include <cstdint>

namespace ed = ax::NodeEditor;

namespace controller {

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
                        uint64_t selected_node_id = selected_nodes[i].Get();

                        active_controller.elements.remove_if(
                            [selected_node_id](const element::instance& elem) {
                                return elem.id() == selected_node_id;
                            }
                        );

                        active_controller.plugs.remove_if(
                            [selected_node_id](const port::plug::instance& plug) {
                                return plug.id() == selected_node_id;
                            }
                        );
                        active_controller.sockets.remove_if(
                            [selected_node_id](const port::socket::instance& socket) {
                                return socket.id() == selected_node_id;
                            }
                        );
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

        uint64_t selected_node_id = query_selected_node();

        if (selected_node_id != 0) {
            selected.element = nullptr;
            selected.plug = nullptr;
            selected.socket = nullptr;

            for (auto& element_instance : active_controller.elements) {
                if (element_instance.id() == selected_node_id) {
                    selected.element = &element_instance;
                    break;
                }
            }
            for (auto& plug_instance : active_controller.plugs) {
                if (plug_instance.id() == selected_node_id) {
                    selected.plug = &plug_instance;
                    break;
                }
            }
            for (auto& socket_instance : active_controller.sockets) {
                if (socket_instance.id() == selected_node_id) {
                    selected.socket = &socket_instance;
                    break;
                }
            }
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
