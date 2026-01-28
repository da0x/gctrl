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
#include "controls/port/object.hpp"
#include "controls/record.hpp"
#include <nlohmann/json.hpp>
#include "controls/code/generator.hpp"
#include "controls/exec/frequency.hpp"

namespace driver {

    using json = nlohmann::json;

    struct object : public controls::record {
    public:
        using list = std::list<object>;
        std::list<port::plug::instance> plugs;
        std::list<port::socket::instance> sockets;
        exec::frequency frequency;

        object() : controls::record("driver"), frequency(exec::frequency::_1_hz) {}

        object(const nlohmann::json& j, const port::object::list& ports) : controls::record("driver", j), frequency(exec::frequency::_1_khz) {
            load_instances(j, "plugs", this->plugs, ports);
            load_instances(j, "sockets", this->sockets, ports);
            frequency = static_cast<exec::frequency>(j.value("frequency", static_cast<int>(exec::frequency::_1_khz)));
        }

        void add_plug(const port::object& p) {
            plugs.push_back(port::plug::instance(p));
        }
        void add_socket(const port::object& p) {
            sockets.push_back(port::socket::instance(p));
        }

        json serialize() const override {
            json j = record::serialize();

            for (const auto& plug : plugs) {
                j["plugs"].push_back(plug.serialize());
            }
            for (const auto& socket : sockets) {
                j["sockets"].push_back(socket.serialize());
            }

            j["frequency"] = static_cast<int>(frequency);
            return j;
        }

        void generate() const override {
            for (const auto& plug : plugs) {
                plug.generate();
            }
            for (const auto& socket : sockets) {
                socket.generate();
            }
            code::begin(display_name());
            for (const auto& plug : plugs) {
                code::include(plug.prototype_filename());
            }
            for (const auto& socket : sockets) {
                code::include(socket.prototype_filename());
            }

            code::begin_gctrl_namespace();
            code::namespace_begin(namespace_);
            code::struct_begin(name);
            code::private_();
            code::line("gctrl::udp::signal_registry registry;");
            code::line("gctrl::udp::listener listener;");
            code::line("gctrl::udp::sender sender;");
            code::public_();
            for (const auto& plug : plugs) {
                code::declare_variable(plug.prototype_typename(), plug.instance_name());
            }
            for (const auto& socket : sockets) {
                code::declare_variable(socket.prototype_typename(), socket.instance_name());
            }

            code::function_begin(name + "() : listener(\"127.0.0.1\", 8081, registry), sender(\"127.0.0.1\", 8080)");
            code::line("listener.start();");
            for (auto& plug : plugs) {
                if (!plug.outgoing.empty()) {
                    code::line("// incoming data from udp device:");
                    for (auto& i : plug.outgoing) {
                        std::stringstream s;
                        s << "registry.subscribe(" << (plug.id() | i.id()) << ", [&](const std::vector<uint8_t>& data) {\n"
                            << "    " << plug.name + ".outgoing." + i.name << " = gctrl::binary::deserialize<" << i.prototype_typename() << ">(data); \n"
                            << "});";
                        code::block(s.str());
                    }
                }
            }
            for (auto& socket : sockets) {
                if (!socket.incoming.empty()) {
                    code::line("// incoming data from udp device:");
                    for (auto& i : socket.incoming) {
                        std::stringstream s;
                        s << "registry.subscribe(" << (socket.id() | i.id()) << ", [&](const std::vector<uint8_t>& data) {\n"
                            << "    " << socket.name + ".outgoing." + i.name << " = gctrl::binary::deserialize<" << i.prototype_typename() << ">(data);\n"
                            << "});";
                        code::block(s.str());
                    }
                }
            }
            code::function_end();

            std::string function_signature = "void " + exec::code::declare(frequency);
            code::function_begin(function_signature);

            for (auto& socket : sockets) {
                if (!socket.outgoing.empty()) {
                    code::line("// send socket data incoming from the controllers");
                    for (auto& i : socket.outgoing) {
                        std::stringstream s;
                        s << "sender.send(" << (socket.id() | i.id()) << ", gctrl::binary::serialize(" << socket.name + ".incoming." + i.name << "));";
                        code::line(s.str());
                    }
                }
            }
            
            for (auto& plug : plugs) {
                if (!plug.outgoing.empty()) {
                    code::line("// send plug data outgoing from the controllers");
                    for (auto& i : plug.outgoing) {
                        std::stringstream s;
                        s << "sender.send(" << (plug.id() | i.id()) << ", gctrl::binary::serialize(" << plug.name + ".incoming." + i.name << "));";
                        code::line(s.str());
                    }
                }
            }

            code::function_end();
            code::struct_end();
            code::namespace_end();
            code::end();
            code::commit(file_name());
        }

        virtual void viewer() const override {
            this->record::viewer();
            ui::text("Driver Frequency: " + exec::frequency_to_string(frequency));
        }
    };

} // namespace driver
