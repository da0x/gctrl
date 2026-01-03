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

#include "controls/element/instance.hpp"
#include "controls/port/instance.hpp"
#include "controls/link.hpp"
#include "controls/graph.hpp"

#include <list>
#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <algorithm>

namespace controller {
    using nlohmann::json;

    class object : public controls::record, public controls::graph {
    public:
        using list = std::list<object>;
        element::instance::list elements;
        port::plug::instance::list plugs;
        port::socket::instance::list sockets;

        object() : controls::record("controller") {}

        object(const json& j, const element::object::list& elements, const port::object::list& ports)
            : controls::record("controller", j) {
            load_instances(j, "elements", this->elements, elements);
            load_instances(j, "plugs", this->plugs, ports);
            load_instances(j, "sockets", this->sockets, ports);

            if (j.contains("graph")) {
                this->graph::deserialize(j["graph"]);
            }
        }

        nlohmann::json serialize() const override {
            auto j = this->record::serialize();
            for (const auto& elem : elements) {
                j["elements"].push_back(elem.serialize());
            }
            for (const auto& plug : plugs) {
                j["plugs"].push_back(plug.serialize());
            }
            for (const auto& socket : sockets) {
                j["sockets"].push_back(socket.serialize());
            }
            j["graph"] = this->graph::serialize();
            return j;
        }

        void generate() const override {
            for (const auto& elem : elements) {
                elem.generate();
            }
            for (const auto& plug : plugs) {
                plug.generate();
            }
            for (const auto& socket : sockets) {
                socket.generate();
            }

            code::begin(display_name());
            for (const auto& elem : elements) {
                code::include(elem.prototype.file_name());
            }
            for (const auto& plug : plugs) {
                code::include(plug.prototype_filename());
            }
            for (const auto& socket : sockets) {
                code::include(socket.prototype_filename());
            }

            code::namespace_begin(namespace_);
            code::struct_begin(name);
            code::private_();
            for (const auto& elem : elements) {
                code::declare_variable(elem.prototype.display_name(), elem.name);
            }
            code::public_();
            for (const auto& plug : plugs) {
                code::declare_variable(plug.prototype_typename(), plug.instance_name());
            }
            for (const auto& socket : sockets) {
                code::declare_variable(socket.prototype_typename(), socket.instance_name());
            }

            for (auto frequency : exec::frequencies) {
                code::function_begin("void " + exec::code::declare(frequency));
                for (auto& e : elements) {
                    if (dynamic_cast<const element::object*>(&e.prototype)->frequency == frequency) {
                        for (auto& i : e.inputs()) {
                            auto id = e.id() | i.id();
                            try {
                                if (is_connected(id)) {
                                    code::line(variable_name(id) + " = " + variable_name(connection(id)) + ";");
                                }
                            }
                            catch (...) {}
                        }
                        code::line(e.name + "." + exec::code::call(frequency) + ";");
                    }
                }
                for (auto& e : plugs) {
                        for (auto& i : e.inputs()) {
                            auto id = e.id() | i.id();
                            try {
                                if (is_connected(id)) {
                                    code::line(variable_name(id) + " = " + variable_name(connection(id)) + ";");
                                }
                            }
                            catch (...) {}
                        }
                }
                for (auto& e : sockets) {
                        for (auto& i : e.inputs()) {
                            auto id = e.id() | i.id();
                            try {
                                if (is_connected(id)) {
                                    code::line(variable_name(id) + " = " + variable_name(connection(id)) + ";");
                                }
                            }
                            catch (...) {}
                        }
                }
                code::function_end();
            }
            code::struct_end();
            code::namespace_end();
            code::commit(file_name());
        }

        std::string variable_name(uint64_t id) const {
            for (auto& e : elements) {
                for (auto& i : e.inputs()) {
                    if ((e.id() | i.id()) == id) {
                        return e.name + ".in." + i.name;
                    }
                }
                for (auto& i : e.outputs()) {
                    if ((e.id() | i.id()) == id) {
                        return e.name + ".out." + i.name;
                    }
                }
            }

            for (auto& e : plugs) {
                for (auto& i : e.inputs()) {
                    if ((e.id() | i.id()) == id) {
                        return e.name + ".outgoing." + i.name;
                    }
                }
                for (auto& i : e.outputs()) {
                    if ((e.id() | i.id()) == id) {
                        return e.name + ".incoming." + i.name;
                    }
                }
            }

            for (auto& e : sockets) {
                for (auto& i : e.inputs()) {
                    if ((e.id() | i.id()) == id) {
                        return e.name + ".outgoing." + i.name;
                    }
                }
                for (auto& i : e.outputs()) {
                    if ((e.id() | i.id()) == id) {
                        return e.name + ".incoming." + i.name;
                    }
                }
            }

            throw std::runtime_error("controller::variable_name() variable not found");
        }

        uint64_t instance_prototype_id(uint64_t id) const override {
            for (auto& e : elements) {
                for (auto& i : e.inputs()) {
                    if ((e.id() | i.id()) == id) {
                        return i.prototype_id();
                    }
                }
                for (auto& i : e.outputs()) {
                    if ((e.id() | i.id()) == id) {
                        return i.prototype_id();
                    }
                }
            }
            for (auto& e : plugs) {
                for (auto& i : e.inputs()) {
                    if ((e.id() | i.id()) == id) {
                        return i.prototype_id();
                    }
                }
                for (auto& i : e.outputs()) {
                    if ((e.id() | i.id()) == id) {
                        return i.prototype_id();
                    }
                }
            }
            for (auto& e : sockets) {
                for (auto& i : e.inputs()) {
                    if ((e.id() | i.id()) == id) {
                        return i.prototype_id();
                    }
                }
                for (auto& i : e.outputs()) {
                    if ((e.id() | i.id()) == id) {
                        return i.prototype_id();
                    }
                }
            }
            throw std::runtime_error("instance_prototype_id() failed to locate id");
        }
    };
}  // namespace controller
