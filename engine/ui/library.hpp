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

#include "ui/ui.hpp"
#include "ui/icons.hpp"
#include "ui/focus.hpp"
#include "ui/selection.hpp"
#include "controls/machine/object.hpp"
#include "controls/controller/object.hpp"
#include "controls/element/object.hpp"
#include "controls/function/object.hpp"
#include "controls/port/object.hpp"

namespace ui {
namespace library {

    // Current state pointers (set during render)
    inline focus::path* current_focus_path = nullptr;
    inline selection* current_selection = nullptr;

    // Render a library category with objects
    template<typename T>
    void render_category(const char* icon, const char* label, const char* add_label,
                         std::list<T>& records, focus::level level) {

        // Check if any item in this category is currently being edited
        bool category_has_focus = false;
        if (current_focus_path && !current_focus_path->empty()) {
            auto& current = current_focus_path->current();
            if (current.type == level) {
                for (const auto& record : records) {
                    if (record.uuid == current.uuid) {
                        category_has_focus = true;
                        break;
                    }
                }
            }
        }

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

        // Auto-expand if this category has the focused item
        if (category_has_focus) {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        }

        bool open = ImGui::TreeNodeEx(label, flags);

        // Context menu for adding new items
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem(add_label)) {
                records.emplace_back();
                auto& new_record = records.back();
                // Navigate to the new object
                if (current_focus_path) {
                    current_focus_path->clear();
                    current_focus_path->push({level, new_record.uuid, new_record.display_name()});
                }
                if (current_selection) {
                    current_selection->clear();
                }
            }
            ImGui::EndPopup();
        }

        if (open) {
            for (auto& record : records) {
                ImGui::PushID(record.uuid.c_str());

                // Check if this specific item is being edited
                bool is_selected = false;
                if (current_focus_path && !current_focus_path->empty()) {
                    auto& current = current_focus_path->current();
                    is_selected = (current.type == level && current.uuid == record.uuid);
                }

                ImGuiTreeNodeFlags item_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
                if (is_selected) {
                    item_flags |= ImGuiTreeNodeFlags_Selected;
                }

                ImGui::TreeNodeEx(record.uuid.c_str(), item_flags, "%s %s", icon, record.display_name().c_str());

                // Single click navigates to this object's editor
                if (ImGui::IsItemClicked()) {
                    if (current_focus_path) {
                        current_focus_path->clear();
                        current_focus_path->push({level, record.uuid, record.display_name()});
                    }
                    // Clear hierarchy selection - we're now editing a library object directly
                    if (current_selection) {
                        current_selection->clear();
                    }
                }

                // Context menu for deleting
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem(ICON_FA_TRASH " Delete")) {
                        bool was_selected = is_selected;
                        auto it = std::find_if(records.begin(), records.end(),
                            [&](const T& r) { return r.uuid == record.uuid; });
                        if (it != records.end()) {
                            records.erase(it);
                        }
                        if (was_selected && current_focus_path) {
                            current_focus_path->clear();
                        }
                    }
                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }
            ImGui::TreePop();
        }
    }

    // Track expand/collapse request
    inline int tree_open_action = 0; // 0=none, 1=expand, -1=collapse

    inline void render(
        machine::object::list& machines,
        controller::object::list& controllers,
        element::object::list& elements,
        function::object::list& functions,
        port::object::list& ports,
        focus::path& focus_path,
        selection& sel
    ) {
        current_focus_path = &focus_path;
        current_selection = &sel;

        ui::begin("Library");

        // Push prominent selection/hover colors (same as hierarchy)
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.29f, 0.59f, 0.82f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.29f, 0.59f, 0.82f, 0.50f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.29f, 0.59f, 0.82f, 1.00f));

        // Expand/Collapse buttons
        if (ImGui::Button(ICON_FA_EXPAND " Expand")) {
            tree_open_action = 1;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_COMPRESS " Collapse")) {
            tree_open_action = -1;
        }
        ImGui::Separator();

        // Apply expand/collapse before each category
        auto apply_tree_action = []() {
            if (tree_open_action != 0) {
                ImGui::SetNextItemOpen(tree_open_action > 0, ImGuiCond_Always);
            }
        };

        apply_tree_action();
        render_category(ui::icon::machine, "Machines", "Add Machine", machines, focus::level::machine);

        apply_tree_action();
        render_category(ui::icon::controller, "Controllers", "Add Controller", controllers, focus::level::controller);

        apply_tree_action();
        render_category(ui::icon::element, "Elements", "Add Element", elements, focus::level::element);

        apply_tree_action();
        render_category(ui::icon::function, "Functions", "Add Function", functions, focus::level::function);

        apply_tree_action();
        render_category(ui::icon::plug, "Ports", "Add Port", ports, focus::level::plug);

        // Reset action after processing all categories
        tree_open_action = 0;

        ImGui::PopStyleColor(3);

        ui::end();

        current_focus_path = nullptr;
        current_selection = nullptr;
    }

} // namespace library
} // namespace ui
