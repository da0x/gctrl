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
#include "controls/port/object.hpp"

namespace port {
    using nlohmann::json;


    struct instance : protected controls::instance {
        const signal::instance::list incoming;
        const signal::instance::list outgoing;

    protected:
        const std::string type;
        instance(const port::object& p, signal::instance::list in, signal::instance::list out, const char* type) : controls::instance(p), incoming(in), outgoing(out), type(type) {}
        instance(const json &j, const port::object& p, signal::instance::list in, signal::instance::list out, const char* type) : controls::instance(j, p), incoming(in), outgoing(out), type(type) {}
    public:

        json serialize() const override {
            return this->::controls::instance::serialize();
        }

        std::string prototype_typename() const override {
            return prototype.namespace_ + "::" + type + "::" + prototype.name;
        }

        std::string prototype_filename() const override {
            auto& p = *dynamic_cast<const port::object*>(&prototype);
            return p.namespace_ + "/" + p.name + "_" + type + ".hpp";
        }

        std::string icon() const override { return ui::icon::plug; }

        using controls::instance::uuid;
        using controls::instance::id;
        using controls::instance::name;
        using controls::instance::instance_name;
        using controls::instance::prototype_id;
        using controls::instance::get_prototype_uuid;
        using controls::instance::direction;
        using controls::instance::list;

        const signal::instance::list& inputs() const {
            return incoming;
        }
        const signal::instance::list& outputs() const {
            return outgoing;
        }

        void generate() const override {
            auto& port = *dynamic_cast<const port::object*>(&prototype);
            port.generate();

            auto incoming_type = type == "socket" ? port::incoming : port::outgoing;
            auto outgoing_type = type == "socket" ? port::outgoing : port::incoming;

            code::begin(prototype_typename());
            code::include(port.file_name());
            code::begin_gctrl_namespace();
            code::namespace_begin(port.namespace_);
            code::namespace_begin(type);
            code::struct_begin(port.name);
            code::public_();
            code::declare_variable(port.display_name() + "::" + incoming_type, "outgoing");
            code::declare_variable(port.display_name() + "::" + outgoing_type, "incoming");
            code::struct_end();
            code::namespace_end();
            code::namespace_end();
            code::end();
            code::commit(prototype_filename());
        }

        using controls::instance::editor;
    };

    namespace socket {
        struct instance : public port::instance {
            using list = std::list<instance>;
            instance(const port::object& p) : port::instance(p, p.incoming, p.outgoing, "socket") {}
            instance(const json& j, const port::object& p) : port::instance(j, p, p.incoming, p.outgoing, "socket") {}
            std::string icon() const override { return ui::icon::socket; }
        };
    }
    namespace plug {
        struct instance : public port::instance {
            using list = std::list<instance>;
            instance(const port::object& p) : port::instance(p, p.outgoing, p.incoming, "plug") {}
            instance(const json& j, const port::object& p) : port::instance(j, p, p.outgoing, p.incoming, "plug") {}
            std::string icon() const override { return ui::icon::plug; }
        };
    }

}

