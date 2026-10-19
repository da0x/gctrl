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
#include "ui/selection.hpp"
#include "ui/graph.hpp"
#include "ui/connector.hpp"
#include <da0x/uuid.hpp>
#include "controls/network/object.hpp"
#include "controls/machine/object.hpp"
#include "controls/machine/editor.hpp"
#include "controls/runtime/state.hpp"
#include "controls/dashboard/instance.hpp"
#include "controls/dashboard/editor.hpp"
#include "controls/dashboard/listener.hpp"
#include "controls/panel/instance.hpp"
#include "controls/panel/editor.hpp"
#include "controls/client/instance.hpp"
#include "controls/client/editor.hpp"
#include "controls/runtime/build.hpp"
#include "ui/terminal.hpp"
#include "controls/panel/sender.hpp"
#include <functional>

namespace network {
    namespace ed = ax::NodeEditor;

    using drill_down_callback = std::function<void(const ui::focus::focus_entry&)>;
    inline drill_down_callback on_drill_down;

    // Unique context ID for network editor (using a large value to avoid collision)
    inline constexpr uint64_t network_context_id = 0xFFFFFFFF00000001;

    // Pin ID marker for per-driver output pins on machine node (one per driver)
    inline constexpr uint64_t DRIVER_OUTPUT_PIN_MARKER = 0xDC00000000000000ULL;

    inline uint64_t driver_output_pin_id(const machine::instance& mach, const driver::instance& drv) {
        uint64_t base = (mach.id() ^ drv.id()) & 0x00FFFFFFFFFFFFFFULL;
        return base | DRIVER_OUTPUT_PIN_MARKER;
    }

    inline bool is_driver_output_pin(uint64_t pin_id) {
        return (pin_id & 0xFF00000000000000ULL) == DRIVER_OUTPUT_PIN_MARKER;
    }

    inline bool is_dashboard_pin(uint64_t pin_id) {
        return (pin_id & 0xFF00000000000000ULL) == dashboard::PIN_MARKER;
    }

    inline bool is_panel_pin(uint64_t pin_id) {
        return (pin_id & 0xFF00000000000000ULL) == panel::PIN_MARKER;
    }

    inline machine::instance* find_machine_by_node_id(machine::instance::list& machines, uint64_t node_id) {
        for (auto& mach : machines) {
            if (mach.id() == node_id) return &mach;
        }
        return nullptr;
    }

    // Find machine and driver by driver output pin ID
    struct driver_pin_info {
        machine::instance* machine = nullptr;
        driver::instance* driver = nullptr;
    };

    inline driver_pin_info find_by_driver_output_pin(
        machine::instance::list& machines, uint64_t pin_id) {
        if (!is_driver_output_pin(pin_id)) return {};

        for (auto& mach : machines) {
            for (auto& drv : mach.drivers()) {
                if (driver_output_pin_id(mach, drv) == pin_id) {
                    auto& proto = const_cast<machine::object&>(mach.machine_prototype());
                    for (auto& proto_drv : proto.drivers) {
                        if (proto_drv.uuid == drv.uuid) {
                            return {&mach, &proto_drv};
                        }
                    }
                }
            }
        }
        return {};
    }

    // Find dashboard by its input pin ID
    inline dashboard::instance* find_dashboard_by_pin(dashboard::instance::list& dashboards, uint64_t pin_id) {
        if (!is_dashboard_pin(pin_id)) return nullptr;

        for (auto& dash : dashboards) {
            if (dashboard::input_pin_id(dash) == pin_id) {
                return &dash;
            }
        }
        return nullptr;
    }

    // Find config panel by its output pin ID
    inline panel::instance* find_panel_by_pin(panel::instance::list& panels, uint64_t pin_id) {
        if (!is_panel_pin(pin_id)) return nullptr;

        for (auto& panel : panels) {
            if (panel::output_pin_id(panel) == pin_id) {
                return &panel;
            }
        }
        return nullptr;
    }

    // Find client by its input pin ID
    inline client::instance* find_client_by_input_pin(client::instance::list& clients, uint64_t pin_id) {
        if (!client::is_client_input_pin(pin_id)) return nullptr;

        for (auto& cli : clients) {
            if (client::input_pin_id(cli) == pin_id) {
                return &cli;
            }
        }
        return nullptr;
    }

    // Find client and port by plug output pin ID
    struct client_plug_pin_info {
        client::instance* cli = nullptr;
        std::string port_uuid;
    };

    inline client_plug_pin_info find_client_by_plug_output_pin(
        client::instance::list& clients, machine::instance::list& machines, uint64_t pin_id) {
        if (!client::is_client_plug_output_pin(pin_id)) return {};

        for (auto& cli : clients) {
            if (!cli.is_connected()) continue;
            for (auto& mach : machines) {
                if (mach.uuid != cli.connected_machine_uuid) continue;
                for (const auto& drv : mach.drivers()) {
                    if (drv.uuid != cli.connected_driver_uuid) continue;
                    for (const auto& plug : drv.plugs) {
                        if (client::plug_output_pin_id(cli, plug) == pin_id) {
                            return {&cli, plug.uuid};
                        }
                    }
                }
            }
        }
        return {};
    }

    // Find client and port by socket output pin ID
    struct client_socket_pin_info {
        client::instance* cli = nullptr;
        std::string port_uuid;
    };

    inline client_socket_pin_info find_client_by_socket_output_pin(
        client::instance::list& clients, machine::instance::list& machines, uint64_t pin_id) {
        if (!client::is_client_socket_output_pin(pin_id)) return {};

        for (auto& cli : clients) {
            if (!cli.is_connected()) continue;
            for (auto& mach : machines) {
                if (mach.uuid != cli.connected_machine_uuid) continue;
                for (const auto& drv : mach.drivers()) {
                    if (drv.uuid != cli.connected_driver_uuid) continue;
                    for (const auto& socket : drv.sockets) {
                        if (client::socket_output_pin_id(cli, socket) == pin_id) {
                            return {&cli, socket.uuid};
                        }
                    }
                }
            }
        }
        return {};
    }

    inline void render_machine_node(machine::instance& mach, runtime::tracker& runtime_tracker,
                                    const network::object& net) {
        auto status = runtime_tracker.status_of(mach.machine_prototype().id());

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

        float padding = 40.0f;
        std::string type_text = mach.prototype_typename();
        float type_width = ui::text_size(type_text).x;
        float name_width = ui::text_size(mach.instance_name()).x;

        float max_driver_width = 0.0f;
        for (const auto& drv : mach.drivers()) {
            std::string drv_text = std::string(driver::type_to_icon(drv.backend_type)) + " " + drv.name;
            float drv_width = ui::text_size(drv_text).x + 30.0f;
            max_driver_width = std::max(max_driver_width, drv_width);
        }

        float total_width = std::max({type_width, name_width, max_driver_width}) + 2 * padding;
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
        ui::text(mach.instance_name());
        ui::style::color::pop();

        ui::dummy(0.0f, 10.0f);

        // Summary info
        size_t controller_count = mach.controllers().size();
        size_t driver_count = mach.drivers().size();
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
                status_icon = ui::icon::stop;
                status_text = "Stopped";
                status_color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                break;
            case runtime::status::building:
                status_icon = ui::icon::hammer;
                status_text = "Building...";
                status_color = ImVec4(1.0f, 0.7f, 0.0f, 1.0f);
                break;
            case runtime::status::running:
                status_icon = ui::icon::play;
                status_text = "Running";
                status_color = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);
                break;
            case runtime::status::error:
                status_icon = ui::icon::warning;
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

        // Driver output pins - one per driver (simple row: icon name [pin])
        if (!mach.drivers().empty()) {
            ui::separator("Drivers", total_width);
            for (const auto& drv : mach.drivers()) {
                float connector_size = 16.0f;
                float spacing = 10.0f;

                uint64_t output_pin = driver_output_pin_id(mach, drv);

                bool driver_connected = false;
                for (const auto& link : net.client_links) {
                    if (link.machine_uuid == mach.uuid && link.driver_uuid == drv.uuid) {
                        driver_connected = true;
                        break;
                    }
                }

                std::string drv_text = std::string(driver::type_to_icon(drv.backend_type)) + " " + drv.name;
                float drv_text_width = ui::text_size(drv_text).x;
                float drv_spacing = total_width - drv_text_width - connector_size - spacing;
                ui::dummy(drv_spacing > 0 ? drv_spacing : 0.0f, 0.0f);
                ui::same_line();
                ui::text(drv_text);
                ui::same_line();
                ui::dummy(spacing, 0.0f);
                ui::same_line();

                ed::BeginPin(ed::PinId(output_pin), ed::PinKind::Output);
                ui::connector::square(driver_connected);
                ed::EndPin();
            }
        }

        ui::dummy(0.0f, 10.0f);

        ui::id::pop();
        ui::node::end_node();

        ed::PopStyleColor(2);
    }

    inline void render_client_node(client::instance& cli, network::object& net,
                                    machine::instance::list& machines,
                                    runtime::tracker& runtime_tracker) {
        const driver::instance* connected_driver = nullptr;

        if (cli.is_connected()) {
            for (auto& mach : machines) {
                if (mach.uuid == cli.connected_machine_uuid) {
                    for (const auto& drv : mach.drivers()) {
                        if (drv.uuid == cli.connected_driver_uuid) {
                            connected_driver = &drv;
                            break;
                        }
                    }
                    break;
                }
            }
        }

        client::render_node(cli, connected_driver, net);
    }

    inline void render_dashboard_node(dashboard::instance& dash, network::object& net,
                                      machine::instance::list& machines,
                                      runtime::tracker& runtime_tracker) {
        const driver::instance* connected_driver = nullptr;
        std::map<uint64_t, std::vector<float>> signal_data;
        bool machine_is_running = false;
        bool is_linked = dash.is_connected();

        if (is_linked) {
            for (auto& mach : machines) {
                if (mach.uuid == dash.connected_machine_uuid) {
                    std::string exe_path = runtime::get_executable_path(mach.machine_prototype());
                    std::string current_cmd = terminal::current();
                    machine_is_running = !current_cmd.empty() && current_cmd.find(exe_path) != std::string::npos;

                    for (const auto& drv : mach.drivers()) {
                        if (drv.uuid == dash.connected_driver_uuid) {
                            connected_driver = &drv;

                            if (drv.backend_type == driver::type::udp) {
                                auto* existing_listener = dashboard::get_listener(dash.uuid);

                                if (machine_is_running) {
                                    if (!existing_listener || !existing_listener->keep_listening.load()) {
                                        std::string listen_ip = drv.backend_config.value("send_ip", "127.0.0.1");
                                        int listen_port = drv.backend_config.value("send_port", 8080);
                                        dashboard::start_listener(dash.uuid, listen_ip, listen_port);
                                        ui::cout << "Dashboard connected: listening on " << listen_ip << ":" << listen_port << ui::endl;
                                    }
                                } else {
                                    if (existing_listener && existing_listener->keep_listening.load()) {
                                        dashboard::pause_listener(dash.uuid);
                                        ui::cout << "Dashboard paused: machine stopped" << ui::endl;
                                    }
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
            }

            auto* listener = dashboard::get_listener(dash.uuid);
            if (listener) {
                signal_data = listener->get_data();
            }
        }

        dashboard::render_node(dash, connected_driver, signal_data, is_linked, machine_is_running);
    }

    inline void render_panel_node(panel::instance& pnl, network::object& net,
                                   machine::instance::list& machines,
                                   runtime::tracker& runtime_tracker) {
        const driver::instance* connected_driver = nullptr;
        panel::sender* udp_sender = nullptr;
        bool machine_is_running = false;
        bool is_linked = pnl.is_connected();

        if (is_linked) {
            for (auto& mach : machines) {
                if (mach.uuid == pnl.connected_machine_uuid) {
                    std::string exe_path = runtime::get_executable_path(mach.machine_prototype());
                    std::string current_cmd = terminal::current();
                    machine_is_running = !current_cmd.empty() && current_cmd.find(exe_path) != std::string::npos;

                    for (const auto& drv : mach.drivers()) {
                        if (drv.uuid == pnl.connected_driver_uuid) {
                            connected_driver = &drv;

                            if (drv.backend_type == driver::type::udp) {
                                auto* existing_sender = panel::get_sender(pnl.uuid);

                                if (machine_is_running) {
                                    if (!existing_sender || !existing_sender->is_configured()) {
                                        std::string send_ip = drv.backend_config.value("listen_ip", "127.0.0.1");
                                        int send_port = drv.backend_config.value("listen_port", 8081);
                                        panel::configure_sender(pnl.uuid, send_ip, send_port);
                                        ui::cout << "Panel connected: sending to " << send_ip << ":" << send_port << ui::endl;
                                    }
                                    udp_sender = panel::get_sender(pnl.uuid);
                                } else {
                                    if (existing_sender) {
                                        panel::remove_sender(pnl.uuid);
                                        ui::cout << "Panel disconnected: machine stopped" << ui::endl;
                                    }
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
            }
        }

        panel::render_node(pnl, connected_driver, is_linked, machine_is_running, udp_sender);
    }

    inline void handle_link_creation(network::object& net, machine::instance::list& machines) {
        if (ed::BeginCreate()) {
            ed::PinId start_pin, end_pin;
            if (ed::QueryNewLink(&start_pin, &end_pin) && start_pin && end_pin && start_pin != end_pin) {
                uint64_t start_id = start_pin.Get();
                uint64_t end_id = end_pin.Get();

                bool handled = false;

                // Case 1: Machine driver output <-> Client input
                {
                    uint64_t driver_pin_val = 0;
                    uint64_t client_pin_val = 0;

                    if (is_driver_output_pin(start_id) && client::is_client_input_pin(end_id)) {
                        driver_pin_val = start_id;
                        client_pin_val = end_id;
                    } else if (client::is_client_input_pin(start_id) && is_driver_output_pin(end_id)) {
                        client_pin_val = start_id;
                        driver_pin_val = end_id;
                    }

                    if (driver_pin_val && client_pin_val) {
                        auto info = find_by_driver_output_pin(machines, driver_pin_val);
                        auto* cli = find_client_by_input_pin(net.clients, client_pin_val);

                        if (info.machine && info.driver && cli) {
                            if (ed::AcceptNewItem()) {
                                // Remove existing client connection if any
                                if (cli->is_connected()) {
                                    // Cascade: remove dashboard/panel links that go through this client
                                    net.dashboard_links.remove_if([&cli](const dashboard_link& link) {
                                        if (link.machine_uuid == cli->connected_machine_uuid &&
                                            link.driver_uuid == cli->connected_driver_uuid) {
                                            dashboard::stop_listener(link.dashboard_uuid);
                                            return true;
                                        }
                                        return false;
                                    });
                                    net.panel_links.remove_if([&cli](const panel_link& link) {
                                        if (link.machine_uuid == cli->connected_machine_uuid &&
                                            link.driver_uuid == cli->connected_driver_uuid) {
                                            panel::remove_sender(link.panel_uuid);
                                            return true;
                                        }
                                        return false;
                                    });

                                    for (auto it = net.client_links.begin(); it != net.client_links.end(); ++it) {
                                        if (it->client_uuid == cli->uuid) {
                                            net.client_links.erase(it);
                                            break;
                                        }
                                    }
                                }

                                net.add_client_link(cli->uuid, info.machine->uuid, info.driver->uuid);
                                ui::cout << "Connected client to driver: " << info.driver->name << ui::endl;
                            }
                            handled = true;
                        }
                    }
                }

                // Case 2: Client plug/socket output <-> Dashboard input
                if (!handled) {
                    uint64_t client_output_pin = 0;
                    uint64_t dashboard_pin = 0;

                    auto is_client_output = [](uint64_t id) {
                        return client::is_client_plug_output_pin(id) || client::is_client_socket_output_pin(id);
                    };

                    if (is_client_output(start_id) && is_dashboard_pin(end_id)) {
                        client_output_pin = start_id;
                        dashboard_pin = end_id;
                    } else if (is_dashboard_pin(start_id) && is_client_output(end_id)) {
                        dashboard_pin = start_id;
                        client_output_pin = end_id;
                    }

                    if (client_output_pin && dashboard_pin) {
                        client::instance* resolved_cli = nullptr;
                        std::string resolved_port_uuid;

                        auto plug_info = find_client_by_plug_output_pin(net.clients, machines, client_output_pin);
                        if (plug_info.cli) {
                            resolved_cli = plug_info.cli;
                            resolved_port_uuid = plug_info.port_uuid;
                        } else {
                            auto socket_info = find_client_by_socket_output_pin(net.clients, machines, client_output_pin);
                            if (socket_info.cli) {
                                resolved_cli = socket_info.cli;
                                resolved_port_uuid = socket_info.port_uuid;
                            }
                        }

                        auto* dash = find_dashboard_by_pin(net.dashboards, dashboard_pin);

                        if (resolved_cli && dash) {
                            if (ed::AcceptNewItem()) {
                                // Remove existing dashboard connection if any
                                if (dash->is_connected()) {
                                    dashboard::stop_listener(dash->uuid);
                                    for (auto it = net.dashboard_links.begin(); it != net.dashboard_links.end(); ++it) {
                                        if (it->dashboard_uuid == dash->uuid) {
                                            net.dashboard_links.erase(it);
                                            break;
                                        }
                                    }
                                }

                                net.add_dashboard_link(dash->uuid,
                                    resolved_cli->connected_machine_uuid,
                                    resolved_cli->connected_driver_uuid,
                                    resolved_port_uuid);

                                // Find the driver to start listener
                                for (auto& mach : machines) {
                                    if (mach.uuid == resolved_cli->connected_machine_uuid) {
                                        for (const auto& drv : mach.drivers()) {
                                            if (drv.uuid == resolved_cli->connected_driver_uuid) {
                                                if (drv.backend_type == driver::type::udp) {
                                                    std::string listen_ip = drv.backend_config.value("send_ip", "127.0.0.1");
                                                    int listen_port = drv.backend_config.value("send_port", 8080);
                                                    dashboard::start_listener(dash->uuid, listen_ip, listen_port);
                                                }
                                                break;
                                            }
                                        }
                                        break;
                                    }
                                }

                                ui::cout << "Connected dashboard to client port" << ui::endl;
                            }
                            handled = true;
                        }
                    }
                }

                // Case 3: Panel output <-> Client socket output
                if (!handled) {
                    uint64_t panel_pin = 0;
                    uint64_t client_socket_pin = 0;

                    if (is_panel_pin(start_id) && client::is_client_socket_output_pin(end_id)) {
                        panel_pin = start_id;
                        client_socket_pin = end_id;
                    } else if (client::is_client_socket_output_pin(start_id) && is_panel_pin(end_id)) {
                        client_socket_pin = start_id;
                        panel_pin = end_id;
                    }

                    if (panel_pin && client_socket_pin) {
                        auto socket_info = find_client_by_socket_output_pin(net.clients, machines, client_socket_pin);
                        auto* pnl = find_panel_by_pin(net.panels, panel_pin);

                        if (socket_info.cli && pnl) {
                            if (ed::AcceptNewItem()) {
                                // Remove existing panel connection if any
                                if (pnl->is_connected()) {
                                    panel::remove_sender(pnl->uuid);
                                    for (auto it = net.panel_links.begin(); it != net.panel_links.end(); ++it) {
                                        if (it->panel_uuid == pnl->uuid) {
                                            net.panel_links.erase(it);
                                            break;
                                        }
                                    }
                                }

                                net.add_panel_link(pnl->uuid,
                                    socket_info.cli->connected_machine_uuid,
                                    socket_info.cli->connected_driver_uuid,
                                    socket_info.port_uuid);

                                // Find the driver to configure sender
                                for (auto& mach : machines) {
                                    if (mach.uuid == socket_info.cli->connected_machine_uuid) {
                                        for (const auto& drv : mach.drivers()) {
                                            if (drv.uuid == socket_info.cli->connected_driver_uuid) {
                                                if (drv.backend_type == driver::type::udp) {
                                                    std::string send_ip = drv.backend_config.value("listen_ip", "127.0.0.1");
                                                    int send_port = drv.backend_config.value("listen_port", 8081);
                                                    panel::configure_sender(pnl->uuid, send_ip, send_port);
                                                }
                                                break;
                                            }
                                        }
                                        break;
                                    }
                                }

                                ui::cout << "Connected panel to client socket" << ui::endl;
                            }
                            handled = true;
                        }
                    }
                }

                if (!handled) {
                    ed::RejectNewItem();
                }
            }
        }
        ed::EndCreate();
    }

    inline void handle_link_deletion(network::object& net) {
        if (ed::BeginDelete()) {
            ed::LinkId link_id;
            while (ed::QueryDeletedLink(&link_id)) {
                if (ed::AcceptDeletedItem()) {
                    uint64_t id = link_id.Get();

                    // Check client links
                    for (auto it = net.client_links.begin(); it != net.client_links.end(); ++it) {
                        if (it->id == id) {
                            auto* cli = net.find_client_by_uuid(it->client_uuid);

                            // Cascade: remove dashboard/panel links through this client
                            std::string mach_uuid = it->machine_uuid;
                            std::string drv_uuid = it->driver_uuid;
                            net.dashboard_links.remove_if([&](const dashboard_link& link) {
                                if (link.machine_uuid == mach_uuid && link.driver_uuid == drv_uuid) {
                                    dashboard::stop_listener(link.dashboard_uuid);
                                    auto* dash = net.find_dashboard_by_uuid(link.dashboard_uuid);
                                    if (dash) dash->disconnect();
                                    return true;
                                }
                                return false;
                            });
                            net.panel_links.remove_if([&](const panel_link& link) {
                                if (link.machine_uuid == mach_uuid && link.driver_uuid == drv_uuid) {
                                    panel::remove_sender(link.panel_uuid);
                                    auto* pnl = net.find_panel_by_uuid(link.panel_uuid);
                                    if (pnl) pnl->disconnect();
                                    return true;
                                }
                                return false;
                            });

                            if (cli) cli->disconnect();
                            net.client_links.erase(it);
                            ui::cout << "Disconnected client link" << ui::endl;
                            break;
                        }
                    }

                    // Check dashboard links
                    for (auto it = net.dashboard_links.begin(); it != net.dashboard_links.end(); ++it) {
                        if (it->id == id) {
                            dashboard::stop_listener(it->dashboard_uuid);
                            auto* dash = net.find_dashboard_by_uuid(it->dashboard_uuid);
                            if (dash) dash->disconnect();
                            net.dashboard_links.erase(it);
                            ui::cout << "Disconnected dashboard link" << ui::endl;
                            break;
                        }
                    }

                    // Check config panel links
                    for (auto it = net.panel_links.begin(); it != net.panel_links.end(); ++it) {
                        if (it->id == id) {
                            panel::remove_sender(it->panel_uuid);
                            auto* pnl = net.find_panel_by_uuid(it->panel_uuid);
                            if (pnl) pnl->disconnect();
                            net.panel_links.erase(it);
                            ui::cout << "Disconnected config panel link" << ui::endl;
                            break;
                        }
                    }
                }
            }

            // Handle node deletion
            ed::NodeId node_id;
            while (ed::QueryDeletedNode(&node_id)) {
                if (ed::AcceptDeletedItem()) {
                    uint64_t id = node_id.Get();

                    // Check if it's a client
                    auto* cli = net.find_client(id);
                    if (cli) {
                        // Cascade: remove dashboard/panel links through this client
                        if (cli->is_connected()) {
                            std::string mach_uuid = cli->connected_machine_uuid;
                            std::string drv_uuid = cli->connected_driver_uuid;
                            net.dashboard_links.remove_if([&](const dashboard_link& link) {
                                if (link.machine_uuid == mach_uuid && link.driver_uuid == drv_uuid) {
                                    dashboard::stop_listener(link.dashboard_uuid);
                                    auto* dash = net.find_dashboard_by_uuid(link.dashboard_uuid);
                                    if (dash) dash->disconnect();
                                    return true;
                                }
                                return false;
                            });
                            net.panel_links.remove_if([&](const panel_link& link) {
                                if (link.machine_uuid == mach_uuid && link.driver_uuid == drv_uuid) {
                                    panel::remove_sender(link.panel_uuid);
                                    auto* pnl = net.find_panel_by_uuid(link.panel_uuid);
                                    if (pnl) pnl->disconnect();
                                    return true;
                                }
                                return false;
                            });
                        }
                        net.client_links.remove_if([&cli](const client_link& link) {
                            return link.client_uuid == cli->uuid;
                        });
                        net.clients.remove_if([id](const client::instance& c) {
                            return c.id() == id;
                        });
                        ui::cout << "Deleted client" << ui::endl;
                    }

                    // Check if it's a dashboard
                    auto* dash = net.find_dashboard(id);
                    if (dash) {
                        dashboard::stop_listener(dash->uuid);
                        net.dashboard_links.remove_if([&dash](const dashboard_link& link) {
                            return link.dashboard_uuid == dash->uuid;
                        });
                        net.dashboards.remove_if([id](const dashboard::instance& d) {
                            return d.id() == id;
                        });
                        ui::cout << "Deleted dashboard" << ui::endl;
                    }

                    // Check if it's a config panel
                    auto* panel = net.find_panel(id);
                    if (panel) {
                        panel::remove_sender(panel->uuid);
                        net.panel_links.remove_if([&panel](const panel_link& link) {
                            return link.panel_uuid == panel->uuid;
                        });
                        net.panels.remove_if([id](const panel::instance& p) {
                            return p.id() == id;
                        });
                        ui::cout << "Deleted config panel" << ui::endl;
                    }
                }
            }
        }
        ed::EndDelete();
    }

    inline void render_all_links(network::object& net, machine::instance::list& machines) {
        // Render client links (machine driver output -> client input)
        for (const auto& link : net.client_links) {
            auto* cli = net.find_client_by_uuid(link.client_uuid);
            if (!cli) continue;

            uint64_t cli_pin = client::input_pin_id(*cli);
            uint64_t drv_pin = 0;

            for (auto& mach : machines) {
                if (mach.uuid == link.machine_uuid) {
                    for (const auto& drv : mach.drivers()) {
                        if (drv.uuid == link.driver_uuid) {
                            drv_pin = driver_output_pin_id(mach, drv);
                            break;
                        }
                    }
                    break;
                }
            }

            if (drv_pin) {
                ed::Link(ed::LinkId(link.id), ed::PinId(drv_pin), ed::PinId(cli_pin));
            }
        }

        // Render dashboard links (client plug output -> dashboard input)
        for (const auto& link : net.dashboard_links) {
            auto* dash = net.find_dashboard_by_uuid(link.dashboard_uuid);
            if (!dash) continue;

            uint64_t dash_pin = dashboard::input_pin_id(*dash);
            uint64_t src_pin = 0;

            // Find the client connected to this machine/driver
            auto* cli = net.find_client_for_driver(link.machine_uuid, link.driver_uuid);
            if (cli) {
                // Route through client plug or socket output pin
                for (auto& mach : machines) {
                    if (mach.uuid == link.machine_uuid) {
                        for (const auto& drv : mach.drivers()) {
                            if (drv.uuid == link.driver_uuid) {
                                for (const auto& plug : drv.plugs) {
                                    if (plug.uuid == link.port_uuid) {
                                        src_pin = client::plug_output_pin_id(*cli, plug);
                                        break;
                                    }
                                }
                                if (!src_pin) {
                                    for (const auto& socket : drv.sockets) {
                                        if (socket.uuid == link.port_uuid) {
                                            src_pin = client::socket_output_pin_id(*cli, socket);
                                            break;
                                        }
                                    }
                                }
                                break;
                            }
                        }
                        break;
                    }
                }
            }

            if (src_pin) {
                ed::Link(ed::LinkId(link.id + 0x20000000), ed::PinId(src_pin), ed::PinId(dash_pin));
            }
        }

        // Render config panel links (panel output -> client socket output)
        for (const auto& link : net.panel_links) {
            auto* pnl = net.find_panel_by_uuid(link.panel_uuid);
            if (!pnl) continue;

            uint64_t panel_pin = panel::output_pin_id(*pnl);
            uint64_t dst_pin = 0;

            // Find the client connected to this machine/driver
            auto* cli = net.find_client_for_driver(link.machine_uuid, link.driver_uuid);
            if (cli) {
                // Route through client socket output pin
                for (auto& mach : machines) {
                    if (mach.uuid == link.machine_uuid) {
                        for (const auto& drv : mach.drivers()) {
                            if (drv.uuid == link.driver_uuid) {
                                for (const auto& socket : drv.sockets) {
                                    if (socket.uuid == link.port_uuid) {
                                        dst_pin = client::socket_output_pin_id(*cli, socket);
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                        break;
                    }
                }
            }

            if (dst_pin) {
                ed::Link(ed::LinkId(link.id + 0x30000000), ed::PinId(panel_pin), ed::PinId(dst_pin));
            }
        }
    }

    inline void render_editor(network::object& net, const machine::object::list& machine_db,
                             runtime::tracker& runtime_tracker, ui::selection& sel) {
        ui::begin("Network Editor", 0);
        ui::node::context_begin(network_context_id);

        // Find currently selected item from shared selection
        machine::instance* selected_machine = find_machine_by_node_id(net.machines, sel.selected_id);
        client::instance* selected_client = net.find_client(sel.selected_id);
        dashboard::instance* selected_dashboard = net.find_dashboard(sel.selected_id);
        panel::instance* selected_panel = net.find_panel(sel.selected_id);

        // Horizontal split: Runtime and Toolbar side by side
        ui::columns(2);

        // Left column: Runtime controls
        ui::separator("Runtime");
        if (selected_machine) {
            machine::render_runtime_toolbar(const_cast<machine::object&>(selected_machine->machine_prototype()), runtime_tracker);
        } else {
            ImGui::TextDisabled("Select a machine to control");
        }

        // Right column: Toolbar controls
        ui::next_column();
        ui::separator("Toolbar");
        if (ui::button("Insert")) {
            ui::popup::open("InsertMenu");
        }
        if (ui::popup::begin("InsertMenu")) {
            if (ui::menu::begin("Machine")) {
                for (const auto& mach : machine_db) {
                    if (ui::menu::item(mach.display_name())) {
                        machine::instance new_instance(mach);
                        new_instance.name = mach.name + "_instance";
                        net.machines.push_back(new_instance);
                        ui::cout << "Added machine instance: " << new_instance.name << " (prototype: " << mach.display_name() << ")" << ui::endl;
                    }
                }
                ui::menu::end();
            }
            if (ui::menu::begin("Drivers")) {
                if (ui::menu::item(std::string(ui::icon::network_wired) + " udp::client")) {
                    client::instance new_client;
                    new_client.name = "udp_client_" + std::to_string(net.clients.size() + 1);
                    net.clients.push_back(new_client);
                    ui::cout << "Added UDP client: " << new_client.name << ui::endl;
                }
                ui::menu::end();
            }
            if (ui::menu::item(std::string(ui::icon::dashboard) + " Dashboard")) {
                dashboard::instance new_dashboard;
                new_dashboard.name = "dashboard_" + std::to_string(net.dashboards.size() + 1);
                net.dashboards.push_back(new_dashboard);
                ui::cout << "Added dashboard: " << new_dashboard.name << ui::endl;
            }
            if (ui::menu::item(std::string(ui::icon::panel) + " Panel")) {
                panel::instance new_panel;
                new_panel.name = "panel_" + std::to_string(net.panels.size() + 1);
                net.panels.push_back(new_panel);
                ui::cout << "Added panel: " << new_panel.name << ui::endl;
            }
            ui::popup::end();
        }

        ui::same_line();

        // Delete selected item
        bool has_selection = selected_machine || selected_client || selected_dashboard || selected_panel;
        if (!has_selection) {
            ui::disabled::begin();
        }
        if (ui::button(ui::icon::remove)) {
            if (selected_machine) {
                uint64_t id_to_remove = selected_machine->id();
                net.machines.remove_if([id_to_remove](const machine::instance& m) {
                    return m.id() == id_to_remove;
                });
                sel.clear();
            } else if (selected_client) {
                // Cascade: remove dashboard/panel links through this client
                if (selected_client->is_connected()) {
                    std::string mach_uuid = selected_client->connected_machine_uuid;
                    std::string drv_uuid = selected_client->connected_driver_uuid;
                    net.dashboard_links.remove_if([&](const dashboard_link& link) {
                        if (link.machine_uuid == mach_uuid && link.driver_uuid == drv_uuid) {
                            dashboard::stop_listener(link.dashboard_uuid);
                            auto* d = net.find_dashboard_by_uuid(link.dashboard_uuid);
                            if (d) d->disconnect();
                            return true;
                        }
                        return false;
                    });
                    net.panel_links.remove_if([&](const panel_link& link) {
                        if (link.machine_uuid == mach_uuid && link.driver_uuid == drv_uuid) {
                            panel::remove_sender(link.panel_uuid);
                            auto* p = net.find_panel_by_uuid(link.panel_uuid);
                            if (p) p->disconnect();
                            return true;
                        }
                        return false;
                    });
                }
                net.client_links.remove_if([&selected_client](const client_link& link) {
                    return link.client_uuid == selected_client->uuid;
                });
                uint64_t id_to_remove = selected_client->id();
                net.clients.remove_if([id_to_remove](const client::instance& c) {
                    return c.id() == id_to_remove;
                });
                sel.clear();
            } else if (selected_dashboard) {
                dashboard::stop_listener(selected_dashboard->uuid);
                net.dashboard_links.remove_if([&selected_dashboard](const dashboard_link& link) {
                    return link.dashboard_uuid == selected_dashboard->uuid;
                });
                uint64_t id_to_remove = selected_dashboard->id();
                net.dashboards.remove_if([id_to_remove](const dashboard::instance& d) {
                    return d.id() == id_to_remove;
                });
                sel.clear();
            } else if (selected_panel) {
                panel::remove_sender(selected_panel->uuid);
                net.panel_links.remove_if([&selected_panel](const panel_link& link) {
                    return link.panel_uuid == selected_panel->uuid;
                });
                uint64_t id_to_remove = selected_panel->id();
                net.panels.remove_if([id_to_remove](const panel::instance& p) {
                    return p.id() == id_to_remove;
                });
                sel.clear();
            }
        }
        if (!has_selection) {
            ui::disabled::end();
        }

        // Reset to single column for the node editor canvas
        ui::columns(1);
        ui::separator();

        ui::node::begin(std::string("network_") + std::to_string(network_context_id));

        // Sync node editor with shared selection
        if (sel.has_selection()) {
            auto* mach = find_machine_by_node_id(net.machines, sel.selected_id);
            auto* cli = net.find_client(sel.selected_id);
            auto* dash = net.find_dashboard(sel.selected_id);
            auto* panel = net.find_panel(sel.selected_id);
            if (mach || cli || dash || panel) {
                ed::SelectNode(ed::NodeId(sel.selected_id), false);
            } else {
                ed::ClearSelection();
            }
        } else {
            ed::ClearSelection();
        }

        ui::font::push(ui::font::type::code);

        // Render machine nodes
        for (auto& mach : net.machines) {
            render_machine_node(mach, runtime_tracker, net);
        }

        // Render client nodes
        for (auto& cli : net.clients) {
            render_client_node(cli, net, net.machines, runtime_tracker);
        }

        // Render dashboard nodes
        for (auto& dash : net.dashboards) {
            render_dashboard_node(dash, net, net.machines, runtime_tracker);
        }

        // Render config panel nodes
        for (auto& panel : net.panels) {
            render_panel_node(panel, net, net.machines, runtime_tracker);
        }

        ui::font::pop();

        // Render existing links
        render_all_links(net, net.machines);

        // Handle link creation/deletion
        handle_link_creation(net, net.machines);
        handle_link_deletion(net);

        if (net.machines.empty() && net.clients.empty() && net.dashboards.empty() && net.panels.empty()) {
            ImGui::TextDisabled("No machines, clients, dashboards, or panels in network");
        }

        // Handle click on empty canvas to deselect
        if (ed::GetBackgroundClickButtonIndex() == 0) {
            sel.clear();
        }

        // Handle double-click drill-down (only for machines)
        ed::NodeId double_clicked_node = ed::GetDoubleClickedNode();
        if (double_clicked_node && on_drill_down) {
            uint64_t id = double_clicked_node.Get();
            auto* mach = find_machine_by_node_id(net.machines, id);
            if (mach) {
                sel.clear();
                on_drill_down({ui::focus::level::machine, mach->uuid, mach->instance_name()});
            }
        }

        // Handle single click on node - update shared selection
        ed::NodeId clicked_node = ed::GetClickedNode();
        if (clicked_node) {
            uint64_t node_id = clicked_node.Get();
            sel.select(node_id);
        }

        ui::node::end();
        ui::node::context_end();
        ui::end();

        // Refresh selections after potential changes
        selected_machine = find_machine_by_node_id(net.machines, sel.selected_id);
        selected_client = net.find_client(sel.selected_id);
        selected_dashboard = net.find_dashboard(sel.selected_id);
        selected_panel = net.find_panel(sel.selected_id);

        // Instance window - show appropriate panel based on selection
        if (selected_machine) {
            ui::begin("Machine Instance");
            selected_machine->editor();
            ui::graph::render_list(ui::icon::controller, "Controllers", selected_machine->controllers());
            ui::graph::render_list(ui::icon::driver, "Drivers", selected_machine->drivers());
            ui::end();
        } else if (selected_client) {
            ui::begin("Client Instance");
            const driver::instance* client_driver = nullptr;
            if (selected_client->is_connected()) {
                for (auto& mach : net.machines) {
                    if (mach.uuid == selected_client->connected_machine_uuid) {
                        for (const auto& drv : mach.drivers()) {
                            if (drv.uuid == selected_client->connected_driver_uuid) {
                                client_driver = &drv;
                                break;
                            }
                        }
                        break;
                    }
                }
            }
            client::render_instance_editor(*selected_client, client_driver);
            ui::end();
        } else if (selected_dashboard) {
            ui::begin("Dashboard Instance");
            const driver::instance* dashboard_driver = nullptr;
            if (selected_dashboard->is_connected()) {
                for (auto& mach : net.machines) {
                    if (mach.uuid == selected_dashboard->connected_machine_uuid) {
                        for (const auto& drv : mach.drivers()) {
                            if (drv.uuid == selected_dashboard->connected_driver_uuid) {
                                dashboard_driver = &drv;
                                break;
                            }
                        }
                        break;
                    }
                }
            }
            dashboard::render_instance_editor(*selected_dashboard, dashboard_driver);
            ui::end();
        } else if (selected_panel) {
            ui::begin("Panel Instance");
            panel::render_instance_editor(*selected_panel);
            ui::end();
        } else {
            ui::begin("Network Instance");
            ui::font::push(ui::font::type::code);
            ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_typename);
            ui::separator("Network");
            ui::style::color::pop();
            ui::font::pop();

            ui::separator("Machines");
            for (auto& mach : net.machines) {
                ui::text(std::string(ui::icon::machine) + " ");
                ui::same_line();
                ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_typename);
                ui::text(mach.prototype_typename());
                ui::style::color::pop();
                ui::same_line();
                ui::text(" ");
                ui::same_line();
                ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_instance);
                ui::text(mach.instance_name());
                ui::style::color::pop();
            }

            ui::separator("Dashboards");
            for (auto& dash : net.dashboards) {
                ui::text(std::string(ui::icon::dashboard) + " ");
                ui::same_line();
                ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_instance);
                ui::text(dash.name);
                ui::style::color::pop();
                if (dash.is_connected()) {
                    ui::same_line();
                    ImGui::TextDisabled(" (connected)");
                }
            }
            ui::end();
        }
    }

} // namespace network
