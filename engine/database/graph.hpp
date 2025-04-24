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

#include "database/record.hpp"
#include "database/link.hpp"
#include <list>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace database {

    class graph {
    public:
        using link_list = std::list<database::link>;

    protected:
        link_list links;

    public:
        graph() {}
        virtual ~graph() = default;

        virtual uint64_t instance_prototype_id(uint64_t instance_id) const = 0;

        void add_link(uint64_t start_id, uint64_t end_id) {
            links.emplace_back(static_cast<int>(links.size()) + 1, start_id, end_id);
        }

        void remove_link(uint64_t link_id) {
            links.remove_if([link_id](const database::link& link) {
                return link.id == link_id;
                });
        }

        const link_list& get_links() const {
            return links;
        }

        uint64_t connection(uint64_t instance_id) const {
            for (const auto& link : links) {
                if (link.start == instance_id) {
                    return link.end;
                }
                if (link.end == instance_id) {
                    return link.start;
                }
            }
            throw std::runtime_error("graph::connection() connection not found.");
        }

        bool is_connected(uint64_t instance_id) const {
            for (const auto& link : links) {
                if (link.start == instance_id || link.end == instance_id) {
                    return true;
                }
            }
            return false;
        }

        bool is_connectable(uint64_t id1, uint64_t id2) const {
            return instance_prototype_id(id1) == instance_prototype_id(id2);
        }

        void viewer() const {
            for (const auto& link : links) {
                ui::text("id: " + std::to_string(link.id));
                ui::text("start: " + std::to_string(link.start));
                ui::text("start prototype id: " + std::to_string(instance_prototype_id(link.start)));
                ui::text("end: " + std::to_string(link.end));
                ui::text("end prototype id: " + std::to_string(instance_prototype_id(link.end)));
            }
        }

        nlohmann::json serialize() const {
            nlohmann::json j_links;
            for (const auto& link : links) {
                j_links.push_back(link.serialize());
            }
            return j_links;
        }

        void deserialize(const nlohmann::json& j_links) {
            links.clear();
            for (const auto& link_json : j_links) {
                try {
                    uint64_t start = link_json.at("start");
                    uint64_t end = link_json.at("end");
                    links.emplace_back(link_json.value("id", static_cast<int>(links.size()) + 1), start, end);
                }
                catch (const std::exception& e) {
                    ui::cerr << "Dropped link due to deserialization error: " << e.what() << ui::endl;
                }
            }
        }
    };

}  // namespace database
