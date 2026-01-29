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

#include "controls/controller/instance.hpp"
#include "controls/driver/instance.hpp"
#include "controls/link.hpp"
#include <list>
#include <string>
#include <nlohmann/json.hpp>
#include <da0x/uuid.hpp>
#include <algorithm>

namespace machine {
    using nlohmann::json;

    class object : public controls::record, public controls::graph {
    public:
        using list = std::list<object>;
        controller::instance::list controllers;
        driver::instance::list drivers;  // Now stores driver::object directly

        object() : controls::record("machine") {}

        object(const nlohmann::json& j, const controller::object::list& controllers, const port::object::list& ports)
            : controls::record("machine", j) {
            load_instances(j, "controllers", this->controllers, controllers);

            // Load drivers directly as objects (not instances)
            if (j.contains("drivers")) {
                for (const auto& drv_json : j["drivers"]) {
                    this->drivers.emplace_back(drv_json, ports);
                }
            }

            if (j.contains("graph")) {
                this->graph::deserialize(j["graph"]);
            }
        }

        nlohmann::json serialize() const override {
            nlohmann::json j = this->record::serialize();
            j["uuid"] = uuid;
            j["name"] = name;
            j["namespace"] = namespace_;
            j["description"] = description;
            for (const auto& ctrl : controllers) {
                j["controllers"].push_back(ctrl.serialize());
            }
            for (const auto& drv : drivers) {
                j["drivers"].push_back(drv.serialize());
            }
            j["graph"] = this->graph::serialize();
            return j;
        }

        void generate() const override {
            for (const auto& controller : controllers) {
                controller.prototype.generate();
            }
            for (const auto& driver : drivers) {
                driver.generate();
            }
            code::begin(display_name());
            for (const auto& ctrl : controllers) {
                code::line("#include <" + ctrl.prototype.file_name() + ">");
            }
            for (const auto& drv : drivers) {
                code::line("#include <" + drv.file_name() + ">");
            }
            code::begin_gctrl_namespace();
            code::namespace_begin(namespace_);
            code::struct_begin(name);
            code::public_();
            for (const auto& ctrl : controllers) {
                code::declare_variable(ctrl.prototype_typename(), ctrl.name);
            }
            for (const auto& drv : drivers) {
                code::declare_variable(drv.display_name(), drv.name);
            }

            for (auto frequency : exec::frequencies) {
                code::function_begin("void " + exec::code::declare(frequency));
                for (auto& e : controllers) {
                    for (auto& i : e.inputs()) { // sockets
                        auto id = e.id() | i.id();
                        if (is_connected(id)) {
                            try {
                                code::line(variable_name(id) + ".incoming = " + variable_name(connection(id)) + ".outgoing;");
                            }
                            catch (...) {}
                        }
                    }

                    for (auto& o : e.outputs()) { // plugs
                        auto id = e.id() | o.id();
                        if (is_connected(id)) {
                            try {
                                code::line(variable_name(id) + ".incoming = " + variable_name(connection(id)) + ".outgoing;");
                            }
                            catch (...) {}
                        }
                    }

                    code::line(e.name + "." + exec::code::call(frequency) + ";");
                }

                for (auto& e : drivers) {
                    if (e.get_frequency() == frequency) {
                        for (auto& i : e.inputs()) { // sockets
                            auto id = e.id() | i.id();
                            if (is_connected(id)) {
                                try {
                                    code::line(variable_name(id) + ".incoming = " + variable_name(connection(id)) + ".outgoing;");
                                }
                                catch (...) {}
                            }
                        }

                        for (auto& o : e.outputs()) { // plugs
                            auto id = e.id() | o.id();
                            if (is_connected(id)) {
                                try {
                                    code::line(variable_name(id) + ".incoming = " + variable_name(connection(id)) + ".outgoing;");
                                }
                                catch (...) {}
                            }
                        }

                        code::line(e.name + "." + exec::code::call(frequency) + ";");
                    }
                }
                code::function_end();
            }


            code::struct_end();
            code::namespace_end();
            code::end();
            code::commit(file_name());

            // Generate boilerplate files
            code::generate_boilerplate(name);
        }

        std::string variable_name(uint64_t id) const {
            for (auto& e : controllers) {
                for (auto& i : e.inputs()) {
                    if ((e.id() | i.id()) == id) {
                        return e.name + "." + i.name;
                    }
                }
                for (auto& i : e.outputs()) {
                    if ((e.id() | i.id()) == id) {
                        return e.name + "." + i.name;
                    }
                }
            }

            for (auto& e : drivers) {
                for (auto& i : e.inputs()) {
                    if ((e.id() | i.id()) == id) {
                        return e.name + "." + i.name;
                    }
                }
                for (auto& i : e.outputs()) {
                    if ((e.id() | i.id()) == id) {
                        return e.name + "." + i.name;
                    }
                }
            }

            throw std::runtime_error("machine::variable_name() variable not found");
        }

        uint64_t instance_prototype_id(uint64_t id) const {
            for (const auto& controller : controllers) {
                for (const auto& plug : controller.inputs())
                    if (id == (controller.id() | plug.id())) {
                        return plug.prototype_id();
                    }
                for (const auto& socket : controller.outputs()) {
                    if (id == (controller.id() | socket.id())) {
                        return socket.prototype_id();
                    }
                }
            }
            for (const auto& driver : drivers) {
                for (const auto& plug : driver.inputs())
                    if (id == (driver.id() | plug.id())) {
                        return plug.prototype_id();
                    }
                for (const auto& socket : driver.outputs()) {
                    if (id == (driver.id() | socket.id())) {
                        return socket.prototype_id();
                    }
                }
            }

            throw std::runtime_error("machine::instance_prototype_id() instance not found");
        }
    };

}  // namespace machine
