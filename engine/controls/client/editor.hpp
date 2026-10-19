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
#include "ui/connector.hpp"
#include "controls/client/instance.hpp"
#include "controls/driver/instance.hpp"
#include "controls/driver/type.hpp"
#include <imgui-node-editor/imgui_node_editor.h>

namespace client {
    namespace ed = ax::NodeEditor;

    // Pin ID markers for client pins
    inline constexpr uint64_t INPUT_PIN_MARKER        = 0xC100000000000000ULL;
    inline constexpr uint64_t PLUG_OUTPUT_PIN_MARKER   = 0xC200000000000000ULL;
    inline constexpr uint64_t SOCKET_OUTPUT_PIN_MARKER  = 0xC300000000000000ULL;

    inline uint64_t input_pin_id(const instance& cli) {
        uint64_t base = cli.id() & 0x00FFFFFFFFFFFFFFULL;
        return base | INPUT_PIN_MARKER;
    }

    inline uint64_t plug_output_pin_id(const instance& cli, const port::plug::instance& plug) {
        uint64_t base = (cli.id() ^ plug.id()) & 0x00FFFFFFFFFFFFFFULL;
        return base | PLUG_OUTPUT_PIN_MARKER;
    }

    inline uint64_t socket_output_pin_id(const instance& cli, const port::socket::instance& socket) {
        uint64_t base = (cli.id() ^ socket.id()) & 0x00FFFFFFFFFFFFFFULL;
        return base | SOCKET_OUTPUT_PIN_MARKER;
    }

    inline bool is_client_input_pin(uint64_t pin_id) {
        return (pin_id & 0xFF00000000000000ULL) == INPUT_PIN_MARKER;
    }

    inline bool is_client_plug_output_pin(uint64_t pin_id) {
        return (pin_id & 0xFF00000000000000ULL) == PLUG_OUTPUT_PIN_MARKER;
    }

    inline bool is_client_socket_output_pin(uint64_t pin_id) {
        return (pin_id & 0xFF00000000000000ULL) == SOCKET_OUTPUT_PIN_MARKER;
    }

    inline void render_node(instance& cli, const driver::instance* driver,
                            const network::object& net) {
        ed::PushStyleVar(ed::StyleVar_NodeRounding, 0.0f);
        ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(15, 15, 15, 15));
        ed::PushStyleColor(ed::StyleColor_NodeBg, ui::theme::vs2022::frame_background);

        ImVec4 border_color = ImVec4(0.4f, 0.6f, 0.8f, 1.0f);
        if (cli.is_connected()) {
            border_color = ImVec4(0.3f, 0.7f, 0.9f, 1.0f);
        }
        ed::PushStyleColor(ed::StyleColor_NodeBorder, border_color);

        ui::node::begin_node(cli.id());
        ui::id::push(cli.id());

        float total_width = 250.0f;

        // Header
        ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_typename);
        ui::text(std::string(ui::icon::network_wired) + " udp::client");
        ui::style::color::pop();

        ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_instance);
        ui::text(cli.name);
        ui::style::color::pop();

        ui::dummy(total_width, 0.0f);
        ui::dummy(0.0f, 5.0f);

        // Machine-facing input pin (left side)
        ed::BeginPin(ed::PinId(input_pin_id(cli)), ed::PinKind::Input);
        ui::connector::square(cli.is_connected());
        ed::EndPin();
        ui::same_line();
        ui::dummy(10.0f, 0.0f);
        ui::same_line();

        if (cli.is_connected() && driver) {
            ui::text(std::string(driver::type_to_icon(driver->backend_type)) + " " + driver->name);
        } else {
            ImGui::TextDisabled("Not connected");
        }

        if (cli.is_connected() && driver) {
            ui::dummy(0.0f, 5.0f);

            float connector_size = 16.0f;
            float spacing = 10.0f;

            // Sockets - output pins on the right side (dashboards and panels connect here)
            for (const auto& socket : driver->sockets) {
                uint64_t pin = socket_output_pin_id(cli, socket);

                bool port_connected = false;
                for (const auto& link : net.dashboard_links) {
                    if (link.machine_uuid == cli.connected_machine_uuid &&
                        link.driver_uuid == cli.connected_driver_uuid &&
                        link.port_uuid == socket.uuid) {
                        port_connected = true;
                        break;
                    }
                }
                if (!port_connected) {
                    for (const auto& link : net.panel_links) {
                        if (link.machine_uuid == cli.connected_machine_uuid &&
                            link.driver_uuid == cli.connected_driver_uuid &&
                            link.port_uuid == socket.uuid) {
                            port_connected = true;
                            break;
                        }
                    }
                }

                std::string socket_text = socket.instance_name() + " " + std::string(ui::icon::socket);
                float socket_text_width = ui::text_size(socket_text).x;
                float socket_spacing = total_width - socket_text_width - connector_size - spacing;
                ui::dummy(socket_spacing > 0 ? socket_spacing : 0.0f, 0.0f);
                ui::same_line();
                ui::text(socket_text);
                ui::same_line();
                ui::dummy(spacing, 0.0f);
                ui::same_line();

                ed::BeginPin(ed::PinId(pin), ed::PinKind::Output);
                ui::connector::diamond(port_connected);
                ed::EndPin();
            }

            // Plugs - output pins on the right side (dashboards connect here)
            for (const auto& plug : driver->plugs) {
                uint64_t pin = plug_output_pin_id(cli, plug);

                bool port_connected = false;
                for (const auto& link : net.dashboard_links) {
                    if (link.machine_uuid == cli.connected_machine_uuid &&
                        link.driver_uuid == cli.connected_driver_uuid &&
                        link.port_uuid == plug.uuid) {
                        port_connected = true;
                        break;
                    }
                }

                std::string plug_text = plug.instance_name() + " " + std::string(ui::icon::plug);
                float plug_text_width = ui::text_size(plug_text).x;
                float plug_spacing = total_width - plug_text_width - connector_size - spacing;
                ui::dummy(plug_spacing > 0 ? plug_spacing : 0.0f, 0.0f);
                ui::same_line();
                ui::text(plug_text);
                ui::same_line();
                ui::dummy(spacing, 0.0f);
                ui::same_line();

                ed::BeginPin(ed::PinId(pin), ed::PinKind::Output);
                ui::connector::diamond(port_connected);
                ed::EndPin();
            }
        }

        ui::dummy(0.0f, 10.0f);

        ui::id::pop();
        ui::node::end_node();

        ed::PopStyleColor(2);
        ed::PopStyleVar(2);
    }

    inline void render_instance_editor(instance& cli, const driver::instance* driver) {
        ui::font::push(ui::font::type::code);
        ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_typename);
        ui::separator("UDP Client");
        ui::style::color::pop();
        ui::font::pop();

        ui::input::text("Name", cli.name);

        if (cli.is_connected()) {
            ui::separator("Connection");
            ui::text("Machine: " + cli.connected_machine_uuid.substr(0, 8) + "...");
            ui::text("Driver: " + cli.connected_driver_uuid.substr(0, 8) + "...");
            if (driver) {
                ui::text("Driver Name: " + driver->name);
            }
            if (ui::button("Disconnect")) {
                cli.disconnect();
            }
        } else {
            ImGui::TextDisabled("Not connected to any driver");
        }
    }

} // namespace client
