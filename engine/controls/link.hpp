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

namespace controls {

    struct link {
        int id;
        uint64_t start;
        uint64_t end;

        link() : id(0), start(0), end(0) {}

        link(int id, uint64_t start, uint64_t end)
            : id(id), start(start), end(end) {}

        link(const nlohmann::json& j)
            : id(j.value("id", 0)),
            start(j.at("start")),
            end(j.at("end")) {}

        nlohmann::json serialize() const {
            return {
                {"id", id},
                {"start", start},
                {"end", end}
            };
        }
    };

} // namespace controls
