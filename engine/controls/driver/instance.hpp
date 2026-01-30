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
#include "controls/driver/type.hpp"
#include <nlohmann/json.hpp>
#include "controls/code/generator.hpp"
#include "controls/exec/frequency.hpp"

namespace driver {

    using json = nlohmann::json;

    // Driver instance - a specific driver in a machine with its own configuration
    // The backend_type determines which built-in driver backend (UDP, Config) is used
    struct instance : public controls::record {
    public:
        using list = std::list<instance>;
        std::list<port::plug::instance> plugs;
        std::list<port::socket::instance> sockets;
        exec::frequency frequency;
        driver::type backend_type;
        nlohmann::json backend_config;
        ui::direction direction = ui::direction::ltr;

        // Default constructor
        instance() : controls::record("driver"), frequency(exec::frequency::_1_hz), backend_type(driver::type::udp) {
            init_backend_config();
        }

        // Constructor with backend type - used when adding a new driver to a machine
        explicit instance(driver::type t) : controls::record("driver"), frequency(exec::frequency::_1_hz), backend_type(t) {
            init_backend_config();
        }

        // Deserialization constructor
        instance(const nlohmann::json& j, const port::object::list& ports) : controls::record("driver", j), frequency(exec::frequency::_1_khz) {
            load_instances(j, "plugs", this->plugs, ports);
            load_instances(j, "sockets", this->sockets, ports);
            frequency = static_cast<exec::frequency>(j.value("frequency", static_cast<int>(exec::frequency::_1_khz)));

            // Backend type - defaults to udp for backward compatibility
            std::string type_str = j.value("backend_type", "udp");
            backend_type = string_to_type(type_str);

            // Backend config
            if (j.contains("backend_config")) {
                backend_config = j["backend_config"];
            } else {
                init_backend_config();
            }

            // Direction - defaults to ltr
            std::string dir = j.value("direction", "ltr");
            direction = (dir == "rtl") ? ui::direction::rtl : ui::direction::ltr;
        }

        std::string instance_name() const {
            return name;
        }

        std::string prototype_typename() const {
            return type_to_typename(backend_type);
        }

        std::string icon() const {
            return type_to_icon(backend_type);
        }

        void init_backend_config() {
            switch (backend_type) {
                case driver::type::udp:
                    backend_config["listen_ip"] = "127.0.0.1";
                    backend_config["listen_port"] = 8081;
                    backend_config["send_ip"] = "127.0.0.1";
                    backend_config["send_port"] = 8080;
                    break;
                case driver::type::config:
                    backend_config["config_file"] = "config.json";
                    break;
            }
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
            j["backend_type"] = type_to_string(backend_type);
            j["backend_config"] = backend_config;
            j["direction"] = (direction == ui::direction::ltr) ? "ltr" : "rtl";
            return j;
        }

        void generate() const override {
            for (const auto& plug : plugs) {
                plug.generate();
            }
            for (const auto& socket : sockets) {
                socket.generate();
            }

            switch (backend_type) {
                case driver::type::udp:
                    generate_udp();
                    break;
                case driver::type::config:
                    generate_config();
                    break;
            }
        }

    private:
        void generate_udp() const {
            std::string listen_ip = backend_config.value("listen_ip", "127.0.0.1");
            int listen_port = backend_config.value("listen_port", 8081);
            std::string send_ip = backend_config.value("send_ip", "127.0.0.1");
            int send_port = backend_config.value("send_port", 8080);

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

            std::stringstream ctor;
            ctor << name << "() : listener(\"" << listen_ip << "\", " << listen_port << ", registry), sender(\"" << send_ip << "\", " << send_port << ")";
            code::function_begin(ctor.str());
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

        void generate_config() const {
            std::string config_file = backend_config.value("config_file", "config.json");

            code::begin(display_name());
            code::line("#include <fstream>");
            code::line("#include <nlohmann/json.hpp>");
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
            code::line("std::string config_path = \"" + config_file + "\";");
            code::public_();
            for (const auto& plug : plugs) {
                code::declare_variable(plug.prototype_typename(), plug.instance_name());
            }
            for (const auto& socket : sockets) {
                code::declare_variable(socket.prototype_typename(), socket.instance_name());
            }

            // Constructor - loads config on startup
            code::function_begin(name + "()");
            code::line("load();");
            code::function_end();

            // Load method - reads config values into plug outgoing signals
            code::function_begin("void load()");
            code::line("std::ifstream file(config_path);");
            code::line("if (!file.is_open()) return;");
            code::line("nlohmann::json j;");
            code::line("file >> j;");
            for (auto& plug : plugs) {
                for (auto& i : plug.outgoing) {
                    std::string key = plug.name + "_" + i.name;
                    code::line("if (j.contains(\"" + key + "\")) " + plug.name + ".outgoing." + i.name + " = j[\"" + key + "\"].get<" + i.prototype_typename() + ">();");
                }
            }
            code::function_end();

            // Save method - writes socket incoming signals to config
            code::function_begin("void save()");
            code::line("nlohmann::json j;");
            for (auto& socket : sockets) {
                for (auto& i : socket.incoming) {
                    std::string key = socket.name + "_" + i.name;
                    code::line("j[\"" + key + "\"] = " + socket.name + ".incoming." + i.name + ";");
                }
            }
            code::line("std::ofstream file(config_path);");
            code::line("file << j.dump(4);");
            code::function_end();

            // Execution function - saves at configured frequency
            std::string function_signature = "void " + exec::code::declare(frequency);
            code::function_begin(function_signature);
            code::line("save();");
            code::function_end();

            code::struct_end();
            code::namespace_end();
            code::end();
            code::commit(file_name());
        }

    public:
        virtual void viewer() const override {
            this->record::viewer();
            ui::text("Driver Type: " + type_to_display_name(backend_type));
            ui::text("Driver Frequency: " + exec::frequency_to_string(frequency));
        }

        // Interface methods for machine to access ports
        const port::socket::instance::list& inputs() const {
            return sockets;
        }
        const port::plug::instance::list& outputs() const {
            return plugs;
        }
        exec::frequency get_frequency() const {
            return frequency;
        }
    };

} // namespace driver
