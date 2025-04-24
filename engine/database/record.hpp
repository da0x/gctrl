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
#include <nlohmann/json.hpp>
#include <da0x/uuid.hpp>

namespace database {
    using nlohmann::json;
    using std::string;

    class record {
    public:
        std::string uuid;
        std::string name;
        std::string namespace_;
        std::string description;

        virtual ~record() = default;

        virtual json serialize() const {
            json j;
            j["uuid"] = uuid;
            j["name"] = name;
            j["namespace"] = namespace_;
            j["description"] = description;
            return j;
        }

        virtual uint64_t id() const {
            return uuid::uuid_to_uint64(uuid);
        }

        virtual std::string display_name() const {
            return namespace_.empty() ? name : namespace_ + "::" + name;
        }

        virtual std::string file_name() const {
            return namespace_.empty() ? name : namespace_ + "/" + name + ".hpp";
        }

        virtual void generate() const = 0;

        virtual void editor() {
            ui::font::push(ui::font::type::code);
            ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_typename);
            ui::separator(display_name());
            ui::style::color::pop();
            ui::font::pop();
            ui::input::text("Name", name);
            ui::input::text("Namespace", namespace_);
            ui::input::multiline_text("Description", description);
        }

        virtual void viewer() const {
            ui::font::push(ui::font::type::code);
            ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_typename);
            ui::separator(display_name());
            ui::style::color::pop();
            ui::font::pop();
            ui::text(description);
        }

    protected:
        record(const char* object_namespace, const char* object_name = "", const char* object_description = "") {
            this->uuid = uuid::v4::generate();
            this->name = object_name;
            this->namespace_ = object_namespace;
            this->description = object_description;
        }

        record(const char* object_uuid, const char* object_name, const char * object_namespace, const char* object_description) {
            this->uuid = object_uuid;
            this->name = object_name;
            this->namespace_ = object_namespace;
            this->description = object_description;
        }

        record(const char* object_name, const json& j) : record(object_name) {
            uuid = j.value("uuid", uuid);
            name = j.value("name", name);
            namespace_ = j.value("namespace", namespace_);
            description = j.value("description", description);
        }
    };

} // namespace database
