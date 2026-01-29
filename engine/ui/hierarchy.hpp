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
#include "controls/machine/object.hpp"
#include "controls/controller/object.hpp"
#include "controls/driver/object.hpp"
#include "controls/element/object.hpp"
#include "controls/port/object.hpp"

namespace ui {
namespace hierarchy {

    struct callbacks {
        std::function<void(const focus::focus_entry&)> on_select;
        std::function<void(const focus::focus_entry&)> on_drill_down;
    };

    inline callbacks current_callbacks;

    // Pending node selection - set by hierarchy, consumed by node editor
    inline uint64_t pending_select_node_id = 0;
    // Currently selected node in canvas - set by node editor, used by hierarchy for highlighting
    inline uint64_t canvas_selected_node_id = 0;

    inline void request_select_node(uint64_t id) {
        pending_select_node_id = id;
    }

    inline uint64_t consume_pending_selection() {
        uint64_t id = pending_select_node_id;
        pending_select_node_id = 0;
        return id;
    }

    inline void set_canvas_selection(uint64_t id) {
        canvas_selected_node_id = id;
    }

    inline uint64_t get_canvas_selection() {
        return canvas_selected_node_id;
    }

    // Template for leaf nodes (no children, no drill-down)
    template<typename Instance, typename UuidGetter>
    void render_leaf(const Instance& item, const char* icon, focus::level level, UuidGetter&& get_uuid) {
        bool is_canvas_selected = (canvas_selected_node_id == item.id());

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (is_canvas_selected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        ImGui::TreeNodeEx(reinterpret_cast<void*>(item.id()), flags, "%s %s", icon, item.instance_name().c_str());

        if (ImGui::IsItemClicked()) {
            request_select_node(item.id());
            if (current_callbacks.on_select) {
                current_callbacks.on_select({level, get_uuid(item), item.instance_name()});
            }
        }
    }

    // Template for parent nodes (has children and drill-down)
    template<typename Instance, typename UuidGetter, typename ChildRenderer>
    void render_parent(const Instance& item, const char* icon, focus::level level,
                       UuidGetter&& get_uuid, ChildRenderer&& render_children) {
        bool is_canvas_selected = (canvas_selected_node_id == item.id());

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
        if (is_canvas_selected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(item.id()), flags, "%s %s", icon, item.instance_name().c_str());

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            request_select_node(item.id());
            if (current_callbacks.on_select) {
                current_callbacks.on_select({level, get_uuid(item), item.instance_name()});
            }
        }

        if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0) && current_callbacks.on_drill_down) {
            current_callbacks.on_drill_down({level, get_uuid(item), item.instance_name()});
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

    inline void render_driver_instance(const driver::instance& drv) {
        render_parent(drv, ui::icon::driver, focus::level::driver,
            [](const auto& d) { return d.prototype.uuid; },
            [&]() {
                const auto& proto = dynamic_cast<const driver::object&>(drv.prototype);
                for (const auto& plug : proto.plugs) render_port_instance(plug);
                for (const auto& socket : proto.sockets) render_socket_instance(socket);
            });
    }

    inline void render_machine(machine::object& mach) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

        bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(mach.id()), flags, "%s %s", ui::icon::machine, mach.display_name().c_str());

        // Single click on machine drills down to show machine view
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            if (current_callbacks.on_drill_down) {
                current_callbacks.on_drill_down({focus::level::machine, mach.uuid, mach.display_name()});
            }
        }

        if (open) {
            for (const auto& ctrl : mach.controllers) render_controller_instance(ctrl);
            for (const auto& drv : mach.drivers) render_driver_instance(drv);
            ImGui::TreePop();
        }
    }

    inline void render(
        machine::object::list& machines,
        const callbacks& cbs = {}
    ) {
        current_callbacks = cbs;

        ui::begin("Hierarchy");
        ui::tree::context::begin("hierarchy_panel");

        for (auto& mach : machines) {
            render_machine(mach);
        }

        ui::tree::context::end();
        ui::end();
    }

} // namespace hierarchy
} // namespace ui
