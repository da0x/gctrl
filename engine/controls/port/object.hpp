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
#include "controls/record.hpp"
#include "controls/instance.hpp"
#include "controls/signal/object.hpp"
#include <nlohmann/json.hpp>
#include <da0x/uuid.hpp>

namespace port {

    using json = nlohmann::json;
    const std::string incoming = "socket_data";
    const std::string outgoing = "plug_data";

    struct object : public controls::record {
        using list = std::list<object>;
        signal::instance::list incoming;
        signal::instance::list outgoing;

        object() : controls::record("port") {}

        object(const json& j, const signal::object::list& signals) : controls::record("port", j) {
            load_instances(j, port::incoming, incoming, signals);
            load_instances(j, port::outgoing, outgoing, signals);
        }

        json serialize() const override {
            json j = this->record::serialize();

            for (const auto& sig : incoming) {
                j[port::incoming].push_back(sig.serialize());
            }

            for (const auto& sig : outgoing) {
                j[port::outgoing].push_back(sig.serialize());
            }

            return j;
        }

        void generate() const override {
            for (const auto& i : incoming) {
                i.generate();
            }
            for (const auto& o : outgoing) {
                o.generate();
            }
            code::begin(display_name());
            code::begin_gctrl_namespace();
            code::namespace_begin(namespace_);
            code::struct_begin(name);
            code::public_();

            code::struct_begin(port::incoming);
            for (const auto& socket_signal : incoming) {
                code::declare_variable(socket_signal.prototype_typename(), socket_signal.name, /*ui::input::engineering::to_string*/(socket_signal.value));
            }
            code::struct_end();

            code::struct_begin(port::outgoing);
            for (const auto& plug_signal : outgoing) {
                code::declare_variable(plug_signal.prototype_typename(), plug_signal.name, /*ui::input::engineering::to_string*/(plug_signal.value));
            }
            code::struct_end();

            code::struct_end();
            code::namespace_end();
            code::end();
            code::commit(file_name());
        }

    };

} // namespace port

