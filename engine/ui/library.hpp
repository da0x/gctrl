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
#include "ui/navigation.hpp"
#include "controls/machine/object.hpp"
#include "controls/controller/object.hpp"
#include "controls/driver/object.hpp"
#include "controls/element/object.hpp"
#include "controls/function/object.hpp"
#include "controls/port/object.hpp"

namespace ui {
namespace library {

    // Callback to clear focus path when library item is selected
    inline std::function<void()> on_item_selected;

    template<typename T>
    void render_category(const char* icon, const char* label, const char* add_label,
                         std::list<T>& records, ui::navigation::type type) {
        auto current_entry = ui::navigation::current_entry();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        bool is_category_selected = current_entry.type == static_cast<int>(type) && current_entry.index == -1;
        if (is_category_selected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        bool open = ImGui::TreeNodeEx(label, flags);

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            ui::navigation::push(ui::navigation::entry{ static_cast<int>(type), -1, current_entry.mode });
        }

        // Context menu for adding new items
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem(add_label)) {
                records.emplace_back();
                ui::navigation::push(ui::navigation::entry{
                    static_cast<int>(type),
                    static_cast<int>(records.size() - 1),
                    current_entry.mode
                });
            }
            ImGui::EndPopup();
        }

        if (open) {
            int index = 0;
            for (auto& record : records) {
                ImGui::PushID(record.uuid.c_str());

                ImGuiTreeNodeFlags item_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                bool is_selected = current_entry.type == static_cast<int>(type) && current_entry.index == index;
                if (is_selected) {
                    item_flags |= ImGuiTreeNodeFlags_Selected;
                }

                ImGui::TreeNodeEx(record.uuid.c_str(), item_flags, "%s %s", icon, record.display_name().c_str());

                if (ImGui::IsItemClicked()) {
                    ui::navigation::push(ui::navigation::entry{ static_cast<int>(type), index, current_entry.mode });
                    if (on_item_selected) on_item_selected();
                }

                // Context menu for deleting
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem(ICON_FA_TRASH " Delete")) {
                        auto it = records.begin();
                        std::advance(it, index);
                        records.erase(it);
                        ui::navigation::delete_entry(static_cast<int>(type), index, current_entry.mode);
                        if (is_selected) {
                            ui::navigation::push(ui::navigation::entry{ static_cast<int>(type), -1, current_entry.mode });
                        }
                    }
                    ImGui::EndPopup();
                }

                ImGui::PopID();
                ++index;
            }
            ImGui::TreePop();
        }
    }

    // Track expand/collapse request
    inline int tree_open_action = 0; // 0=none, 1=expand, -1=collapse

    inline void render(
        machine::object::list& machines,
        controller::object::list& controllers,
        driver::object::list& drivers,
        element::object::list& elements,
        function::object::list& functions,
        port::object::list& ports
    ) {
        ui::begin("Library");

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
        render_category(ui::icon::machine, "Machines", "Add Machine", machines, ui::navigation::type::machine);

        apply_tree_action();
        render_category(ui::icon::controller, "Controllers", "Add Controller", controllers, ui::navigation::type::controller);

        apply_tree_action();
        render_category(ui::icon::driver, "Drivers", "Add Driver", drivers, ui::navigation::type::driver);

        apply_tree_action();
        render_category(ui::icon::element, "Elements", "Add Element", elements, ui::navigation::type::element);

        apply_tree_action();
        render_category(ui::icon::function, "Functions", "Add Function", functions, ui::navigation::type::function);

        apply_tree_action();
        render_category(ui::icon::plug, "Ports", "Add Port", ports, ui::navigation::type::port);

        // Reset action after processing all categories
        tree_open_action = 0;

        ui::end();
    }

} // namespace library
} // namespace ui
