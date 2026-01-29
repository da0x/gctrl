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

    inline void render_port_instance(const port::plug::instance& plug) {
        ui::tree::leaf(plug.id(), [&]() {
            ui::text(std::string(ui::icon::plug) + " " + plug.instance_name());
        });
        if (ui::tree::is_selected(plug.id()) && current_callbacks.on_select) {
            current_callbacks.on_select({focus::level::plug, plug.get_prototype_uuid(), plug.instance_name()});
        }
    }

    inline void render_socket_instance(const port::socket::instance& socket) {
        ui::tree::leaf(socket.id(), [&]() {
            ui::text(std::string(ui::icon::socket) + " " + socket.instance_name());
        });
        if (ui::tree::is_selected(socket.id()) && current_callbacks.on_select) {
            current_callbacks.on_select({focus::level::socket, socket.get_prototype_uuid(), socket.instance_name()});
        }
    }

    inline void render_element_instance(const element::instance& elem) {
        ui::tree::leaf(elem.id(), [&]() {
            ui::text(std::string(ui::icon::element) + " " + elem.instance_name());
        });
        if (ui::tree::is_selected(elem.id()) && current_callbacks.on_select) {
            current_callbacks.on_select({focus::level::element, elem.prototype.uuid, elem.instance_name()});
        }
    }

    inline void render_controller_instance(const controller::instance& ctrl) {
        bool open = ui::tree::child(ctrl.id(), [&]() {
            ui::text(std::string(ui::icon::controller) + " " + ctrl.instance_name());
        });

        if (ui::tree::is_selected(ctrl.id())) {
            if (current_callbacks.on_select) {
                current_callbacks.on_select({focus::level::controller, ctrl.prototype.uuid, ctrl.instance_name()});
            }
            if (ImGui::IsMouseDoubleClicked(0) && current_callbacks.on_drill_down) {
                current_callbacks.on_drill_down({focus::level::controller, ctrl.prototype.uuid, ctrl.instance_name()});
            }
        }

        if (open) {
            const auto& proto = dynamic_cast<const controller::object&>(ctrl.prototype);
            for (const auto& elem : proto.elements) {
                render_element_instance(elem);
            }
            for (const auto& plug : proto.plugs) {
                render_port_instance(plug);
            }
            for (const auto& socket : proto.sockets) {
                render_socket_instance(socket);
            }
            ui::tree::child_end();
        }
    }

    inline void render_driver_instance(const driver::instance& drv) {
        bool open = ui::tree::child(drv.id(), [&]() {
            ui::text(std::string(ui::icon::driver) + " " + drv.instance_name());
        });

        if (ui::tree::is_selected(drv.id())) {
            if (current_callbacks.on_select) {
                current_callbacks.on_select({focus::level::driver, drv.prototype.uuid, drv.instance_name()});
            }
            if (ImGui::IsMouseDoubleClicked(0) && current_callbacks.on_drill_down) {
                current_callbacks.on_drill_down({focus::level::driver, drv.prototype.uuid, drv.instance_name()});
            }
        }

        if (open) {
            const auto& proto = dynamic_cast<const driver::object&>(drv.prototype);
            for (const auto& plug : proto.plugs) {
                render_port_instance(plug);
            }
            for (const auto& socket : proto.sockets) {
                render_socket_instance(socket);
            }
            ui::tree::child_end();
        }
    }

    inline void render_machine(machine::object& mach) {
        bool open = ui::tree::child(mach.id(), [&]() {
            ui::text(std::string(ui::icon::machine) + " " + mach.display_name());
        });

        if (ui::tree::is_selected(mach.id())) {
            if (current_callbacks.on_select) {
                current_callbacks.on_select({focus::level::machine, mach.uuid, mach.display_name()});
            }
            if (ImGui::IsMouseDoubleClicked(0) && current_callbacks.on_drill_down) {
                current_callbacks.on_drill_down({focus::level::machine, mach.uuid, mach.display_name()});
            }
        }

        if (open) {
            for (const auto& ctrl : mach.controllers) {
                render_controller_instance(ctrl);
            }
            for (const auto& drv : mach.drivers) {
                render_driver_instance(drv);
            }
            ui::tree::child_end();
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
