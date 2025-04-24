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

#include <nlohmann/json.hpp>
#include <da0x/uuid.hpp>
#include "record.hpp"

namespace database {
    using std::string;
    namespace ed = ax::NodeEditor;

    class instance {
    public:
        using list = std::list<instance>;

        string uuid;
        string name;
        string description;
        string prototype_uuid;
        ui::direction direction = ui::direction::ltr;
    protected:
        const record& prototype;
    public:
        virtual ~instance() = default;

        virtual string icon() const = 0;

        virtual json serialize() const {
            nlohmann::json j;
            j["uuid"] = uuid;
            j["name"] = name;
            j["description"] = description;
            j["prototype_uuid"] = prototype_uuid;
            j["direction"] = (direction == ui::direction::ltr) ? "ltr" : "rtl";
            return j;
        }

        virtual uint64_t id() const {
            return uuid::uuid_to_uint64(uuid);
        }
        virtual string instance_name() const {
            return name;
        }
        virtual string prototype_typename() const {
            return prototype.display_name();
        }

        virtual string prototype_filename() const {
            return prototype.file_name();
        }

        uint64_t prototype_id() const {
            return prototype.id();
        }

        virtual void generate() const {
            prototype.generate();
        }

        virtual void make_unique() {
            uuid = uuid::v4::generate();
        }

        instance(const record& type): prototype(type) {
            uuid = uuid::v4::generate();
            name = "";
            description = type.description;
            prototype_uuid = type.uuid;
            direction = ui::direction::ltr;
        }

        instance(const json& j, const record& type): prototype(type) {
            uuid = j.value("uuid", uuid);
            name = j.value("name", name);
            description = j.value("description", description);
            prototype_uuid = j.value("prototype_uuid", type.uuid);
            string dir = j.value("direction", "ltr");
            direction = (dir == "rtl") ? ui::direction::rtl : ui::direction::ltr;
        }

        virtual void editor() {
            ui::font::push(ui::font::type::code);
            ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_instance);
            ui::separator(instance_name());
            ui::style::color::pop();
            ui::font::pop();
            ui::input::text("Name", name);
            ui::input::multiline_text("Description", description);
            prototype.viewer();
        }
    };

    template <typename InstanceType, typename PrototypeType>
    void load_instances(const json& json_data, const string& key, std::list<InstanceType>& instances, const std::list<PrototypeType>& prototypes) {
        if (json_data.contains(key)) {
            for (const auto& item_json : json_data[key]) {
                string name = item_json["name"];
                string prototype_uuid = item_json.value("prototype_uuid", "");
                bool found = false;
                for (const auto& prototype : prototypes) {
                    if (prototype.uuid == prototype_uuid) {
                        instances.push_back({ item_json, prototype });
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    ui::cerr << "cannot find prototype: " << prototype_uuid << ":" << item_json.dump() << ui::endl;
                }
            }
        }
    }


} // namespace database
