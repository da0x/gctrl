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
#include "controls/record.hpp"
#include "controls/signal/object.hpp"
#include "controls/code/generator.hpp"

namespace function {

    using json = nlohmann::json;

    struct object : public controls::record {
    public:
        using list = std::list<object>;
        std::string code_block;
        signal::instance::list inputs;
        signal::instance::list outputs;
        signal::instance::list memory;

        object() : controls::record("function") {
            code_block = "// Add your code here";
        }

        object(const nlohmann::json& j, const signal::object::list& signals) : controls::record("function", j) {
            code_block = j.value("code_block", "// Add your code here");

            load_instances(j, "inputs", inputs, signals);
            load_instances(j, "outputs", outputs, signals);
            load_instances(j, "memory", memory, signals);
        }

        nlohmann::json serialize() const override {
            nlohmann::json j = this->record::serialize();
            j["code_block"] = code_block;

            for (const auto& input : inputs) {
                j["inputs"].push_back(input.serialize());
            }

            for (const auto& output : outputs) {
                j["outputs"].push_back(output.serialize());
            }

            for (const auto& mem : memory) {
                j["memory"].push_back(mem.serialize());
            }

            return j;
        }

        void generate() const override {
            code::begin(display_name());
            code::begin_gctrl_namespace();
            code::namespace_begin(namespace_);
            code::struct_begin(name);
            code::public_();

            if (!inputs.empty()) {
                code::struct_begin("input_type");
                for (const auto& input : inputs) {
                    code::declare_variable(input.prototype_typename(), input.name, /*ui::input::engineering::to_string*/(input.value));
                }
                code::struct_end("in");
            }

            if (!outputs.empty()) {
                code::struct_begin("output_type");
                for (const auto& output : outputs) {
                    code::declare_variable(output.prototype_typename(), output.name, /*ui::input::engineering::to_string*/(output.value));
                }
                code::struct_end("out");
            }

            if (!memory.empty()) {
                code::struct_begin("memory_type");
                for (const auto& mem : memory) {
                    code::declare_variable(mem.prototype_typename(), mem.name, /*ui::input::engineering::to_string*/(mem.value));
                }
                code::struct_end("mem");
            }

            code::function_begin("void operator()()");
            code::block(code_block);
            code::function_end();

            code::struct_end();
            code::namespace_end();
            code::end();
            code::commit(file_name());
        }
    };

} // namespace element
