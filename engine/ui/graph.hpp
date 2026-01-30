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
#include "ui/connector.hpp"
#include "ui/instance.hpp"
#include "ui/focus.hpp"

namespace ui {
	namespace graph {
        namespace ed = ax::NodeEditor;

        // Find an item in a collection by its node ID
        template<typename T>
        T* find_selected_in(std::list<T>& collection, uint64_t node_id) {
            for (auto& item : collection) {
                if (item.id() == node_id) return &item;
            }
            return nullptr;
        }

        // Handle drill-down on double-click for a collection
        template<typename T, typename Callback, typename UuidGetter>
        bool try_drilldown(std::list<T>& collection, uint64_t node_id,
                          focus::level level, Callback&& callback, UuidGetter&& get_uuid) {
            for (auto& item : collection) {
                if (item.id() == node_id) {
                    callback({level, get_uuid(item), item.instance_name()});
                    return true;
                }
            }
            return false;
        }

        // Convenience overload for items with prototype.uuid
        template<typename T, typename Callback>
        bool try_drilldown_prototype(std::list<T>& collection, uint64_t node_id,
                                     focus::level level, Callback&& callback) {
            return try_drilldown(collection, node_id, level, callback,
                [](const T& item) { return item.prototype.uuid; });
        }

        // Convenience overload for items with get_prototype_uuid()
        template<typename T, typename Callback>
        bool try_drilldown_port(std::list<T>& collection, uint64_t node_id,
                               focus::level level, Callback&& callback) {
            return try_drilldown(collection, node_id, level, callback,
                [](const T& item) { return item.get_prototype_uuid(); });
        }

        // Delete selected nodes from multiple collections
        template<typename... Collections>
        void delete_nodes_by_id(uint64_t node_id, Collections&... collections) {
            (collections.remove_if([node_id](const auto& item) {
                return item.id() == node_id;
            }), ...);
        }

        void render_list(const char* icon, const std::string& label, auto& instances) {
            ui::separator(label);
            for (auto& instance : instances) {
                ui::instance::title(instance);
            }
        }


        void render_list(const char* icon, const std::string& label, auto& instances, auto*& selected_instance) {
            ui::separator(label);
            for (auto& instance : instances) {
                if (ui::selectable("##" + instance.uuid)) {
                    ed::SelectNode(instance.id());
                }
                ui::same_line();
                ui::instance::title(instance);
            }
        }

        void handle_link_creation(controls::graph& graph) {
            if (ed::BeginCreate()) {
                ed::PinId start_pin, end_pin;
                if (ed::QueryNewLink(&start_pin, &end_pin) && start_pin && end_pin && start_pin != end_pin) {
                    uint64_t start_id = start_pin.Get();
                    uint64_t end_id = end_pin.Get();

                    if (graph.is_connectable(start_id, end_id)) {
                        if (ed::AcceptNewItem()) {
                            graph.add_link(start_id, end_id);
                            ui::cout << "Created link between instances " << start_id << " and " << end_id << ui::endl;
                        }
                    }
                    else {
                        ui::cout << "Instances are not connectable due to prototype mismatch" << ui::endl;
                    }
                }
            }
            ed::EndCreate();
        }

        void render_existing_links(const controls::graph& graph) {
            for (const auto& link : graph.get_links()) {
                ed::Link(ed::LinkId(link.id), ed::PinId(link.start), ed::PinId(link.end));
            }
        }

        template <typename T, typename LG, typename RG>
        void render_node(ui::icon::type icon, T& instance, LG& lg, RG& rg, const controls::graph& graph, ui::connector::shape shape) {
            ui::node::begin_node(instance.id());
            ui::id::push(instance.id());
            ui::text(icon);

            const auto& left_group = lg;
            const auto& right_group = rg;

            float title_width = ui::text_size(instance.prototype_typename()).x;
            float subtitle_width = ui::text_size(instance.instance_name()).x;
            float max_left_width = 0.0f;
            float max_right_width = 0.0f;

            for (const auto& signal : left_group) {
                float signal_width = ui::text_size(signal.name).x;
                max_left_width = std::max(signal_width, max_left_width);
            }

            for (const auto& signal : right_group) {
                float signal_width = ui::text_size(signal.name).x;
                max_right_width = std::max(signal_width, max_right_width);
            }

            float connector_size = 15.0f;
            float spacing = 20.0f;
            float padding = 40.0f;
            float min_total_width = std::max(title_width, subtitle_width) + 2 * padding;
            float total_width = std::max(min_total_width, max_left_width + max_right_width + 2 * connector_size + spacing);
            float title_padding = (total_width - title_width) * 0.5f;
            float subtitle_padding = (total_width - subtitle_width) * 0.5f;
            title_padding = std::max(title_padding, padding);
            subtitle_padding = std::max(subtitle_padding, padding);

            ui::dummy(title_padding, 0.0f);
            ui::same_line();
            ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_typename);
            ui::text(instance.prototype_typename());
            ui::style::color::pop();

            ui::dummy(subtitle_padding, 0.0f);
            ui::same_line();
            ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_instance);
            ui::text(instance.instance_name());
            ui::style::color::pop();

            ui::dummy(0.0f, 10.0f);

            ui::group::begin();
            for (const auto& signal : left_group) {
                uint64_t pin_id = instance.id() | signal.id();
                ui::pin::begin(pin_id, ui::pin::input);
                switch (shape) {
                case ui::connector::shape::triangle:
                    ui::connector::triangle(instance.direction, graph.is_connected(pin_id));
                    break;
                case ui::connector::shape::square:
                    ui::connector::square(graph.is_connected(pin_id));
                    break;
                case ui::connector::shape::circle:
                    ui::connector::circle(graph.is_connected(pin_id));
                    break;
                }
                ui::pin::end();
                ui::same_line();
                ui::text(signal.name);
                if (ImGui::IsItemHovered()) {
                    ImVec2 offset(30.0f, -30.0f);
                    ImVec2 mouse_pos = ImGui::GetMousePos();
                    ImVec2 canvas_pos = ax::NodeEditor::CanvasToScreen(ImVec2(mouse_pos.x + offset.x, mouse_pos.y + offset.y));

                    ImGui::SetNextWindowPos(canvas_pos);
                    ImGui::BeginTooltip();
                    ImGui::TextUnformatted(signal.prototype_typename().c_str());
                    ImGui::EndTooltip();
                }
            }
            ui::group::end();

            float available_space = total_width - (max_left_width + max_right_width + 2 * connector_size);
            float spacing_between_io = available_space > 0.0f ? available_space : spacing;

            ui::same_line();
            ui::dummy(spacing_between_io, 0.0f);
            ui::same_line();

            ui::group::begin();
            for (const auto& signal : right_group) {
                uint64_t pin_id = instance.id() | signal.id();
                ui::dummy(max_right_width - ImGui::CalcTextSize(signal.name.c_str()).x, 0.0f);
                ui::same_line();
                ui::text(signal.name);
                if (ImGui::IsItemHovered()) {
                    ImVec2 offset(30.0f, -30.0f);
                    ImVec2 mouse_pos = ImGui::GetMousePos();
                    ImVec2 canvas_pos = ax::NodeEditor::CanvasToScreen(ImVec2(mouse_pos.x + offset.x, mouse_pos.y + offset.y));

                    ImGui::SetNextWindowPos(canvas_pos);
                    ImGui::BeginTooltip();
                    ImGui::TextUnformatted(signal.prototype_typename().c_str());
                    ImGui::EndTooltip();
                }
                ui::same_line();
                ui::pin::begin(pin_id, ui::pin::output);
                switch (shape) {
                case ui::connector::shape::triangle:
                    ui::connector::triangle(instance.direction, graph.is_connected(pin_id));
                    break;
                case ui::connector::shape::square:
                    ui::connector::square(graph.is_connected(pin_id));
                    break;
                case ui::connector::shape::circle:
                    ui::connector::circle(graph.is_connected(pin_id));
                    break;
                }

                ui::pin::end();
            }
            ui::group::end();


            ui::dummy(0.0f, 10.0f);
            ui::group::begin();
            if (ui::button(ICON_FA_ARROW_RIGHT_ARROW_LEFT)) {
                instance.direction = (instance.direction == ui::direction::ltr) ? ui::direction::rtl : ui::direction::ltr;
            }

            if (ImGui::IsItemHovered()) {
                ImVec2 offset(30.0f, -30.0f);
                ImVec2 mouse_pos = ImGui::GetMousePos();
                ImVec2 canvas_pos = ax::NodeEditor::CanvasToScreen(ImVec2(mouse_pos.x + offset.x, mouse_pos.y + offset.y));

                ImGui::SetNextWindowPos(canvas_pos);
                ImGui::BeginTooltip();
                ImGui::TextUnformatted("Swap Direction");
                ImGui::EndTooltip();
            }
            ui::group::end();

            ui::id::pop();
            ui::node::end_node();
        }
	}
}