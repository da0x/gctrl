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
#include "driver/object.hpp"

namespace driver {

    struct instance : public database::instance {
        using list = std::list<instance>;
        instance(const driver::object& e) : database::instance(e) {}
        instance(const json& j, const driver::object& e) : database::instance(j, e) {}
        using database::instance::prototype;
        const port::socket::instance::list& inputs() const {
            return dynamic_cast<const driver::object*>(&prototype)->sockets;
        }
        const port::plug::instance::list& outputs() const {
            return dynamic_cast<const driver::object*>(&prototype)->plugs;
        }
        std::string icon() const override { return ui::icon::driver; }

        exec::frequency prototype_frequency() const {
            return dynamic_cast<const driver::object*>(&prototype)->frequency;
        }
    };

} // namespace driver