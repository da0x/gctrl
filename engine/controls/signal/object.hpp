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
#include <nlohmann/json.hpp>

#include "controls/record.hpp"
#include "controls/instance.hpp"
#include "controls/code/generator.hpp"

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace signal {

    using nlohmann::json;

    struct object : controls::record {

        using list = std::list<object>;

        std::string type;
        std::string include;
        std::string default_value;

        object(const char* uuid, const char* name, const char* default_value, const char* description, const char* include_path)
            : record(uuid, name, "", description), type(name), default_value(default_value), include(include_path) {}

        object(const json& j)
            : record("", j),
            type(j.value("type", "")),
            default_value(j.value("default_value", "")) {}

        json serialize() const {
            json j = record::serialize();
            j["type"] = type;
            j["default_value"] = default_value;
            return j;
        }

        std::string file_name() const override {
            return namespace_.empty() ? "" : namespace_ + "/" + name + ".hpp";
        }

        void generate() const override {
            return; // unused for now.
            code::begin(display_name());
            if (!include.empty())
                code::include(include);
            code::begin_gctrl_namespace();
            code::namespace_begin(namespace_);
            code::line("typedef " + type + " " + name + ";");
            code::namespace_end();
            code::end();
            code::commit(file_name());
        }
    };

    inline const object::list default_signals = {
        signal::object("12270a17-e1f4-4473-91c3-f698d303e093", "float", "0e+0f", "Real Number", ""),
        signal::object("3a87cbc2-b213-4e5e-8c5a-720dadb19f91", "complex", "(0.0f, 0.0f)", "Complex Number", "complex"),
        signal::object("9786c237-804e-4109-b3d8-e8f9bc43f682", "bool", "false", "Boolean", ""),
//      signal::object("1e5735ac-9f77-481c-839d-258279bf786b", "vec2", "glm::vec2(0.0f, 0.0f)", "2D Vector", "glm/vec2.hpp"),
//      signal::object("3ffe44e9-a734-48d3-8249-f03cb6652cb2", "vec3", "glm::vec3(0.0f, 0.0f, 0.0f)", "3D Vector", "glm/vec3.hpp")
    };

} // namespace signal

