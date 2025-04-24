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

namespace controller {

    namespace viewer {

        void render_controller_properties(controller::object& active_controller, selectable& selected) {
            ui::begin("Controller Properties");
            active_controller.viewer();
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

        void render_node_viewer(controller::object& active_controller, controller::selectable& selected) {
            ui::begin("Controller Viewer", 0);
            ui::node::context_begin(active_controller.id());
            ui::columns(2);

            ui::next_column();
            ui::separator("Signals");

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

        void render_viewer(controller::object& active_controller) {
            selectable selected;
            render_controller_properties(active_controller, selected);
            render_node_viewer(active_controller, selected);

            if (selected.element) {
                ui::begin("Element Signals");
                ui::graph::render_list(ui::icon::signal, "Inputs", selected.element->inputs());
                ui::graph::render_list(ui::icon::signal, "Outputs", selected.element->outputs());
                ui::end();
            }
            else if (selected.plug) {
                ui::begin("Plug Signals");
                ui::graph::render_list(ui::icon::signal, "Inputs", selected.plug->inputs());
                ui::graph::render_list(ui::icon::signal, "Outputs", selected.plug->outputs());
                ui::end();
            }
            else if (selected.socket) {
                ui::begin("Socket Signals");
                ui::graph::render_list(ui::icon::signal, "Inputs", selected.socket->inputs());
                ui::graph::render_list(ui::icon::signal, "Outputs", selected.socket->outputs());
                ui::end();
            }
        }
    }

} // namespace controller
