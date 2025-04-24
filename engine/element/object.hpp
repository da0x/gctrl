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
#include "database/record.hpp"
#include "code/generator.hpp"
#include "exec/frequency.hpp"
#include "signal/instance.hpp"
#include "function/instance.hpp"

namespace element {

    using json = nlohmann::json;

    struct object : public database::record {
    public:
        using list = std::list<object>;
        std::string code_block;
        signal::instance::list inputs;
        signal::instance::list outputs;
        signal::instance::list memory;
        function::instance::list functions;
        exec::frequency frequency;

        object() : database::record("element"), frequency(exec::frequency::_1_khz) {
            code_block = "// Add your code here";
        }

        object(const nlohmann::json& j, const function::object::list& functions, const signal::object::list& signals) : database::record("element", j), frequency(exec::frequency::_1_khz) {
            code_block = j.value("code_block", "// Add your code here");

            load_instances(j, "inputs", this->inputs, signals);
            load_instances(j, "outputs", this->outputs, signals);
            load_instances(j, "memory", this->memory, signals);
            load_instances(j, "functions", this->functions, functions);
            frequency = static_cast<exec::frequency>(j.value("frequency", static_cast<int>(exec::frequency::_1_khz)));
        }

        json serialize() const override {
            json j = record::serialize();
            j["code_block"] = code_block;
            j["frequency"] = static_cast<int>(frequency);

            for (const auto& i : inputs) {
                j["inputs"].push_back(i.serialize());
            }
            for (const auto& o : outputs) {
                j["outputs"].push_back(o.serialize());
            }
            for (const auto& m : memory) {
                j["memory"].push_back(m.serialize());
            }
            for (const auto& f : functions) {
                j["functions"].push_back(f.serialize());
            }
            return j;
        }

        void generate() const override {
            for (const auto& i : inputs) {
                i.generate();
            }
            for (const auto& o : outputs) {
                o.generate();
            }
            for (const auto& func : functions) {
                func.generate();
            }
            code::begin(display_name());

            for (const auto& f : functions) code::include(f.prototype_filename());

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

            for (const auto& func : functions) {
                code::declare_variable(func.prototype_typename(), func.name);
            }

            std::string function_signature = "void " + exec::code::declare(frequency);
            code::function_begin(function_signature);
            code::block(code_block);
            code::function_end();

            code::struct_end();
            code::namespace_end();
            code::commit(file_name());
        }


        virtual void viewer() const override {
            this->record::viewer();
            ui::text("Element Frequency: " + exec::frequency_to_string(frequency));
        }
    };

} // namespace element
