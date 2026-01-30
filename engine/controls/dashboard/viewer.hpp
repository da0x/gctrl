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
#include "controls/dashboard/instance.hpp"
#include "controls/dashboard/listener.hpp"
#include "controls/driver/instance.hpp"
#include "controls/driver/type.hpp"
#include <implot.h>
#include <map>
#include <vector>
#include <string>

namespace dashboard {

    // Signal metadata for display
    struct signal_info {
        std::string name;
        std::string type;
        std::string port_name;
        std::string port_type;  // "plug" or "socket"
        uint64_t signal_id;
        uint64_t port_id;
    };

    // Chart settings per signal
    struct chart_settings {
        bool auto_scale = true;
        float y_min = -1.0f;
        float y_max = 1.0f;
        float y_center = 0.0f;
    };

    // State for the dashboard viewer window
    struct viewer_state {
        bool open = false;
        std::string dashboard_uuid;
        std::string dashboard_name;
        const driver::instance* driver = nullptr;
        std::vector<signal_info> signals;
        std::map<uint64_t, chart_settings> settings;
    };

    inline viewer_state current_viewer;

    // Build signal info list from driver, grouped by port
    inline void build_signal_list(const driver::instance* drv) {
        current_viewer.signals.clear();
        if (!drv) return;

        // Collect outgoing signals from plugs (data FROM machine)
        for (const auto& plug : drv->plugs) {
            for (const auto& signal : plug.outgoing) {
                signal_info info;
                info.name = signal.name;
                info.type = signal.prototype_typename();
                info.port_name = plug.name;
                info.port_type = "plug";
                info.signal_id = plug.id() | signal.id();
                info.port_id = plug.id();
                current_viewer.signals.push_back(info);
            }
        }

        // Collect outgoing signals from sockets (data FROM machine)
        for (const auto& socket : drv->sockets) {
            for (const auto& signal : socket.outgoing) {
                signal_info info;
                info.name = signal.name;
                info.type = signal.prototype_typename();
                info.port_name = socket.name;
                info.port_type = "socket";
                info.signal_id = socket.id() | signal.id();
                info.port_id = socket.id();
                current_viewer.signals.push_back(info);
            }
        }
    }

    // Open the viewer for a specific dashboard
    inline void open_viewer(const instance& dash, const driver::instance* drv) {
        current_viewer.open = true;
        current_viewer.dashboard_uuid = dash.uuid;
        current_viewer.dashboard_name = dash.name;
        current_viewer.driver = drv;
        build_signal_list(drv);
    }

    // Close the viewer
    inline void close_viewer() {
        current_viewer.open = false;
        current_viewer.dashboard_uuid.clear();
        current_viewer.driver = nullptr;
        current_viewer.signals.clear();
    }

    // Render a single signal chart with controls
    inline void render_signal_chart(const signal_info& info,
                                    const std::map<uint64_t, std::vector<float>>& signal_data,
                                    float chart_height) {
        // Ensure settings exist for this signal
        if (current_viewer.settings.find(info.signal_id) == current_viewer.settings.end()) {
            current_viewer.settings[info.signal_id] = chart_settings();
        }
        auto& settings = current_viewer.settings[info.signal_id];

        auto it = signal_data.find(info.signal_id);

        // Header with signal type and name
        ImGui::PushStyleColor(ImGuiCol_Text, ui::theme::vs2022::code_typename);
        ImGui::Text("%s", info.type.c_str());
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Text("%s", info.name.c_str());

        if (it == signal_data.end() || it->second.empty()) {
            ImGui::SameLine();
            ImGui::TextDisabled("- no data");
            ImGui::Dummy(ImVec2(0, chart_height));
            return;
        }

        const auto& values = it->second;
        float current_value = values.back();

        // Current value
        ImGui::SameLine();
        ImGui::Text("= %.6f", current_value);

        // Create x values for plotting
        std::vector<float> x_values(values.size());
        for (size_t i = 0; i < values.size(); ++i) {
            x_values[i] = static_cast<float>(i);
        }

        // Calculate y-axis limits
        auto [min_it, max_it] = std::minmax_element(values.begin(), values.end());
        float data_min = *min_it;
        float data_max = *max_it;
        float range = data_max - data_min;
        if (range < 0.0001f) range = 1.0f;
        float padding = range * 0.2f;

        if (settings.auto_scale) {
            settings.y_min = data_min - padding;
            settings.y_max = data_max + padding;
            settings.y_center = (data_min + data_max) / 2.0f;
        }

        // Layout: 85% plot, 15% controls
        float total_width = ImGui::GetContentRegionAvail().x;
        float plot_width = total_width * 0.85f;
        float form_width = total_width * 0.15f - 10.0f;

        // Plot area
        std::string plot_child_id = "PlotArea##" + std::to_string(info.signal_id);
        ImGui::BeginChild(plot_child_id.c_str(), ImVec2(plot_width, chart_height), true, ImGuiWindowFlags_NoScrollbar);

        std::string plot_id = "##plot_" + std::to_string(info.signal_id);
        if (ImPlot::BeginPlot(plot_id.c_str(), ImVec2(-1, -1), ImPlotFlags_NoTitle | ImPlotFlags_NoLegend)) {
            ImPlot::SetupAxesLimits(
                0, static_cast<double>(values.size()),
                settings.y_min, settings.y_max,
                ImPlotCond_Always);

            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.0f, 0.8f, 0.4f, 1.0f));
            ImPlot::PlotLine(info.name.c_str(), x_values.data(), values.data(),
                            static_cast<int>(values.size()));
            ImPlot::PopStyleColor();

            ImPlot::EndPlot();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Controls area
        std::string form_child_id = "FormArea##" + std::to_string(info.signal_id);
        ImGui::BeginChild(form_child_id.c_str(), ImVec2(form_width, chart_height), true, ImGuiWindowFlags_NoScrollbar);

        std::string auto_id = "Auto##" + std::to_string(info.signal_id);
        ImGui::Checkbox(auto_id.c_str(), &settings.auto_scale);

        if (!settings.auto_scale) {
            ImGui::SetNextItemWidth(-1);
            std::string max_id = "##max" + std::to_string(info.signal_id);
            ImGui::InputFloat(max_id.c_str(), &settings.y_max, 0, 0, "%.2f");
            ui::tooltip("Y-Max");

            ImGui::SetNextItemWidth(-1);
            std::string center_id = "##center" + std::to_string(info.signal_id);
            ImGui::InputFloat(center_id.c_str(), &settings.y_center, 0, 0, "%.2f");
            ui::tooltip("Y-Center");

            ImGui::SetNextItemWidth(-1);
            std::string min_id = "##min" + std::to_string(info.signal_id);
            ImGui::InputFloat(min_id.c_str(), &settings.y_min, 0, 0, "%.2f");
            ui::tooltip("Y-Min");
        }

        ImGui::EndChild();
    }

    // Render the viewer window
    inline void render_viewer() {
        if (!current_viewer.open) return;

        ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);

        std::string title = std::string(ui::icon::dashboard) + " " + current_viewer.dashboard_name + " - Signal Monitor";

        if (ImGui::Begin(title.c_str(), &current_viewer.open)) {
            // Get signal data from listener
            std::map<uint64_t, std::vector<float>> signal_data;
            auto* listener = get_listener(current_viewer.dashboard_uuid);
            if (listener) {
                signal_data = listener->get_data();
            }

            // Connection status
            if (current_viewer.driver) {
                ImGui::Text("Driver: %s (%s)",
                    current_viewer.driver->name.c_str(),
                    driver::type_to_string(current_viewer.driver->backend_type).c_str());

                if (current_viewer.driver->backend_type == driver::type::udp) {
                    std::string ip = current_viewer.driver->backend_config.value("send_ip", "127.0.0.1");
                    int port = current_viewer.driver->backend_config.value("send_port", 8080);
                    ImGui::SameLine();
                    ImGui::TextDisabled("(%s:%d)", ip.c_str(), port);
                }

                bool is_listening = listener && listener->keep_listening.load();
                if (is_listening) {
                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
                    ImGui::Text("%s Receiving", ui::icon::circle);
                    ImGui::PopStyleColor();
                } else {
                    ImGui::SameLine();
                    ImGui::TextDisabled("%s Paused", ui::icon::circle);
                }
            } else {
                ImGui::TextDisabled("Not connected to any driver");
            }

            ImGui::Separator();

            // Signal count
            ImGui::Text("Signals: %zu", current_viewer.signals.size());
            if (!signal_data.empty()) {
                ImGui::SameLine();
                ImGui::TextDisabled("(%zu receiving data)", signal_data.size());
            }

            ImGui::Separator();

            // Scrollable signal list with charts, grouped by port
            float chart_height = 120.0f;

            ImGui::BeginChild("SignalCharts", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            if (current_viewer.signals.empty()) {
                ImGui::TextDisabled("No outgoing signals defined in driver");
            } else {
                // Group signals by port
                uint64_t current_port_id = 0;
                std::string current_port_name;
                std::string current_port_type;

                for (const auto& info : current_viewer.signals) {
                    // New port group - show separator with port name
                    if (info.port_id != current_port_id) {
                        current_port_id = info.port_id;
                        current_port_name = info.port_name;
                        current_port_type = info.port_type;

                        if (current_port_id != current_viewer.signals.front().port_id) {
                            ImGui::Spacing();
                        }

                        // Port header with icon
                        std::string port_icon = (info.port_type == "plug") ? ui::icon::plug : ui::icon::socket;
                        std::string port_header = port_icon + " " + info.port_name;
                        ui::separator(port_header);
                        ImGui::Spacing();
                    }

                    ImGui::PushID(static_cast<int>(info.signal_id));
                    render_signal_chart(info, signal_data, chart_height);
                    ImGui::Spacing();
                    ImGui::PopID();
                }
            }

            ImGui::EndChild();
        }
        ImGui::End();

        if (!current_viewer.open) {
            close_viewer();
        }
    }

} // namespace dashboard
