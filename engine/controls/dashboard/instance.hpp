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

#include <string>
#include <list>
#include <nlohmann/json.hpp>
#include <da0x/uuid.hpp>

namespace dashboard {
    using nlohmann::json;

    struct instance {
        using list = std::list<instance>;

        std::string uuid;
        std::string name;
        std::string connected_machine_uuid;  // UUID of connected machine (empty if not connected)
        std::string connected_driver_uuid;   // UUID of connected driver (empty if not connected)
        float node_size = 400.0f;

        // Default constructor
        instance() {
            uuid = uuid::v4::generate();
            name = "dashboard";
        }

        // Deserialization constructor
        instance(const json& j) {
            uuid = j.value("uuid", uuid::v4::generate());
            name = j.value("name", "dashboard");
            connected_machine_uuid = j.value("connected_machine_uuid", "");
            connected_driver_uuid = j.value("connected_driver_uuid", "");
            node_size = j.value("node_size", 400.0f);
        }

        json serialize() const {
            json j;
            j["uuid"] = uuid;
            j["name"] = name;
            j["connected_machine_uuid"] = connected_machine_uuid;
            j["connected_driver_uuid"] = connected_driver_uuid;
            j["node_size"] = node_size;
            return j;
        }

        uint64_t id() const {
            return uuid::uuid_to_uint64(uuid);
        }

        bool is_connected() const {
            return !connected_machine_uuid.empty() && !connected_driver_uuid.empty();
        }

        void connect(const std::string& machine_uuid, const std::string& driver_uuid) {
            connected_machine_uuid = machine_uuid;
            connected_driver_uuid = driver_uuid;
        }

        void disconnect() {
            connected_machine_uuid.clear();
            connected_driver_uuid.clear();
        }

        std::string instance_name() const {
            return name;
        }
    };

} // namespace dashboard
