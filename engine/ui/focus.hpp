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

#include <vector>
#include <string>
#include <nlohmann/json.hpp>

namespace ui {
namespace focus {

    enum class level {
        network,    // Top level - network view showing all machines
        machine,
        controller,
        driver,
        element,
        function,
        port,
        plug,
        socket
    };

    struct focus_entry {
        level type;
        std::string uuid;
        std::string name;

        nlohmann::json serialize() const {
            return {
                {"type", static_cast<int>(type)},
                {"uuid", uuid},
                {"name", name}
            };
        }

        static focus_entry deserialize(const nlohmann::json& j) {
            focus_entry entry;
            entry.type = static_cast<level>(j.value("type", 0));
            entry.uuid = j.value("uuid", "");
            entry.name = j.value("name", "");
            return entry;
        }
    };

    class path {
        std::vector<focus_entry> entries;

    public:
        // Navigate to an entry - handles same-level replacement and hierarchy navigation
        void push(const focus_entry& entry) {
            // Check if already at this item
            if (!entries.empty() && entries.back().uuid == entry.uuid) {
                return;
            }

            // Check if this item already exists in the path - if so, pop to it
            for (size_t i = 0; i < entries.size(); ++i) {
                if (entries[i].uuid == entry.uuid) {
                    pop_to_depth(i + 1);
                    return;
                }
            }

            // Check if navigating to same level - replace instead of push
            // This handles cases like controller A -> controller B (should replace, not stack)
            if (!entries.empty() && entries.back().type == entry.type) {
                entries.back() = entry;
                return;
            }

            // Otherwise, push the new entry
            entries.push_back(entry);
        }

        bool pop() {
            if (entries.empty()) return false;
            entries.pop_back();
            return true;
        }

        void clear() {
            entries.clear();
        }

        const focus_entry& current() const {
            static focus_entry empty{level::machine, "", ""};
            if (entries.empty()) return empty;
            return entries.back();
        }

        const focus_entry* parent() const {
            if (entries.size() < 2) return nullptr;
            return &entries[entries.size() - 2];
        }

        bool empty() const {
            return entries.empty();
        }

        size_t depth() const {
            return entries.size();
        }

        const std::vector<focus_entry>& all() const {
            return entries;
        }

        void pop_to_depth(size_t target_depth) {
            while (entries.size() > target_depth) {
                entries.pop_back();
            }
        }

        nlohmann::json serialize() const {
            nlohmann::json j = nlohmann::json::array();
            for (const auto& entry : entries) {
                j.push_back(entry.serialize());
            }
            return j;
        }

        void deserialize(const nlohmann::json& data) {
            entries.clear();
            if (data.is_array()) {
                for (const auto& item : data) {
                    entries.push_back(focus_entry::deserialize(item));
                }
            }
        }
    };

    inline path current_path;

} // namespace focus
} // namespace ui
