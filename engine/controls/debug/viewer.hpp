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

#include <optional>
#include <map>
#include <vector>
#include <string>
#include "ui/ui.hpp"
#include "controls/machine/instance.hpp"

namespace controls {
    namespace debug {
        namespace viewer {

            std::optional<uint64_t> selected_driver_id;

            void render_signal_data(uint64_t id, const controls::instance& instance, const std::map<uint64_t, std::vector<float>>& signal_data) {
                ui::instance::title(instance);
                ui::tooltip("ID = " + std::to_string(id));

                static std::map<uint64_t, float> y_min_map, y_max_map, y_center_map;
                static std::map<uint64_t, bool> auto_scale_map;

                if (auto_scale_map.find(id) == auto_scale_map.end()) {
                    auto_scale_map[id] = true;
                    y_min_map[id] = -1.0f;
                    y_max_map[id] = 1.0f;
                    y_center_map[id] = 0.0f;
                }

                auto it = signal_data.find(id);
                if (it != signal_data.end()) {
                    const auto& values = it->second;
                    std::vector<float> x_values(values.size());
                    for (size_t i = 0; i < values.size(); ++i) {
                        x_values[i] = static_cast<float>(i);
                    }

                    auto [min_value, max_value] = std::minmax_element(values.begin(), values.end());
                    float padding = (*max_value - *min_value + 0.01) * 0.2f;
                    if (auto_scale_map[id]) {
                        y_min_map[id] = *min_value - padding;
                        y_max_map[id] = *max_value + padding;
                    }

                    float total_width = ImGui::GetContentRegionAvail().x;
                    float plot_width = total_width * 0.85f;
                    float form_width = total_width * 0.15f;

                    ImGui::BeginChild(("PlotArea##" + std::to_string(id)).c_str(), ImVec2(plot_width, 200), true);
                    if (ImPlot::BeginPlot(instance.name.c_str(), ImVec2(-1, -1))) {
                        ImPlot::SetupAxesLimits(
                            x_values.front(),
                            x_values.back(),
                            y_min_map[id],
                            y_max_map[id],
                            ImPlotCond_Always);

                        ImPlot::PlotLine("Value", x_values.data(), values.data(), static_cast<int>(values.size()));
                        ImPlot::EndPlot();
                    }
                    ImGui::EndChild();

                    ImGui::SameLine();

                    ImGui::BeginChild(("FormArea##" + std::to_string(id)).c_str(), ImVec2(form_width, 200), true);
                    ImGui::Text("Settings");

                    ImGui::Checkbox(("Auto-Scale##" + std::to_string(id)).c_str(), &auto_scale_map[id]);

                    ImGui::InputFloat(("Y-Max##" + std::to_string(id)).c_str(), &y_max_map[id]);
                    ImGui::InputFloat(("Y-Center##" + std::to_string(id)).c_str(), &y_center_map[id]);
                    ImGui::InputFloat(("Y-Min##" + std::to_string(id)).c_str(), &y_min_map[id]);

                    ImGui::EndChild();
                }
                else {
                    static std::map<uint64_t, float> values;
                    ImGui::InputFloat((instance.name + "##" + std::to_string(id)).c_str(), &values[id]);
                    if (ImGui::Button(("Send##" + std::to_string(id)).c_str())) {
                        ::debug::send(id, values[id]);
                    }
                }
            }

            void render_signal_data_drag_drop(const controls::instance& instance, const std::map<uint64_t, std::vector<float>>& signal_data) {
                return;
                static std::map<uint64_t, std::vector<float>> active_signals;
                static std::map<uint64_t, ImVec4> signal_colors;

                ImGui::Begin("Available Signals");

                for (const auto& [id, values] : signal_data) {
                    if (ImGui::Selectable(("Signal " + std::to_string(id)).c_str())) {
                        if (signal_colors.find(id) == signal_colors.end()) {
                            signal_colors[id] = ImVec4(static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX,
                                static_cast<float>(rand()) / RAND_MAX, 1.0f);
                        }
                    }
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        ImGui::SetDragDropPayload("SIGNAL_ID", &id, sizeof(uint64_t));
                        ImGui::Text("Dragging Signal %llu", id);
                        ImGui::EndDragDropSource();
                    }
                }
                ImGui::End();

                ImGui::Begin("Chart Area");

                // Drop target for signals
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SIGNAL_ID")) {
                        uint64_t dropped_id = *(uint64_t*)payload->Data;
                        active_signals[dropped_id] = signal_data.at(dropped_id);
                    }
                    ImGui::EndDragDropTarget();
                }
                if (ImPlot::BeginPlot("Signal Chart", ImVec2(-1, 300))) {
                    for (const auto& [id, values] : active_signals) {
                        std::vector<float> x_values(values.size());
                        for (size_t i = 0; i < values.size(); ++i) {
                            x_values[i] = static_cast<float>(i);
                        }

                        // Ensure correct function signature
                        ImPlot::PlotLine(
                            ("Signal " + std::to_string(id)).c_str(),
                            x_values.data(),                    // Explicit X values
                            values.data(),                      // Corresponding Y values
                            static_cast<int>(values.size()),    // Number of points
                            ImPlotLineFlags_None,               // Line flags
                            0,                                  // Offset (default)
                            sizeof(float));                     // Stride
                    }
                    ImPlot::EndPlot();
                }

                ImGui::End();

                // Settings panel for customization
                ImGui::Begin("Settings");
                for (const auto& [id, color] : signal_colors) {
                    ImGui::Text("Signal %llu", id);
                    ImGui::ColorEdit4(("Color##" + std::to_string(id)).c_str(), (float*)&signal_colors[id]);
                }
                ImGui::End();
            }




            std::vector<std::pair<uint64_t, const controls::instance&>> collect_signals(const driver::instance& driver) {
                std::vector<std::pair<uint64_t, const controls::instance&>> signal_list;
                for (const auto& element : driver.inputs()) {
                    for (const auto& signal : element.inputs()) {
                        signal_list.emplace_back(element.id() | signal.id(), signal);
                    }
                    for (const auto& signal : element.outputs()) {
                        signal_list.emplace_back(element.id() | signal.id(), signal);
                    }
                }
                for (const auto& element : driver.outputs()) {
                    for (const auto& signal : element.inputs()) {
                        signal_list.emplace_back(element.id() | signal.id(), signal);
                    }
                    for (const auto& signal : element.outputs()) {
                        signal_list.emplace_back(element.id() | signal.id(), signal);
                    }
                }
                return signal_list;
            }

            void render_signals(const std::vector<std::pair<uint64_t, const controls::instance&>>& signal_list, const std::map<uint64_t, std::vector<float>>& signal_data) {
                for (const auto& [id, instance] : signal_list) {
                    render_signal_data(id, instance, signal_data);
                    render_signal_data_drag_drop(instance, signal_data);
                }
            }

            void render_ui(const std::vector<std::pair<uint64_t, const controls::instance&>>& signal_list, const std::map<uint64_t, std::vector<float>>& signal_data) {
                if (ImGui::Begin("Signal Viewer")) {
                    if (!selected_driver_id.has_value()) {
                        ImGui::Text("No driver selected.");
                    }
                    else {
                        render_signals(signal_list, signal_data);
                    }
                }
                ImGui::End();
            }

            void render_driver_selection(const machine::object::list& machines) {
                for (auto& machine : machines) {
                    auto mi = machine::instance(machine);
                    if (ui::tree::child(machine.id(), ui::instance::title_functor(mi))) {
                        for (auto& driver : machine.drivers) {
                            if (ui::tree::child(driver.id(), ui::instance::title_functor(driver))) {
                                if (ui::tree::is_selected(driver.id())) {
                                    selected_driver_id = driver.id();
                                }
                                ui::tree::child_end();
                            }
                        }

                        for (auto& controller : machine.controllers) {
                            if (ui::tree::child(controller.id(), ui::instance::title_functor(controller))) {
                                if (ui::tree::is_selected(controller.id())) {
                                    selected_driver_id = controller.id();
                                }

                                ui::tree::child_end();
                            }
                        }
                        ui::tree::child_end();
                    }
                }
            }

            void render(const machine::object::list& machines, const std::map<uint64_t, std::vector<float>>& signal_data) {
                ui::begin("Tree View");
                ui::tree::context::begin("machine_tree");
                render_driver_selection(machines);
                ui::tree::context::end();
                ui::end();

                ui::begin("Detail View");
                if (selected_driver_id.has_value()) {
                    auto driver_it = std::find_if(machines.begin(), machines.end(), [](const machine::object& m) {
                        return std::any_of(m.drivers.begin(), m.drivers.end(), [](const driver::instance& d) {
                            return d.id() == *selected_driver_id;
                            });
                        });
                    if (driver_it != machines.end()) {
                        const auto& driver = *std::find_if(driver_it->drivers.begin(), driver_it->drivers.end(),
                            [](const driver::instance& d) { return d.id() == *selected_driver_id; });
                        auto signal_list = collect_signals(driver);
                        render_ui(signal_list, signal_data);
                    }
                }
                ui::end();
            }
        }
    }
} // namespace controls



/*

                        for (auto& controller : machine.controllers) {
                            if (ui::tree::child(controller.id(), ui::instance::title_functor(controller))) {

                                const auto& object = *dynamic_cast<const controller::object*>(&controller.prototype);
                                for (auto& element : object.elements) {
                                    if (ui::tree::child(element.id(), ui::instance::title_functor(element))) {
                                        for (auto& signal : element.inputs()) {
                                            ui::tree::leaf(signal.id(), ui::instance::title_functor(signal));
                                        }
                                        for (auto& signal : element.outputs()) {
                                            ui::tree::leaf(signal.id(), ui::instance::title_functor(signal));
                                        }
                                        ui::tree::child_end();
                                    }
                                }

                                for (auto& element : object.plugs) {
                                    if (ui::tree::child(element.id(), ui::instance::title_functor(element))) {
                                        for (auto& signal : element.inputs()) {
                                            ui::tree::leaf(signal.id(), ui::instance::title_functor(signal));
                                        }
                                        for (auto& signal : element.outputs()) {
                                            ui::tree::leaf(signal.id(), ui::instance::title_functor(signal));
                                        }
                                        ui::tree::child_end();
                                    }
                                }

                                for (auto& element : object.sockets) {
                                    if (ui::tree::child(element.id(), ui::instance::title_functor(element))) {
                                        for (auto& signal : element.inputs()) {
                                            ui::tree::leaf(signal.id(), ui::instance::title_functor(signal));
                                        }
                                        for (auto& signal : element.outputs()) {
                                            ui::tree::leaf(signal.id(), ui::instance::title_functor(signal));
                                        }
                                        ui::tree::child_end();
                                    }
                                }
                                ui::tree::child_end();
                            }
                        }

                        */