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
#include "controls/dashboard/instance.hpp"
#include "controls/dashboard/listener.hpp"
#include "controls/driver/instance.hpp"
#include "controls/driver/type.hpp"
#include <imgui-node-editor/imgui_node_editor.h>
#include <implot.h>
#include <map>
#include <vector>

namespace dashboard {
    namespace ed = ax::NodeEditor;

    // Pin ID marker for dashboard input pins (uses high byte to avoid collision)
    inline constexpr uint64_t PIN_MARKER = 0xDA00000000000000ULL;

    inline uint64_t input_pin_id(const instance& dash) {
        // Clear high byte of dashboard ID before OR-ing with marker
        uint64_t base = dash.id() & 0x00FFFFFFFFFFFFFFULL;
        return base | PIN_MARKER;
    }

    // Render signal chart with name and value - similar to debug view
    inline void render_signal_chart(const std::string& label, uint64_t signal_id,
                                    const std::map<uint64_t, std::vector<float>>& signal_data,
                                    float width, float height) {
        auto it = signal_data.find(signal_id);

        // Signal name header
        if (it == signal_data.end() || it->second.empty()) {
            ImGui::Text("%s", label.c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("(no data)");
            return;
        }

        const auto& values = it->second;
        float current_value = values.back();

        // Signal name and current value
        ImGui::Text("%s: %.4f", label.c_str(), current_value);

        // Create x values for plotting
        std::vector<float> x_values(values.size());
        for (size_t i = 0; i < values.size(); ++i) {
            x_values[i] = static_cast<float>(i);
        }

        // Calculate y-axis limits with padding
        auto [min_it, max_it] = std::minmax_element(values.begin(), values.end());
        float min_val = *min_it;
        float max_val = *max_it;
        float range = max_val - min_val;
        if (range < 0.001f) range = 1.0f;  // Avoid zero range
        float padding = range * 0.1f;

        // Unique plot ID
        std::string plot_id = "##plot_" + std::to_string(signal_id);

        ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(8, 4));
        if (ImPlot::BeginPlot(plot_id.c_str(), ImVec2(width, height),
                              ImPlotFlags_NoTitle | ImPlotFlags_NoLegend | ImPlotFlags_NoMenus |
                              ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMouseText)) {
            ImPlot::SetupAxes(nullptr, nullptr,
                              ImPlotAxisFlags_NoDecorations,
                              ImPlotAxisFlags_NoDecorations);
            ImPlot::SetupAxesLimits(0, static_cast<double>(values.size()),
                                    min_val - padding, max_val + padding,
                                    ImPlotCond_Always);

            ImPlot::PlotLine(label.c_str(), x_values.data(), values.data(),
                            static_cast<int>(values.size()));

            ImPlot::EndPlot();
        }
        ImPlot::PopStyleVar();
    }

    // Render the dashboard node with embedded charts
    inline void render_node(instance& dash, const driver::instance* driver,
                           const std::map<uint64_t, std::vector<float>>& signal_data,
                           bool is_linked, bool is_active) {
        // Dashboard style - sharp corners for square appearance
        ed::PushStyleVar(ed::StyleVar_NodeRounding, 0.0f);
        ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(15, 15, 15, 15));
        ed::PushStyleColor(ed::StyleColor_NodeBg, ui::theme::vs2022::frame_background);

        // Border color based on state: green=active, yellow=linked but inactive, default=not linked
        ImVec4 border_color = ui::theme::vs2022::highlight;
        if (is_linked && is_active) {
            border_color = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);  // Green - active
        } else if (is_linked) {
            border_color = ImVec4(0.6f, 0.6f, 0.2f, 1.0f);  // Yellow/dim - linked but inactive
        }
        ed::PushStyleColor(ed::StyleColor_NodeBorder, border_color);

        ui::node::begin_node(dash.id());
        ui::id::push(dash.id());

        // Header
        ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_typename);
        ui::text(std::string(ui::icon::dashboard) + " Dashboard");
        ui::style::color::pop();

        ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_instance);
        ui::text(dash.name);
        ui::style::color::pop();

        ui::dummy(0.0f, 5.0f);

        // Input pin on left side
        ed::BeginPin(ed::PinId(input_pin_id(dash)), ed::PinKind::Input);
        ui::connector::diamond(is_linked);
        ed::EndPin();
        ui::same_line();

        // Connection status
        if (is_linked && driver) {
            if (is_active) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
                ui::text(std::string(ui::icon::circle) + " " + driver->name);
                ImGui::PopStyleColor();
            } else {
                ImGui::TextDisabled("%s %s (stopped)", ui::icon::circle, driver->name.c_str());
            }
        } else {
            ImGui::TextDisabled("Not linked");
        }

        ui::dummy(0.0f, 10.0f);

        // Content area - fixed size for square node
        float content_width = dash.node_size - 30.0f;  // Account for padding
        float chart_height = 60.0f;
        int max_charts = 5;

        if (driver && driver->backend_type == driver::type::udp) {
            // Show signal charts for UDP driver
            int chart_count = 0;

            // Count expected signals
            int expected_signals = 0;
            for (const auto& plug : driver->plugs) {
                expected_signals += plug.outgoing.size();
            }
            for (const auto& socket : driver->sockets) {
                expected_signals += socket.outgoing.size();
            }

            if (signal_data.empty()) {
                // Show listening status
                std::string listen_ip = driver->backend_config.value("send_ip", "127.0.0.1");
                int listen_port = driver->backend_config.value("send_port", 8080);
                ImGui::TextDisabled("Listening on %s:%d", listen_ip.c_str(), listen_port);
                ImGui::TextDisabled("Expecting %d signals", expected_signals);
                ImGui::TextDisabled("Waiting for data...");
            } else {
                // Show charts for plug outgoing signals (data FROM machine)
                for (const auto& plug : driver->plugs) {
                    for (const auto& signal : plug.outgoing) {
                        if (chart_count >= max_charts) break;
                        uint64_t signal_id = plug.id() | signal.id();
                        render_signal_chart(signal.name, signal_id, signal_data, content_width, chart_height);
                        chart_count++;
                    }
                }

                // Show charts for socket outgoing signals (data FROM machine)
                for (const auto& socket : driver->sockets) {
                    for (const auto& signal : socket.outgoing) {
                        if (chart_count >= max_charts) break;
                        uint64_t signal_id = socket.id() | signal.id();
                        render_signal_chart(signal.name, signal_id, signal_data, content_width, chart_height);
                        chart_count++;
                    }
                }

                if (chart_count == 0 && !signal_data.empty()) {
                    // Data received but signal IDs don't match expected
                    ImGui::TextDisabled("Received %zu signals (ID mismatch)", signal_data.size());
                    // Show what IDs we're receiving for debugging
                    for (const auto& [id, vals] : signal_data) {
                        if (!vals.empty()) {
                            ImGui::TextDisabled("  ID %llu: %.3f", static_cast<unsigned long long>(id), vals.back());
                        }
                    }
                }
            }
        } else if (driver && driver->backend_type == driver::type::config) {
            // Show config driver info
            ui::text("Config Driver");
            std::string config_file = driver->backend_config.value("config_file", "config.json");
            ui::text("File: " + config_file);
        } else if (!is_linked) {
            // Not connected - show placeholder
            ImGui::TextDisabled("Connect to a driver to view signals");
            ui::dummy(content_width, 200.0f);
        } else {
            ImGui::TextDisabled("Unknown driver type");
        }

        ui::id::pop();
        ui::node::end_node();

        ed::PopStyleColor(2);
        ed::PopStyleVar(2);
    }

    // Instance editor for properties panel
    inline void render_instance_editor(instance& dash) {
        ui::font::push(ui::font::type::code);
        ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_typename);
        ui::separator("Dashboard");
        ui::style::color::pop();
        ui::font::pop();

        ui::input::text("Name", dash.name);
        ImGui::SliderFloat("Node Size", &dash.node_size, 300.0f, 600.0f);

        if (dash.is_connected()) {
            ui::separator("Connection");
            ui::text("Machine: " + dash.connected_machine_uuid.substr(0, 8) + "...");
            ui::text("Driver: " + dash.connected_driver_uuid.substr(0, 8) + "...");
            if (ui::button("Disconnect")) {
                dash.disconnect();
            }
        } else {
            ImGui::TextDisabled("Not connected to any driver");
        }
    }

} // namespace dashboard
