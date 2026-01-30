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

#include "controls/record.hpp"
#include "controls/link.hpp"
#include <list>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace controls {

    class graph {
    public:
        using link_list = std::list<controls::link>;

    protected:
        link_list links;
        int next_link_id = 1;  // Monotonically increasing link ID

    public:
        graph() {}
        virtual ~graph() = default;

        virtual uint64_t instance_prototype_id(uint64_t instance_id) const = 0;

        // Check if a link already exists between two pins (in either direction)
        bool link_exists(uint64_t start_id, uint64_t end_id) const {
            for (const auto& link : links) {
                if ((link.start == start_id && link.end == end_id) ||
                    (link.start == end_id && link.end == start_id)) {
                    return true;
                }
            }
            return false;
        }

        void add_link(uint64_t start_id, uint64_t end_id) {
            // Prevent duplicate links
            if (link_exists(start_id, end_id)) {
                return;
            }
            links.emplace_back(next_link_id++, start_id, end_id);
        }

        void remove_link(uint64_t link_id) {
            links.remove_if([link_id](const controls::link& link) {
                return link.id == link_id;
            });
        }

        // Remove all links connected to a given instance/pin ID
        void remove_links_to(uint64_t instance_id) {
            links.remove_if([instance_id](const controls::link& link) {
                return link.start == instance_id || link.end == instance_id;
            });
        }

        // Remove all links connected to any pin of a node (given node_id mask)
        void remove_links_to_node(uint64_t node_id) {
            // Node ID is the upper bits; pin IDs have node_id | signal_id
            // We check if either endpoint's upper bits match the node_id
            links.remove_if([node_id](const controls::link& link) {
                // Check if the link's start or end belongs to this node
                // A pin ID is: instance.id() | signal.id() where instance.id() is node_id shifted
                // We need to check if the pin belongs to this node
                return (link.start & ~0xFFFFULL) == node_id ||
                       (link.end & ~0xFFFFULL) == node_id;
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
            nlohmann::json j;
            j["next_link_id"] = next_link_id;
            nlohmann::json j_links;
            for (const auto& link : links) {
                j_links.push_back(link.serialize());
            }
            j["links"] = j_links;
            return j;
        }

        void deserialize(const nlohmann::json& j) {
            links.clear();
            // Handle both old format (array of links) and new format (object with next_link_id)
            nlohmann::json j_links;
            if (j.is_array()) {
                // Old format: just an array of links
                j_links = j;
                next_link_id = 1;
            } else {
                // New format: object with next_link_id and links array
                next_link_id = j.value("next_link_id", 1);
                j_links = j.value("links", nlohmann::json::array());
            }

            int max_id = 0;
            for (const auto& link_json : j_links) {
                try {
                    uint64_t start = link_json.at("start");
                    uint64_t end = link_json.at("end");
                    int id = link_json.value("id", static_cast<int>(links.size()) + 1);
                    max_id = std::max(max_id, id);
                    links.emplace_back(id, start, end);
                }
                catch (const std::exception& e) {
                    ui::cerr << "Dropped link due to deserialization error: " << e.what() << ui::endl;
                }
            }
            // Ensure next_link_id is greater than any existing ID
            next_link_id = std::max(next_link_id, max_id + 1);
        }
    };

}  // namespace controls
