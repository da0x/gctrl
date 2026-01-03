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

#include "controls/instance.hpp"
#include "controls/signal/instance.hpp"
#include "controls/element/object.hpp"
#include <nlohmann/json.hpp>

namespace element {
    using nlohmann::json;

    struct instance : public ::controls::instance {
        using list = std::list<instance>;
        instance(const element::object& e) : ::controls::instance(e) {}
        instance(const json& j, const element::object& e) : controls::instance(j, e) {}
        using ::controls::instance::prototype;
        const signal::instance::list& inputs() const {
            return dynamic_cast<const element::object*>(&prototype)->inputs;
        }
        const signal::instance::list& outputs() const {
            return dynamic_cast<const element::object*>(&prototype)->outputs;
        }

        std::string icon() const override { return ui::icon::element; }
    };

} // namespace element
