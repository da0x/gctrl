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

#include "database/instance.hpp"
#include "signal/object.hpp"

namespace signal {
    using std::string;

    struct instance : public database::instance {
        using list = std::list<instance>;

        string value = "";

        instance(const signal::object& s) : database::instance(s), value(s.default_value) {}
        instance(const json& j, const signal::object& s) : database::instance(j, s) {
            value = j.value("value", value);
        }
        json serialize() const override {
            nlohmann::json j = this->database::instance::serialize();
            j["value"] = value;
            return j;
        }
        using database::instance::prototype;
        std::string icon() const override { return ui::icon::signal; }
    };

} // namespace signal
