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
#include <algorithm>
#include <list>
#include <string>
#include <cstring>
#include "controls/code/generator.hpp"
#include "controls/driver/instance.hpp"
#include "ui/code.hpp"
#include <da0x/uuid.hpp>
#include "imgui.h"

namespace driver {

    template <typename T>
    void render_ports(ui::icon::type icon, T& ports) {
        if (ImGui::BeginTable("PortsTable", 4, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("##Icon", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Port Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("##Actions", ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();

            for (auto it = ports.begin(); it != ports.end();) {
                auto& port_instance = *it;
                ImGui::TableNextRow();
                ui::id::push(static_cast<int>(port_instance.id()));

                ImGui::TableSetColumnIndex(0);
                ui::text(icon);

                ImGui::TableSetColumnIndex(1);
                ui::input::text("##Name", port_instance.name);

                ImGui::TableSetColumnIndex(2);
                ui::text(port_instance.prototype_typename());

                ImGui::TableSetColumnIndex(3);
                bool delete_clicked = ui::button(ui::icon::trash);
                if (delete_clicked) {
                    it = ports.erase(it);
                }
                else {
                    ++it;
                }
                ui::id::pop();
            }

            ImGui::EndTable();
        }
    }

    void render_udp_config(driver::instance& drv) {
        ui::separator("UDP Configuration");

        // Get current values from backend_config
        std::string listen_ip = drv.backend_config.value("listen_ip", "127.0.0.1");
        int listen_port = drv.backend_config.value("listen_port", 8081);
        std::string send_ip = drv.backend_config.value("send_ip", "127.0.0.1");
        int send_port = drv.backend_config.value("send_port", 8080);

        // Listen settings
        ui::input::text("Listen IP", listen_ip);
        drv.backend_config["listen_ip"] = listen_ip;

        if (ImGui::InputInt("Listen Port", &listen_port)) {
            drv.backend_config["listen_port"] = listen_port;
        }

        // Send settings
        ui::input::text("Send IP", send_ip);
        drv.backend_config["send_ip"] = send_ip;

        if (ImGui::InputInt("Send Port", &send_port)) {
            drv.backend_config["send_port"] = send_port;
        }
    }

    void render_config_config(driver::instance& drv) {
        ui::separator("Config File Settings");

        std::string config_file = drv.backend_config.value("config_file", "config.json");

        ui::input::text("Config File Path", config_file);
        drv.backend_config["config_file"] = config_file;

        ui::text("The config driver reads values from this JSON file into plugs");
        ui::text("and writes socket values to this file at the configured frequency.");
    }

    void render_editor(driver::instance& drv, const port::object::list& db) {
        ui::id::push(drv.uuid.c_str());

        drv.editor();

        // Display backend type (read-only)
        ui::separator("Driver Type");
        std::string type_label = std::string(type_to_icon(drv.backend_type)) + " " + type_to_display_name(drv.backend_type);
        ui::text(type_label);
        ui::text(type_to_description(drv.backend_type));

        ui::separator("Behavior");

        drv.frequency = static_cast<exec::frequency>(ui::combo("Driver Frequency", static_cast<int>(drv.frequency), exec::frequency_map));

        // Backend-specific configuration
        switch (drv.backend_type) {
            case driver::type::udp:
                render_udp_config(drv);
                break;
            case driver::type::config:
                render_config_config(drv);
                break;
        }

        ui::columns(2);
        ui::separator("Sockets");

        if (ui::button("Insert Socket")) {
            ImGui::OpenPopup("InsertSocketMenu");
        }

        if (ImGui::BeginPopup("InsertSocketMenu")) {
            for (const auto& available_port : db) {
                if (ImGui::MenuItem(available_port.display_name().c_str())) {
                    drv.add_socket(available_port);
                }
            }
            ImGui::EndPopup();
        }

        render_ports(ui::icon::socket, drv.sockets);

        ui::next_column();
        ui::separator("Plugs");

        if (ui::button("Insert Plugs")) {
            ImGui::OpenPopup("InsertPlugsMenu");
        }

        if (ImGui::BeginPopup("InsertPlugsMenu")) {
            for (const auto& available_port : db) {
                if (ImGui::MenuItem(available_port.display_name().c_str())) {
                    drv.add_plug(available_port);
                }
            }
            ImGui::EndPopup();
        }

        render_ports(ui::icon::plug, drv.plugs);

        ui::id::pop();
    }

}  // namespace driver
