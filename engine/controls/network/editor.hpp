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

    // Pin ID markers (use high byte to avoid collision)
    inline constexpr uint64_t DRIVER_OUTPUT_PIN_MARKER = 0xDC00000000000000ULL;  // Driver output (to dashboard)
    inline constexpr uint64_t DRIVER_INPUT_PIN_MARKER = 0xDD00000000000000ULL;   // Driver input (from config panel)

    // Generate unique pin ID for driver output (right side, connects to dashboard)
    inline uint64_t driver_output_pin_id(const machine::instance& mach, const driver::instance& drv) {
        uint64_t base = (mach.id() ^ drv.id()) & 0x00FFFFFFFFFFFFFFULL;
        return base | DRIVER_OUTPUT_PIN_MARKER;
    }

    // Generate unique pin ID for driver input (left side, connects from config panel)
    inline uint64_t driver_input_pin_id(const machine::instance& mach, const driver::instance& drv) {
        uint64_t base = (mach.id() ^ drv.id()) & 0x00FFFFFFFFFFFFFFULL;
        return base | DRIVER_INPUT_PIN_MARKER;
    }

    // Check if a pin ID is a driver output pin
    inline bool is_driver_output_pin(uint64_t pin_id) {
        return (pin_id & 0xFF00000000000000ULL) == DRIVER_OUTPUT_PIN_MARKER;
    }

    // Check if a pin ID is a driver input pin
    inline bool is_driver_input_pin(uint64_t pin_id) {
        return (pin_id & 0xFF00000000000000ULL) == DRIVER_INPUT_PIN_MARKER;
    }

    // Check if a pin ID is a dashboard pin
    inline bool is_dashboard_pin(uint64_t pin_id) {
        return (pin_id & 0xFF00000000000000ULL) == dashboard::PIN_MARKER;
    }

    // Check if a pin ID is a config panel pin
    inline bool is_panel_pin(uint64_t pin_id) {
        return (pin_id & 0xFF00000000000000ULL) == panel::PIN_MARKER;
    }

    inline machine::instance* find_machine_by_node_id(machine::instance::list& machines, uint64_t node_id) {
        for (auto& mach : machines) {
            if (mach.id() == node_id) return &mach;
        }
        return nullptr;
    }

    // Find machine and driver by driver output pin ID (for dashboard connections)
    inline std::pair<machine::instance*, driver::instance*> find_by_driver_output_pin(
        machine::instance::list& machines, uint64_t pin_id) {
        if (!is_driver_output_pin(pin_id)) return {nullptr, nullptr};

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
        return {nullptr, nullptr};
    }

    // Find machine and driver by driver input pin ID (for config panel connections)
    inline std::pair<machine::instance*, driver::instance*> find_by_driver_input_pin(
        machine::instance::list& machines, uint64_t pin_id) {
        if (!is_driver_input_pin(pin_id)) return {nullptr, nullptr};

        for (auto& mach : machines) {
            for (auto& drv : mach.drivers()) {
                if (driver_input_pin_id(mach, drv) == pin_id) {
                    auto& proto = const_cast<machine::object&>(mach.machine_prototype());
                    for (auto& proto_drv : proto.drivers) {
                        if (proto_drv.uuid == drv.uuid) {
                            return {&mach, &proto_drv};
                        }
                    }
                }
            }
        }
        return {nullptr, nullptr};
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

    inline void render_machine_node(machine::instance& mach, runtime::tracker& runtime_tracker,
                                    const network::object& net) {
        // Use the prototype's ID for runtime status (that's what runtime::run_machine uses)
        auto status = runtime_tracker.status_of(mach.machine_prototype().id());

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

        // Calculate widths for centering
        float padding = 40.0f;
        std::string type_text = mach.prototype_typename();
        float type_width = ui::text_size(type_text).x;
        float name_width = ui::text_size(mach.instance_name()).x;

        // Calculate max driver text width
        float max_driver_width = 0.0f;
        for (const auto& drv : mach.drivers()) {
            std::string drv_text = std::string(driver::type_to_icon(drv.backend_type)) + " " + drv.name;
            float drv_width = ui::text_size(drv_text).x + 30.0f; // Extra space for connector
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

        // Driver pins - full row: [input pin] [name] [output pin]
        if (!mach.drivers().empty()) {
            ui::separator("Drivers", total_width);
            for (const auto& drv : mach.drivers()) {
                uint64_t input_pin = driver_input_pin_id(mach, drv);
                uint64_t output_pin = driver_output_pin_id(mach, drv);

                // Check if this driver is connected to dashboard (output)
                bool output_connected = false;
                for (const auto& link : net.dashboard_links) {
                    if (link.machine_uuid == mach.uuid && link.driver_uuid == drv.uuid) {
                        output_connected = true;
                        break;
                    }
                }

                // Check if this driver is connected to config panel (input)
                bool input_connected = false;
                for (const auto& link : net.panel_links) {
                    if (link.machine_uuid == mach.uuid && link.driver_uuid == drv.uuid) {
                        input_connected = true;
                        break;
                    }
                }

                // Driver row: [input] [icon + name] [output]
                std::string drv_text = std::string(driver::type_to_icon(drv.backend_type)) + " " + drv.name;
                float drv_text_width = ui::text_size(drv_text).x;
                float connector_size = 16.0f;
                float spacing = 10.0f;
                float row_content_width = connector_size + spacing + drv_text_width + spacing + connector_size;
                float padding_left = (total_width - row_content_width) * 0.5f;

                // Input pin (left side)
                ed::BeginPin(ed::PinId(input_pin), ed::PinKind::Input);
                ui::connector::square(input_connected);
                ed::EndPin();

                ui::same_line();
                ui::dummy(padding_left > 0 ? padding_left : spacing, 0.0f);
                ui::same_line();

                // Driver name centered
                ui::text(drv_text);

                ui::same_line();
                ui::dummy(padding_left > 0 ? padding_left : spacing, 0.0f);
                ui::same_line();

                // Output pin (right side)
                ed::BeginPin(ed::PinId(output_pin), ed::PinKind::Output);
                ui::connector::square(output_connected);
                ed::EndPin();
            }
        }

        ui::dummy(0.0f, 10.0f);

        ui::id::pop();
        ui::node::end_node();

        ed::PopStyleColor(2);
    }

    inline void render_dashboard_node(dashboard::instance& dash, network::object& net,
                                      machine::instance::list& machines,
                                      runtime::tracker& runtime_tracker) {
        // Find connected driver if any
        const driver::instance* connected_driver = nullptr;
        std::map<uint64_t, std::vector<float>> signal_data;
        bool machine_is_running = false;
        bool is_linked = dash.is_connected();

        if (is_linked) {
            for (auto& mach : machines) {
                if (mach.uuid == dash.connected_machine_uuid) {
                    // Check if machine executable is actually running (not just status)
                    std::string exe_path = runtime::get_executable_path(mach.machine_prototype());
                    std::string current_cmd = terminal::current();
                    machine_is_running = !current_cmd.empty() && current_cmd.find(exe_path) != std::string::npos;

                    for (const auto& drv : mach.drivers()) {
                        if (drv.uuid == dash.connected_driver_uuid) {
                            connected_driver = &drv;

                            if (drv.backend_type == driver::type::udp) {
                                auto* existing_listener = dashboard::get_listener(dash.uuid);

                                if (machine_is_running) {
                                    // Start listener if machine is running and not already listening
                                    if (!existing_listener || !existing_listener->keep_listening.load()) {
                                        std::string listen_ip = drv.backend_config.value("send_ip", "127.0.0.1");
                                        int listen_port = drv.backend_config.value("send_port", 8080);
                                        dashboard::start_listener(dash.uuid, listen_ip, listen_port);
                                        ui::cout << "Dashboard connected: listening on " << listen_ip << ":" << listen_port << ui::endl;
                                    }
                                } else {
                                    // Pause listener if machine stopped (retain data)
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

            // Get signal data from listener (even when paused, to show retained data)
            auto* listener = dashboard::get_listener(dash.uuid);
            if (listener) {
                signal_data = listener->get_data();
            }
        }

        // Pass both link status and active status to render
        dashboard::render_node(dash, connected_driver, signal_data, is_linked, machine_is_running);
    }

    inline void handle_link_creation(network::object& net, machine::instance::list& machines) {
        if (ed::BeginCreate()) {
            ed::PinId start_pin, end_pin;
            if (ed::QueryNewLink(&start_pin, &end_pin) && start_pin && end_pin && start_pin != end_pin) {
                uint64_t start_id = start_pin.Get();
                uint64_t end_id = end_pin.Get();

                bool handled = false;

                // Check for dashboard -> driver output connection
                {
                    uint64_t dashboard_pin = 0;
                    uint64_t driver_pin_val = 0;

                    if (is_dashboard_pin(start_id) && is_driver_output_pin(end_id)) {
                        dashboard_pin = start_id;
                        driver_pin_val = end_id;
                    } else if (is_driver_output_pin(start_id) && is_dashboard_pin(end_id)) {
                        dashboard_pin = end_id;
                        driver_pin_val = start_id;
                    }

                    if (dashboard_pin && driver_pin_val) {
                        auto* dash = find_dashboard_by_pin(net.dashboards, dashboard_pin);
                        auto [mach, drv] = find_by_driver_output_pin(machines, driver_pin_val);

                        if (dash && mach && drv) {
                            if (ed::AcceptNewItem()) {
                                // Remove existing connection if any
                                if (dash->is_connected()) {
                                    dashboard::stop_listener(dash->uuid);
                                    for (auto it = net.dashboard_links.begin(); it != net.dashboard_links.end(); ++it) {
                                        if (it->dashboard_uuid == dash->uuid) {
                                            net.dashboard_links.erase(it);
                                            break;
                                        }
                                    }
                                }

                                net.add_dashboard_link(dash->uuid, mach->uuid, drv->uuid);

                                if (drv->backend_type == driver::type::udp) {
                                    std::string listen_ip = drv->backend_config.value("send_ip", "127.0.0.1");
                                    int listen_port = drv->backend_config.value("send_port", 8080);
                                    dashboard::start_listener(dash->uuid, listen_ip, listen_port);
                                }

                                ui::cout << "Connected dashboard to driver output: " << drv->name << ui::endl;
                            }
                            handled = true;
                        }
                    }
                }

                // Check for config panel -> driver input connection
                if (!handled) {
                    uint64_t panel_pin = 0;
                    uint64_t driver_pin_val = 0;

                    if (is_panel_pin(start_id) && is_driver_input_pin(end_id)) {
                        panel_pin = start_id;
                        driver_pin_val = end_id;
                    } else if (is_driver_input_pin(start_id) && is_panel_pin(end_id)) {
                        panel_pin = end_id;
                        driver_pin_val = start_id;
                    }

                    if (panel_pin && driver_pin_val) {
                        auto* panel = find_panel_by_pin(net.panels, panel_pin);
                        auto [mach, drv] = find_by_driver_input_pin(machines, driver_pin_val);

                        if (panel && mach && drv) {
                            if (ed::AcceptNewItem()) {
                                // Remove existing connection if any
                                if (panel->is_connected()) {
                                    panel::remove_sender(panel->uuid);
                                    for (auto it = net.panel_links.begin(); it != net.panel_links.end(); ++it) {
                                        if (it->panel_uuid == panel->uuid) {
                                            net.panel_links.erase(it);
                                            break;
                                        }
                                    }
                                }

                                net.add_panel_link(panel->uuid, mach->uuid, drv->uuid);

                                if (drv->backend_type == driver::type::udp) {
                                    std::string send_ip = drv->backend_config.value("listen_ip", "127.0.0.1");
                                    int send_port = drv->backend_config.value("listen_port", 8081);
                                    panel::configure_sender(panel->uuid, send_ip, send_port);
                                }

                                ui::cout << "Connected config panel to driver input: " << drv->name << ui::endl;
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
                            auto* panel = net.find_panel_by_uuid(it->panel_uuid);
                            if (panel) panel->disconnect();
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

    inline void render_panel_node(panel::instance& pnl, network::object& net,
                                   machine::instance::list& machines,
                                   runtime::tracker& runtime_tracker) {
        // Find connected driver if any
        const driver::instance* connected_driver = nullptr;
        panel::sender* udp_sender = nullptr;
        bool machine_is_running = false;
        bool is_linked = pnl.is_connected();

        if (is_linked) {
            for (auto& mach : machines) {
                if (mach.uuid == pnl.connected_machine_uuid) {
                    // Check if machine executable is actually running (not just status)
                    std::string exe_path = runtime::get_executable_path(mach.machine_prototype());
                    std::string current_cmd = terminal::current();
                    machine_is_running = !current_cmd.empty() && current_cmd.find(exe_path) != std::string::npos;

                    for (const auto& drv : mach.drivers()) {
                        if (drv.uuid == pnl.connected_driver_uuid) {
                            connected_driver = &drv;

                            if (drv.backend_type == driver::type::udp) {
                                auto* existing_sender = panel::get_sender(pnl.uuid);

                                if (machine_is_running) {
                                    // Configure sender if machine is running
                                    if (!existing_sender || !existing_sender->is_configured()) {
                                        std::string send_ip = drv.backend_config.value("listen_ip", "127.0.0.1");
                                        int send_port = drv.backend_config.value("listen_port", 8081);
                                        panel::configure_sender(pnl.uuid, send_ip, send_port);
                                        ui::cout << "Panel connected: sending to " << send_ip << ":" << send_port << ui::endl;
                                    }
                                    udp_sender = panel::get_sender(pnl.uuid);
                                } else {
                                    // Remove sender if machine is not running
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

    inline void render_all_links(const network::object& net, machine::instance::list& machines) {
        // Render dashboard links (driver output -> dashboard input)
        for (const auto& link : net.dashboard_links) {
            auto* dash = const_cast<network::object&>(net).find_dashboard_by_uuid(link.dashboard_uuid);
            if (!dash) continue;

            uint64_t dash_pin = dashboard::input_pin_id(*dash);
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
                ed::Link(ed::LinkId(link.id), ed::PinId(drv_pin), ed::PinId(dash_pin));
            }
        }

        // Render config panel links (config panel output -> driver input)
        for (const auto& link : net.panel_links) {
            auto* panel = const_cast<network::object&>(net).find_panel_by_uuid(link.panel_uuid);
            if (!panel) continue;

            uint64_t panel_pin = panel::output_pin_id(*panel);
            uint64_t drv_pin = 0;

            for (auto& mach : machines) {
                if (mach.uuid == link.machine_uuid) {
                    for (const auto& drv : mach.drivers()) {
                        if (drv.uuid == link.driver_uuid) {
                            drv_pin = driver_input_pin_id(mach, drv);
                            break;
                        }
                    }
                    break;
                }
            }

            if (drv_pin) {
                ed::Link(ed::LinkId(link.id + 0x10000000), ed::PinId(panel_pin), ed::PinId(drv_pin));
            }
        }
    }

    inline void render_editor(network::object& net, const machine::object::list& machine_db,
                             runtime::tracker& runtime_tracker, ui::selection& sel) {
        ui::begin("Network Editor", 0);
        ui::node::context_begin(network_context_id);

        // Find currently selected item from shared selection
        machine::instance* selected_machine = find_machine_by_node_id(net.machines, sel.selected_id);
        dashboard::instance* selected_dashboard = net.find_dashboard(sel.selected_id);
        panel::instance* selected_panel = net.find_panel(sel.selected_id);

        // Horizontal split: Runtime and Toolbar side by side
        ui::columns(2);

        // Left column: Runtime controls
        ui::separator("Runtime");
        if (selected_machine) {
            // Runtime toolbar works with the machine prototype (definition)
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
                        // Create an instance of the machine with a default name
                        machine::instance new_instance(mach);
                        new_instance.name = mach.name + "_instance";
                        net.machines.push_back(new_instance);
                        ui::cout << "Added machine instance: " << new_instance.name << " (prototype: " << mach.display_name() << ")" << ui::endl;
                    }
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

        // Delete selected item (machine, dashboard, or panel)
        bool has_selection = selected_machine != nullptr || selected_dashboard != nullptr || selected_panel != nullptr;
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

        // Sync node editor with shared selection - selection is the single source of truth
        if (sel.has_selection()) {
            auto* mach = find_machine_by_node_id(net.machines, sel.selected_id);
            auto* dash = net.find_dashboard(sel.selected_id);
            auto* panel = net.find_panel(sel.selected_id);
            if (mach || dash || panel) {
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

        // Debug: Show machine count in toolbar area
        if (net.machines.empty() && net.dashboards.empty() && net.panels.empty()) {
            ImGui::TextDisabled("No machines, dashboards, or panels in network");
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
        selected_dashboard = net.find_dashboard(sel.selected_id);
        selected_panel = net.find_panel(sel.selected_id);

        // Instance window - show appropriate panel based on selection
        if (selected_machine) {
            ui::begin("Machine Instance");
            selected_machine->editor();
            ui::graph::render_list(ui::icon::controller, "Controllers", selected_machine->controllers());
            ui::graph::render_list(ui::icon::driver, "Drivers", selected_machine->drivers());
            ui::end();
        } else if (selected_dashboard) {
            ui::begin("Dashboard Instance");
            dashboard::render_instance_editor(*selected_dashboard);
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
