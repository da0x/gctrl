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

#include <cstdint>
#include <nlohmann/json.hpp>

namespace ui {

    // Single source of truth for selection state
    // Passed by reference to hierarchy and all editors
    struct selection {
        uint64_t selected_id = 0;

        void select(uint64_t id) {
            selected_id = id;
        }

        void clear() {
            selected_id = 0;
        }

        bool is_selected(uint64_t id) const {
            return selected_id != 0 && selected_id == id;
        }

        bool has_selection() const {
            return selected_id != 0;
        }

        nlohmann::json serialize() const {
            return {{"selected_id", selected_id}};
        }

        void deserialize(const nlohmann::json& j) {
            selected_id = j.value("selected_id", 0ULL);
        }
    };

} // namespace ui
