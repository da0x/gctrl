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
#include "ui/focus.hpp"
#include "ui/selection.hpp"
#include "ui/theme.hpp"
#include "controls/machine/object.hpp"
#include "controls/machine/instance.hpp"
#include "controls/controller/object.hpp"
#include "controls/driver/instance.hpp"
#include "controls/driver/type.hpp"
#include "controls/element/object.hpp"
#include "controls/port/object.hpp"
#include "controls/runtime/state.hpp"

namespace ui {
namespace hierarchy {

    struct callbacks {
        std::function<void(const focus::focus_entry&)> on_select;
        std::function<void(const focus::focus_entry&)> on_drill_down;
    };

    inline callbacks current_callbacks;
    inline selection* current_selection = nullptr;
    inline runtime::tracker* current_runtime_tracker = nullptr;
    inline machine::instance::list* current_machines = nullptr;

    // Push prominent selection/hover colors for tree items
    inline void push_tree_colors() {
        // Selection color - prominent blue highlight
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.29f, 0.59f, 0.82f, 0.80f));
        // Hover color - lighter blue
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.29f, 0.59f, 0.82f, 0.50f));
        // Active (clicked) color
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.29f, 0.59f, 0.82f, 1.00f));
    }

    inline void pop_tree_colors() {
        ImGui::PopStyleColor(3);
    }

    // Find parent controller of an element/plug/socket by searching the tree
    inline const controller::instance* find_parent_controller(uint64_t item_id) {
        if (!current_machines) return nullptr;
        for (const auto& mach : *current_machines) {
            for (const auto& ctrl : mach.controllers()) {
                const auto& proto = dynamic_cast<const controller::object&>(ctrl.prototype);
                for (const auto& elem : proto.elements) {
                    if (elem.id() == item_id) return &ctrl;
                }
                for (const auto& plug : proto.plugs) {
                    if (plug.id() == item_id) return &ctrl;
                }
                for (const auto& socket : proto.sockets) {
                    if (socket.id() == item_id) return &ctrl;
                }
            }
        }
        return nullptr;
    }

    // Template for leaf nodes (elements, plugs, sockets)
    // Single click selects and navigates to parent controller
    // Double click drills down to the prototype editor (same as canvas double-click)
    template<typename Instance, typename UuidGetter>
    void render_leaf(const Instance& item, const char* icon, focus::level level, UuidGetter&& get_uuid) {
        bool is_selected = current_selection && current_selection->is_selected(item.id());

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (is_selected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        ImGui::TreeNodeEx(reinterpret_cast<void*>(item.id()), flags, "%s %s", icon, item.instance_name().c_str());

        // Double-click drills down to prototype editor (same as canvas double-click)
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            if (current_selection) {
                current_selection->clear();
            }
            if (current_callbacks.on_drill_down) {
                current_callbacks.on_drill_down({level, get_uuid(item), item.instance_name()});
            }
        }
        // Single click selects and navigates to parent controller
        else if (ImGui::IsItemClicked()) {
            if (current_selection) {
                current_selection->select(item.id());
            }
            // Navigate to parent controller first (for cross-container selection)
            if (current_callbacks.on_drill_down) {
                const controller::instance* parent = find_parent_controller(item.id());
                if (parent) {
                    current_callbacks.on_drill_down({focus::level::controller, parent->prototype.uuid, parent->instance_name()});
                }
            }
            if (current_callbacks.on_select) {
                current_callbacks.on_select({level, get_uuid(item), item.instance_name()});
            }
        }
    }

    // Template for parent nodes (has children and drill-down)
    // Single click drills down to show the editor for this item
    template<typename Instance, typename UuidGetter, typename ChildRenderer>
    void render_parent(const Instance& item, const char* icon, focus::level level,
                       UuidGetter&& get_uuid, ChildRenderer&& render_children) {
        bool is_selected = current_selection && current_selection->is_selected(item.id());

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (is_selected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(item.id()), flags, "%s %s", icon, item.instance_name().c_str());

        // Single click drills down to this item's editor
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            // Clear selection when drilling down to a container (equivalent to clicking canvas)
            if (current_selection) {
                current_selection->clear();
            }
            if (current_callbacks.on_drill_down) {
                current_callbacks.on_drill_down({level, get_uuid(item), item.instance_name()});
            }
        }

        if (open) {
            render_children();
            ImGui::TreePop();
        }
    }

    inline void render_port_instance(const port::plug::instance& plug) {
        render_leaf(plug, ui::icon::plug, focus::level::plug,
            [](const auto& p) { return p.get_prototype_uuid(); });
    }

    inline void render_socket_instance(const port::socket::instance& socket) {
        render_leaf(socket, ui::icon::socket, focus::level::socket,
            [](const auto& s) { return s.get_prototype_uuid(); });
    }

    inline void render_element_instance(const element::instance& elem) {
        render_leaf(elem, ui::icon::element, focus::level::element,
            [](const auto& e) { return e.prototype.uuid; });
    }

    inline void render_controller_instance(const controller::instance& ctrl) {
        render_parent(ctrl, ui::icon::controller, focus::level::controller,
            [](const auto& c) { return c.prototype.uuid; },
            [&]() {
                const auto& proto = dynamic_cast<const controller::object&>(ctrl.prototype);
                for (const auto& elem : proto.elements) render_element_instance(elem);
                for (const auto& plug : proto.plugs) render_port_instance(plug);
                for (const auto& socket : proto.sockets) render_socket_instance(socket);
            });
    }

    inline void render_driver_object(const driver::instance& drv) {
        bool is_selected = current_selection && current_selection->is_selected(drv.id());

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (is_selected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        // Use backend-specific icon
        const char* icon = driver::type_to_icon(drv.backend_type);
        bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(drv.id()), flags, "%s %s", icon, drv.display_name().c_str());

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            // Update selection
            if (current_selection) {
                current_selection->select(drv.id());
            }
            if (current_callbacks.on_select) {
                current_callbacks.on_select({focus::level::driver, drv.uuid, drv.display_name()});
            }
        }

        if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0) && current_callbacks.on_drill_down) {
            current_callbacks.on_drill_down({focus::level::driver, drv.uuid, drv.display_name()});
        }

        if (open) {
            for (const auto& plug : drv.plugs) render_port_instance(plug);
            for (const auto& socket : drv.sockets) render_socket_instance(socket);
            ImGui::TreePop();
        }
    }

    inline void render_machine_instance(machine::instance& mach) {
        bool is_selected = current_selection && current_selection->is_selected(mach.id());

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (is_selected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        // Get runtime status for visual indicator
        const char* status_indicator = "";
        ImVec4 status_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

        if (current_runtime_tracker) {
            auto status = current_runtime_tracker->get_status(mach.id());
            switch (status) {
                case runtime::status::running:
                    status_indicator = ICON_FA_PLAY " ";
                    status_color = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);
                    break;
                case runtime::status::building:
                    status_indicator = ICON_FA_HAMMER " ";
                    status_color = ImVec4(1.0f, 0.7f, 0.0f, 1.0f);
                    break;
                case runtime::status::error:
                    status_indicator = ICON_FA_TRIANGLE_EXCLAMATION " ";
                    status_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                    break;
                default:
                    break;
            }
        }

        // Build display string: icon + instance name
        std::string display_text = std::string(status_indicator) + ui::icon::machine + " " + mach.instance_name();

        ImGui::PushStyleColor(ImGuiCol_Text, status_color);
        bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(mach.id()), flags, "%s", display_text.c_str());
        ImGui::PopStyleColor();

        // Single click on machine drills down to show machine view
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            // Clear selection when drilling down to a container (equivalent to clicking canvas)
            if (current_selection) {
                current_selection->clear();
            }
            if (current_callbacks.on_drill_down) {
                current_callbacks.on_drill_down({focus::level::machine, mach.uuid, mach.instance_name()});
            }
        }

        if (open) {
            for (const auto& ctrl : mach.controllers()) render_controller_instance(ctrl);
            for (const auto& drv : mach.drivers()) render_driver_object(drv);
            ImGui::TreePop();
        }
    }

    inline void render(
        machine::instance::list& machines,
        runtime::tracker& runtime_tracker,
        selection& sel,
        const callbacks& cbs = {}
    ) {
        current_callbacks = cbs;
        current_runtime_tracker = &runtime_tracker;
        current_machines = &machines;
        current_selection = &sel;

        ui::begin("Hierarchy");

        // Push prominent selection/hover colors
        push_tree_colors();

        ui::tree::context::begin("hierarchy_panel");

        // Network root node
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        bool open = ImGui::TreeNodeEx("network_root", flags, "%s Network", ui::icon::network);

        // Single click on Network navigates to network view
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            // Clear selection when clicking network
            if (current_selection) {
                current_selection->clear();
            }
            if (current_callbacks.on_drill_down) {
                current_callbacks.on_drill_down({focus::level::network, "", "Network"});
            }
        }

        if (open) {
            for (auto& mach : machines) {
                render_machine_instance(mach);
            }
            ImGui::TreePop();
        }

        ui::tree::context::end();

        pop_tree_colors();

        ui::end();

        current_runtime_tracker = nullptr;
        current_machines = nullptr;
        current_selection = nullptr;
    }

} // namespace hierarchy
} // namespace ui
