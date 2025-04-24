#pragma once

#include <nlohmann/json.hpp>
#include <da0x/uuid.hpp>  // Assuming this is the correct path for uuid::v4::generate()
#include <iostream>
#include <string>

namespace da0x {

    // Function to refresh UUIDs in a nlohmann::json object
    void refresh_uuids(nlohmann::json& json_obj) {
        for (auto& el : json_obj.items()) {
            if (el.key() == "uuid" && el.value().is_string()) {
                el.value() = uuid::v4::generate();
            } else if (el.value().is_object()) {
                refresh_uuids(el.value());
            } else if (el.value().is_array()) {
                for (auto& arr_item : el.value()) {
                    if (arr_item.is_object()) {
                        refresh_uuids(arr_item);
                    }
                }
            }
        }
    }

    // Template class that subclasses nlohmann::json (you mentioned da0x::json earlier)
    template<typename T>
        class json : public nlohmann::json {
            public:
                using nlohmann::json::json;
                void refresh_uuids_in_object() {
                    refresh_uuids(*this);
                }
        };

} // namespace da0x

