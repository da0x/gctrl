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
#include "controls/driver/type.hpp"
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

    class object {
    public:
        machine::object::list machines;

        object() = default;

        object(const json& j, const controller::object::list& controllers, const port::object::list& ports) {
            if (j.contains("machines")) {
                for (const auto& mach_json : j["machines"]) {
                    machines.emplace_back(mach_json, controllers, ports);
                }
            }
        }

        json serialize() const {
            json j;
            json machines_json = json::array();
            for (const auto& mach : machines) {
                machines_json.push_back(mach.serialize());
            }
            j["machines"] = machines_json;
            return j;
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
