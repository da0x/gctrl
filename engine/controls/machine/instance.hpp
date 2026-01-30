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
#include "controls/machine/object.hpp"
#include "ui/icons.hpp"

namespace machine {

    struct instance : public controls::instance {
        using list = std::list<instance>;
        instance(const machine::object& e) : controls::instance(e) {}
        instance(const json& j, const machine::object& e) : controls::instance(j, e) {}
        using controls::instance::prototype;
        std::string icon() const override { return ui::icon::machine; }

        // Access prototype's controllers and drivers
        const controller::instance::list& controllers() const {
            return dynamic_cast<const machine::object*>(&prototype)->controllers;
        }
        const driver::instance::list& drivers() const {
            return dynamic_cast<const machine::object*>(&prototype)->drivers;
        }

        // Access the machine prototype
        const machine::object& machine_prototype() const {
            return *dynamic_cast<const machine::object*>(&prototype);
        }
    };

} // namespace machine
