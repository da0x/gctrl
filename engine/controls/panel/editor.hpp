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
#include "controls/panel/instance.hpp"
#include "controls/panel/sender.hpp"
#include "controls/driver/instance.hpp"
#include "controls/driver/type.hpp"
#include <imgui-node-editor/imgui_node_editor.h>
#include <map>
#include <vector>

namespace panel {
    namespace ed = ax::NodeEditor;

    // Pin ID marker for config panel output pins (uses high byte to avoid collision)
    inline constexpr uint64_t PIN_MARKER = 0xCF00000000000000ULL;

    inline uint64_t output_pin_id(const instance& panel) {
        // Clear high byte of panel ID before OR-ing with marker
        uint64_t base = panel.id() & 0x00FFFFFFFFFFFFFFULL;
        return base | PIN_MARKER;
    }

    // Render input control for a signal value
    inline bool render_signal_input(const std::string& label, uint64_t signal_id, instance& panel,
                                    float width) {
        float value = panel.value(signal_id);

        // Calculate sizes for right alignment
        std::string btn_id = std::string(ui::icon::send) + "##send_" + std::to_string(signal_id);
        float label_width = ImGui::CalcTextSize(label.c_str()).x;
        float btn_width = ImGui::CalcTextSize(ui::icon::send).x + ImGui::GetStyle().FramePadding.x * 2 + 4;
        float right_side_width = label_width + btn_width + ImGui::GetStyle().ItemSpacing.x * 2;
        float input_width = width - right_side_width - 10;

        ImGui::PushItemWidth(input_width);
        std::string input_id = "##input_" + std::to_string(signal_id);

        // Always save value as user types (no EnterReturnsTrue)
        bool edited = ImGui::InputFloat(input_id.c_str(), &value, 0.0f, 0.0f, "%.3f");
        if (edited) {
            panel.value(signal_id, value);
        }
        ImGui::PopItemWidth();

        // Right-align label and button
        ImGui::SameLine(width - right_side_width);
        ImGui::Text("%s", label.c_str());

        ImGui::SameLine();
        bool send_clicked = ImGui::SmallButton(btn_id.c_str());
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Send value");
        }

        return send_clicked;
    }

    // Render the panel node with input fields
    inline void render_node(instance& panel, const driver::instance* driver,
                           bool is_linked, bool is_active, sender* udp_sender) {
        // Panel style - sharp corners for square appearance
        ed::PushStyleVar(ed::StyleVar_NodeRounding, 0.0f);
        ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(15, 15, 15, 15));
        ed::PushStyleColor(ed::StyleColor_NodeBg, ui::theme::vs2022::frame_background);

        // Border color based on state: green=active, orange/dim=linked but inactive, orange=not linked
        ImVec4 border_color = ImVec4(0.8f, 0.6f, 0.2f, 1.0f);  // Orange default
        if (is_linked && is_active) {
            border_color = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);  // Green - active
        } else if (is_linked) {
            border_color = ImVec4(0.6f, 0.6f, 0.2f, 1.0f);  // Yellow/dim - linked but inactive
        }
        ed::PushStyleColor(ed::StyleColor_NodeBorder, border_color);

        ui::node::begin_node(panel.id());
        ui::id::push(panel.id());

        // Header
        ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_typename);
        ui::text(std::string(ui::icon::panel) + " Panel");
        ui::style::color::pop();

        ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_instance);
        ui::text(panel.name);
        ui::style::color::pop();

        ui::dummy(0.0f, 5.0f);

        // Connection status row with output pin on right
        float content_width_full = panel.node_size - 30.0f;

        // Connection status text
        std::string status_text;
        if (is_linked && driver) {
            if (is_active) {
                status_text = std::string(ui::icon::circle) + " " + driver->name;
            } else {
                status_text = std::string(ui::icon::circle) + " " + driver->name + " (stopped)";
            }
        } else {
            status_text = "Not linked";
        }

        // Calculate position for right-aligned pin
        float text_width = ImGui::CalcTextSize(status_text.c_str()).x;
        float pin_width = 20.0f;  // Approximate pin width
        float spacing = content_width_full - text_width - pin_width;

        if (is_linked && driver) {
            if (is_active) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
                ui::text(status_text);
                ImGui::PopStyleColor();
            } else {
                ImGui::TextDisabled("%s", status_text.c_str());
            }
        } else {
            ImGui::TextDisabled("%s", status_text.c_str());
        }

        // Add spacing to push pin to the right
        ui::same_line();
        if (spacing > 0) {
            ui::dummy(spacing, 0.0f);
            ui::same_line();
        }

        // Output pin on right side
        ed::BeginPin(ed::PinId(output_pin_id(panel)), ed::PinKind::Output);
        ui::connector::diamond(is_linked);
        ed::EndPin();

        ui::dummy(0.0f, 10.0f);

        // Content area
        float content_width = panel.node_size - 30.0f;

        if (driver && driver->backend_type == driver::type::udp) {
            // Show input controls for driver's input signals (sockets incoming, plugs incoming)
            int control_count = 0;
            int max_controls = 8;
            std::vector<uint64_t> all_signal_ids;

            // Get send configuration
            std::string send_ip = driver->backend_config.value("listen_ip", "127.0.0.1");
            int send_port = driver->backend_config.value("listen_port", 8081);

            ImGui::TextDisabled("Sending to %s:%d", send_ip.c_str(), send_port);
            ui::dummy(0.0f, 5.0f);

            // Sockets have incoming signals that we can send TO the machine
            for (const auto& socket : driver->sockets) {
                for (const auto& signal : socket.incoming) {
                    if (control_count >= max_controls) break;
                    uint64_t signal_id = socket.id() | signal.id();
                    all_signal_ids.push_back(signal_id);

                    if (render_signal_input(signal.name, signal_id, panel, content_width)) {
                        // Value changed, send it
                        if (udp_sender) {
                            udp_sender->send(signal_id, panel.value(signal_id));
                        }
                    }
                    control_count++;
                }
            }

            // Plugs have incoming signals too
            for (const auto& plug : driver->plugs) {
                for (const auto& signal : plug.incoming) {
                    if (control_count >= max_controls) break;
                    uint64_t signal_id = plug.id() | signal.id();
                    all_signal_ids.push_back(signal_id);

                    if (render_signal_input(signal.name, signal_id, panel, content_width)) {
                        // Value changed, send it
                        if (udp_sender) {
                            udp_sender->send(signal_id, panel.value(signal_id));
                        }
                    }
                    control_count++;
                }
            }

            if (control_count == 0) {
                ImGui::TextDisabled("No input signals");
            } else {
                // Send All button
                ui::dummy(0.0f, 5.0f);
                float btn_width = ImGui::CalcTextSize("Send All").x + ImGui::GetStyle().FramePadding.x * 2 + 20;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + content_width - btn_width);
                if (ImGui::Button((std::string(ui::icon::send) + " Send All").c_str())) {
                    if (udp_sender) {
                        for (uint64_t sig_id : all_signal_ids) {
                            udp_sender->send(sig_id, panel.value(sig_id));
                        }
                    }
                }
            }
        } else if (driver && driver->backend_type == driver::type::config) {
            ui::text("Config Driver");
            std::string config_file = driver->backend_config.value("config_file", "config.json");
            ui::text("File: " + config_file);
        } else if (!is_linked) {
            ImGui::TextDisabled("Connect to a driver to send commands");
            ui::dummy(content_width, 150.0f);
        } else {
            ImGui::TextDisabled("Unknown driver type");
        }

        ui::id::pop();
        ui::node::end_node();

        ed::PopStyleColor(2);
        ed::PopStyleVar(2);
    }

    // Instance editor for properties panel
    inline void render_instance_editor(instance& panel) {
        ui::font::push(ui::font::type::code);
        ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_typename);
        ui::separator("Config Panel");
        ui::style::color::pop();
        ui::font::pop();

        ui::input::text("Name", panel.name);
        ImGui::SliderFloat("Node Size", &panel.node_size, 300.0f, 600.0f);

        if (panel.is_connected()) {
            ui::separator("Connection");
            ui::text("Machine: " + panel.connected_machine_uuid.substr(0, 8) + "...");
            ui::text("Driver: " + panel.connected_driver_uuid.substr(0, 8) + "...");
            if (ui::button("Disconnect")) {
                panel.disconnect();
            }
        } else {
            ImGui::TextDisabled("Not connected to any driver");
        }
    }

} // namespace panel
