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

#include "controls/machine/object.hpp"
#include "controls/machine/instance.hpp"
#include "controls/driver/type.hpp"
#include "controls/dashboard/instance.hpp"
#include "controls/panel/instance.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace network {
    using nlohmann::json;

    // UDP endpoint info extracted from a driver
    struct udp_endpoint {
        std::string listen_ip;
        int listen_port;
        std::string send_ip;
        int send_port;
    };

    // Dashboard-to-driver link (dashboard reads from driver output)
    struct dashboard_link {
        uint64_t id;
        std::string dashboard_uuid;
        std::string machine_uuid;
        std::string driver_uuid;

        json serialize() const {
            json j;
            j["id"] = id;
            j["dashboard_uuid"] = dashboard_uuid;
            j["machine_uuid"] = machine_uuid;
            j["driver_uuid"] = driver_uuid;
            return j;
        }

        dashboard_link() : id(0) {}

        dashboard_link(const json& j) {
            id = j.value("id", 0);
            dashboard_uuid = j.value("dashboard_uuid", "");
            machine_uuid = j.value("machine_uuid", "");
            driver_uuid = j.value("driver_uuid", "");
        }
    };

    // ConfigPanel-to-driver link (config panel sends to driver input)
    struct panel_link {
        uint64_t id;
        std::string panel_uuid;
        std::string machine_uuid;
        std::string driver_uuid;

        json serialize() const {
            json j;
            j["id"] = id;
            j["panel_uuid"] = panel_uuid;
            j["machine_uuid"] = machine_uuid;
            j["driver_uuid"] = driver_uuid;
            return j;
        }

        panel_link() : id(0) {}

        panel_link(const json& j) {
            id = j.value("id", 0);
            panel_uuid = j.value("panel_uuid", "");
            machine_uuid = j.value("machine_uuid", "");
            driver_uuid = j.value("driver_uuid", "");
        }
    };

    class object {
    public:
        machine::instance::list machines;
        dashboard::instance::list dashboards;
        panel::instance::list panels;
        std::list<dashboard_link> dashboard_links;
        std::list<panel_link> panel_links;
        uint64_t next_dashboard_link_id = 1;
        uint64_t next_panel_link_id = 1;

        object() = default;

        object(const json& j, const machine::object::list& machine_db) {
            load_instances(j, "machines", machines, machine_db);

            // Load dashboards
            if (j.contains("dashboards")) {
                for (const auto& dash_json : j["dashboards"]) {
                    dashboards.emplace_back(dash_json);
                }
            }

            // Load config panels
            if (j.contains("panels")) {
                for (const auto& panel_json : j["panels"]) {
                    panels.emplace_back(panel_json);
                }
            }

            // Load dashboard links
            if (j.contains("dashboard_links")) {
                for (const auto& link_json : j["dashboard_links"]) {
                    dashboard_links.emplace_back(link_json);
                }
            }

            // Load config panel links
            if (j.contains("panel_links")) {
                for (const auto& link_json : j["panel_links"]) {
                    panel_links.emplace_back(link_json);
                }
            }

            next_dashboard_link_id = j.value("next_dashboard_link_id", 1);
            next_panel_link_id = j.value("next_panel_link_id", 1);
        }

        json serialize() const {
            json j;
            json machines_json = json::array();
            for (const auto& mach : machines) {
                machines_json.push_back(mach.serialize());
            }
            j["machines"] = machines_json;

            // Serialize dashboards
            json dashboards_json = json::array();
            for (const auto& dash : dashboards) {
                dashboards_json.push_back(dash.serialize());
            }
            j["dashboards"] = dashboards_json;

            // Serialize config panels
            json panels_json = json::array();
            for (const auto& panel : panels) {
                panels_json.push_back(panel.serialize());
            }
            j["panels"] = panels_json;

            // Serialize dashboard links
            json dash_links_json = json::array();
            for (const auto& link : dashboard_links) {
                dash_links_json.push_back(link.serialize());
            }
            j["dashboard_links"] = dash_links_json;
            j["next_dashboard_link_id"] = next_dashboard_link_id;

            // Serialize config panel links
            json panel_links_json = json::array();
            for (const auto& link : panel_links) {
                panel_links_json.push_back(link.serialize());
            }
            j["panel_links"] = panel_links_json;
            j["next_panel_link_id"] = next_panel_link_id;

            return j;
        }

        // Find dashboard by node ID
        dashboard::instance* find_dashboard(uint64_t node_id) {
            for (auto& dash : dashboards) {
                if (dash.id() == node_id) return &dash;
            }
            return nullptr;
        }

        // Find dashboard by UUID
        dashboard::instance* find_dashboard_by_uuid(const std::string& uuid) {
            for (auto& dash : dashboards) {
                if (dash.uuid == uuid) return &dash;
            }
            return nullptr;
        }

        // Add a dashboard link
        uint64_t add_dashboard_link(const std::string& dashboard_uuid,
                                    const std::string& machine_uuid,
                                    const std::string& driver_uuid) {
            dashboard_link link;
            link.id = next_dashboard_link_id++;
            link.dashboard_uuid = dashboard_uuid;
            link.machine_uuid = machine_uuid;
            link.driver_uuid = driver_uuid;
            dashboard_links.push_back(link);

            // Update dashboard connection info
            auto* dash = find_dashboard_by_uuid(dashboard_uuid);
            if (dash) {
                dash->connect(machine_uuid, driver_uuid);
            }

            return link.id;
        }

        // Remove a dashboard link by ID
        void remove_dashboard_link(uint64_t link_id) {
            for (auto it = dashboard_links.begin(); it != dashboard_links.end(); ++it) {
                if (it->id == link_id) {
                    // Disconnect the dashboard
                    auto* dash = find_dashboard_by_uuid(it->dashboard_uuid);
                    if (dash) {
                        dash->disconnect();
                    }
                    dashboard_links.erase(it);
                    return;
                }
            }
        }

        // Find link by dashboard UUID
        const dashboard_link* find_link_for_dashboard(const std::string& dashboard_uuid) const {
            for (const auto& link : dashboard_links) {
                if (link.dashboard_uuid == dashboard_uuid) {
                    return &link;
                }
            }
            return nullptr;
        }

        // Check if a link exists between dashboard pin and driver pin
        bool has_link(uint64_t dashboard_pin_id, uint64_t driver_pin_id) const {
            // Not used for rendering - links are managed separately
            return false;
        }

        // Config panel methods
        panel::instance* find_panel(uint64_t node_id) {
            for (auto& panel : panels) {
                if (panel.id() == node_id) return &panel;
            }
            return nullptr;
        }

        panel::instance* find_panel_by_uuid(const std::string& uuid) {
            for (auto& panel : panels) {
                if (panel.uuid == uuid) return &panel;
            }
            return nullptr;
        }

        uint64_t add_panel_link(const std::string& panel_uuid,
                                      const std::string& machine_uuid,
                                      const std::string& driver_uuid) {
            panel_link link;
            link.id = next_panel_link_id++;
            link.panel_uuid = panel_uuid;
            link.machine_uuid = machine_uuid;
            link.driver_uuid = driver_uuid;
            panel_links.push_back(link);

            auto* panel = find_panel_by_uuid(panel_uuid);
            if (panel) {
                panel->connect(machine_uuid, driver_uuid);
            }

            return link.id;
        }

        void remove_panel_link(uint64_t link_id) {
            for (auto it = panel_links.begin(); it != panel_links.end(); ++it) {
                if (it->id == link_id) {
                    auto* panel = find_panel_by_uuid(it->panel_uuid);
                    if (panel) {
                        panel->disconnect();
                    }
                    panel_links.erase(it);
                    return;
                }
            }
        }

        const panel_link* find_link_for_panel(const std::string& panel_uuid) const {
            for (const auto& link : panel_links) {
                if (link.panel_uuid == panel_uuid) {
                    return &link;
                }
            }
            return nullptr;
        }

        // Extract UDP endpoints from a machine's drivers
        // GCtrl listens on machine's send_port (receives what machine sends)
        // GCtrl sends to machine's listen_port (where machine listens)
        static std::vector<udp_endpoint> get_udp_endpoints(const machine::object& mach) {
            std::vector<udp_endpoint> endpoints;
            for (const auto& drv : mach.drivers) {
                if (drv.backend_type == driver::type::udp) {
                    udp_endpoint ep;
                    ep.listen_ip = drv.backend_config.value("send_ip", "127.0.0.1");
                    ep.listen_port = drv.backend_config.value("send_port", 8080);
                    ep.send_ip = drv.backend_config.value("listen_ip", "127.0.0.1");
                    ep.send_port = drv.backend_config.value("listen_port", 8081);
                    endpoints.push_back(ep);
                }
            }
            return endpoints;
        }
    };

} // namespace network
