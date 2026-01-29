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

#include <filesystem>
#include <list>
#include <nlohmann/json.hpp>
#include "controls/machine/instance.hpp"
#include "controls/machine/editor.hpp"
#include "controls/controller/object.hpp"
#include "controls/controller/editor.hpp"
#include "controls/driver/instance.hpp"
#include "controls/driver/editor.hpp"
#include "controls/function/editor.hpp"
#include "controls/element/editor.hpp"
#include "controls/port/editor.hpp"
#include "controls/debug/viewer.hpp"
#include "controls/build/build.hpp"
#include "controls/list.hpp"
#include "controls/network/object.hpp"
#include "controls/network/editor.hpp"
#include "controls/runtime/state.hpp"
#include "controls/runtime/build.hpp"
#include "ui/ui.hpp"
#include "ui/icons.hpp"
#include "ui/navigation.hpp"
#include "ui/focus.hpp"
#include "ui/hierarchy.hpp"
#include "ui/breadcrumb.hpp"
#include "ui/library.hpp"
#include <imgui.h>


namespace controls {

    enum class terminal_operation { none, build, rebuild, clean, run };

    class engine {
    public:
        std::filesystem::path engine_file;
        ui::navigation::mode current_mode;
        bool ui_settings_loaded = false;
        bool ne_settings_loaded = false;
        terminal_operation current_operation = terminal_operation::none;

        const signal::object::list signals;
        function::object::list functions;
        element::object::list elements;
        port::object::list ports;
        controller::object::list controllers;
        network::object network;
        runtime::tracker runtime;

        ui::focus::path focus_path;

        // Active machine for build/run operations (the machine currently being worked on)
        uint64_t active_machine_id = 0;

        engine(const std::filesystem::path& engine_path)
            : engine_file(engine_path),
            current_mode(ui::navigation::mode::edit),
            signals(signal::default_signals) {
            load_engine(engine_path);
            ui::navigation::load(engine_path);
            ui::cout << "Loaded engine from: " << engine_file << ui::endl;
        }

        ~engine() {
            ui::cout << "Saving engine to: " << engine_file << ui::endl;
            save_engine();
            ui::navigation::save(engine_file);
            if (terminal::is_running())
                terminal::cancel();
        }

        void set_mode(ui::navigation::mode new_mode) {
            if (current_mode != new_mode) {
                current_mode = new_mode;
                ui::navigation::push({ -1, -1, static_cast<int>(current_mode) });
                reload_view();
            }
        }

        void save_engine() {
            nlohmann::json engine_data;
            engine_data["functions"] = serialize(functions);
            engine_data["elements"] = serialize(elements);
            engine_data["ports"] = serialize(ports);
            engine_data["controllers"] = serialize(controllers);
            engine_data["network"] = network.serialize();
            engine_data["ui_settings"] = ui::settings();
            engine_data["ne_settings"] = ui::node::settings();
            engine_data["focus_path"] = focus_path.serialize();

            std::ofstream out_file(engine_file);
            out_file << engine_data.dump(4);
            ui::cout << "Engine saved: " << engine_file << ui::endl;
        }

        void load_engine(const std::filesystem::path& path) {
            std::ifstream in_file(path);
            if (in_file.is_open()) {
                if (in_file.peek() == std::ifstream::traits_type::eof()) {
                    ui::cout << "Loaded empty engine file: " << path << ui::endl;
                    return;
                }

                nlohmann::json engine_data;
                in_file >> engine_data;

                functions = deserialize<function::object>(engine_data["functions"], signals);
                elements = deserialize<element::object>(engine_data["elements"], functions, signals);
                ports = deserialize<port::object>(engine_data["ports"], signals);
                controllers = deserialize<controller::object>(engine_data["controllers"], elements, ports);

                if (engine_data.contains("network")) {
                    network = network::object(engine_data["network"], controllers, ports);
                }

                if (engine_data.contains("ui_settings") && !ui_settings_loaded) {
                    ui::settings(engine_data["ui_settings"]);
                    ui_settings_loaded = true;
                }
                if (engine_data.contains("ne_settings") && !ne_settings_loaded) {
                    ui::node::settings(engine_data["ne_settings"]);
                    ne_settings_loaded = true;
                }
                if (engine_data.contains("focus_path")) {
                    focus_path.deserialize(engine_data["focus_path"]);
                }
            }
            else {
                ui::cout << "Failed to open engine file: " << path << ui::endl;
            }
        }

        void reload_view() {
            save_engine();
            load_engine(engine_file);
            ui::navigation::save(engine_file);
        }

        void navigate_to(const ui::navigation::type type, const int index) {
            ui::navigation::entry current_entry = ui::navigation::current_entry();
            if (current_entry.type != static_cast<int>(type) || current_entry.index != index || current_entry.mode != static_cast<int>(current_mode)) {
                ui::navigation::push({ static_cast<int>(type), index, static_cast<int>(current_mode) });
                reload_view();
            }
        }

        void navigate_to_most_recent(ui::navigation::type type) {
            auto entry = ui::navigation::most_recent_for_type(static_cast<int>(type));
            entry.mode = static_cast<int>(current_mode);
            ui::navigation::push(entry);
            reload_view();
        }

        void navigate_to_most_recent(ui::navigation::mode mode) {
            set_mode(mode);
            auto entry = ui::navigation::most_recent_for_mode(static_cast<int>(mode));
            entry.mode = static_cast<int>(current_mode);
            ui::navigation::push(entry);
            reload_view();
        }

        bool navigate_back() {
            ui::navigation::entry entry;
            if (ui::navigation::back(entry)) {
                current_mode = static_cast<ui::navigation::mode>(entry.mode);
                reload_view();
                return true;
            }
            return false;
        }

        bool navigate_forward() {
            ui::navigation::entry entry;
            if (ui::navigation::forward(entry)) {
                current_mode = static_cast<ui::navigation::mode>(entry.mode);
                reload_view();
                return true;
            }
            return false;
        }

        void focus_on(const ui::focus::focus_entry& entry) {
            focus_path.push(entry);
        }

        void focus_up() {
            if (!focus_path.empty()) {
                focus_path.pop();
            }
        }

        void focus_to_depth(size_t depth) {
            focus_path.pop_to_depth(depth);
        }

        void handle_keyboard_navigation() {
            if (ImGui::IsKeyPressed(ImGuiKey_Escape) && !focus_path.empty()) {
                focus_up();
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && !focus_path.empty() && !ImGui::GetIO().WantTextInput) {
                focus_up();
            }
        }

        void render() {
            handle_keyboard_navigation();

            // Set up drill-down callback for network editor (machines can be drilled into)
            network::on_drill_down = [this](const ui::focus::focus_entry& entry) {
                focus_on(entry);
                // Track active machine for build operations
                if (entry.type == ui::focus::level::machine) {
                    for (const auto& mach : network.machines) {
                        if (mach.uuid == entry.uuid) {
                            active_machine_id = mach.id();
                            break;
                        }
                    }
                }
            };

            // Set up drill-down callback for machine editor (controllers can be drilled into)
            machine::on_drill_down = [this](const ui::focus::focus_entry& entry) {
                focus_on(entry);
            };

            // Set up drill-down callback for controller editor (elements can be drilled into)
            controller::on_drill_down = [this](const ui::focus::focus_entry& entry) {
                focus_on(entry);
            };

            // Update runtime state for all machines that might be building/running
            for (auto& machine : network.machines) {
                runtime::update_machine_state(machine.id(), runtime);
            }

            render_menu();

            // Unified mode - always render edit mode, it now includes runtime overlays
            render_edit_mode();

            // Render debug viewer when any machine is running
            if (runtime.is_any_running()) {
                render_debug_viewer();
            }

            ui::terminal::render();
        }

        void render_debug_viewer() {
            auto& state = debug::get_state();
            std::lock_guard<std::mutex> lock(state.data_mutex);
            debug::viewer::render(network.machines, state.signal_data);
        }

    private:
        template<typename T>
        std::list<T> deserialize(const nlohmann::json& data) {
            std::list<T> result;
            for (const auto& item : data) {
                result.emplace_back(item);
            }
            return result;
        }

        template<typename T, typename A>
        std::list<T> deserialize(const nlohmann::json& data, const std::list<A>& a) {
            std::list<T> result;
            for (const auto& item : data) {
                result.emplace_back(item, a);
            }
            return result;
        }

        template<typename T, typename A, typename B>
        std::list<T> deserialize(const nlohmann::json& data, const std::list<A>& a, const std::list<B>& b) {
            std::list<T> result;
            for (const auto& item : data) {
                result.emplace_back(item, a, b);
            }
            return result;
        }

        template<typename T>
        nlohmann::json serialize(const std::list<T>& data) {
            nlohmann::json result;
            for (const auto& item : data) {
                result.push_back(item.serialize());
            }
            return result;
        }

        void render_hierarchy_panel() {
            ui::hierarchy::callbacks cbs;
            cbs.on_select = [this](const ui::focus::focus_entry& entry) {
                // Selection updates property panels (handled by existing code)
            };
            cbs.on_drill_down = [this](const ui::focus::focus_entry& entry) {
                focus_on(entry);
                // Track active machine for build/run operations
                if (entry.type == ui::focus::level::machine) {
                    for (const auto& mach : network.machines) {
                        if (mach.uuid == entry.uuid) {
                            active_machine_id = mach.id();
                            break;
                        }
                    }
                }
            };
            ui::hierarchy::render(network.machines, runtime, cbs);
        }

        void render_breadcrumb() {
            if (!focus_path.empty()) {
                int clicked = ui::breadcrumb::render(focus_path);
                if (clicked > 0) {
                    focus_to_depth(static_cast<size_t>(clicked));
                }
            }
        }

        template<typename T>
        T* find_by_uuid(std::list<T>& collection, const std::string& uuid) {
            for (auto& item : collection) {
                if (item.uuid == uuid) return &item;
            }
            return nullptr;
        }

        driver::instance* find_driver_by_uuid(const std::string& uuid) {
            for (auto& mach : network.machines) {
                for (auto& drv : mach.drivers) {
                    if (drv.uuid == uuid) return &drv;
                }
            }
            return nullptr;
        }

        machine::object* find_machine_by_uuid(const std::string& uuid) {
            for (auto& mach : network.machines) {
                if (mach.uuid == uuid) return &mach;
            }
            return nullptr;
        }

        machine::object* find_machine_by_id(uint64_t id) {
            for (auto& mach : network.machines) {
                if (mach.id() == id) return &mach;
            }
            return nullptr;
        }

        machine::object* get_active_machine() {
            if (active_machine_id == 0 && !network.machines.empty()) {
                active_machine_id = network.machines.front().id();
            }
            return find_machine_by_id(active_machine_id);
        }

        void render_focused_content() {
            // Network view when focus path is empty
            if (focus_path.empty()) {
                network::render_editor(network, runtime);
                return;
            }

            auto& current = focus_path.current();
            switch (current.type) {
                case ui::focus::level::network:
                    network::render_editor(network, runtime);
                    break;
                case ui::focus::level::machine: {
                    auto* mach = find_machine_by_uuid(current.uuid);
                    if (mach) {
                        machine::render_editor(*mach, controllers, elements, ports, functions, runtime);
                    }
                    break;
                }
                case ui::focus::level::controller: {
                    auto* ctrl = find_by_uuid(controllers, current.uuid);
                    if (ctrl) {
                        controller::render_editor(*ctrl, elements, ports);
                    }
                    break;
                }
                case ui::focus::level::driver: {
                    auto* drv = find_driver_by_uuid(current.uuid);
                    if (drv) {
                        ui::begin("Driver Editor");
                        driver::render_editor(*drv, ports);
                        ui::end();
                    }
                    break;
                }
                case ui::focus::level::element: {
                    auto* elem = find_by_uuid(elements, current.uuid);
                    if (elem) {
                        element::render_editor(*elem, functions, signals);
                    }
                    break;
                }
                case ui::focus::level::plug:
                case ui::focus::level::socket: {
                    auto* p = find_by_uuid(ports, current.uuid);
                    if (p) {
                        port::render_editor(*p, signals);
                    }
                    break;
                }
            }
        }

        void render_edit_mode() {
            // Render unified hierarchy panel
            render_hierarchy_panel();

            // Set up library callback to clear focus path when item selected
            ui::library::on_item_selected = [this]() {
                focus_path.clear();
            };

            // Render library panel with all object definitions (drivers removed)
            ui::library::render(network.machines, controllers, elements, functions, ports);

            // Render breadcrumb navigation if we have a focus path
            if (!focus_path.empty()) {
                ui::begin("Editor");
                render_breadcrumb();
                ui::separator();
                ui::end();
            }

            // Render focused content (network view when empty, specific view when focused)
            render_focused_content();

            ui::chart::show_my_window();
        }

        void render_menu() {
            if (ui::main_menu::begin()) {
                ImGui::BeginDisabled(!ui::navigation::can_go_back());
                if (ImGui::Button(ICON_FA_CIRCLE_LEFT " Back")) {
                    navigate_back();
                }
                ImGui::EndDisabled();

                ImGui::SameLine();

                ImGui::BeginDisabled(!ui::navigation::can_go_forward());
                if (ImGui::Button(ICON_FA_CIRCLE_RIGHT " Forward")) {
                    navigate_forward();
                }
                ImGui::EndDisabled();

                if (ui::menu::begin("  File  ")) {
                    if (ui::menu::item("Save Engine", "Ctrl+S")) { save_engine(); }
                    ImGui::Separator();
                    if (ui::menu::item("Exit", "Alt+F4")) { ui::request_exit(); }
                    ui::menu::end();
                }

                if (ui::menu::begin("  Build  ")) {
                    build::create_build_directory();

                    bool is_building = terminal::is_running();

                    if (is_building) {
                        ImGui::BeginDisabled(true);
                    }

                    if (ui::menu::item(string(ui::icon::build) + "\tBuild", "F7", !is_building && current_mode == ui::navigation::mode::edit)) {
                        current_operation = terminal_operation::build;
                        if (!std::filesystem::exists("gctrl")) {
                            ui::cout << "Generating code..." << ui::endl;
                            for (const auto& machine : network.machines) {
                                machine.generate();
                            }
                            ui::good << "Code generation complete." << ui::endl;
                        }
                        terminal::execute({
                            "cmake -S gctrl -B build",
                            "cmake --build build"
                            }
                        );
                    }

                    if (ui::menu::item(string(ui::icon::rebuild) + "\tRebuild", "Shift+F7", !is_building && current_mode == ui::navigation::mode::edit)) {
                        current_operation = terminal_operation::rebuild;
                        ui::cout << "Generating code..." << ui::endl;
                        code::delete_gctrl_directory();
                        for (const auto& machine : network.machines) {
                            machine.generate();
                        }
                        ui::good << "Code generation complete." << ui::endl;
                        terminal::execute({
                            "rm -rf build",
                            "cmake -S gctrl -B build",
                            "cmake --build build"
                            }
                        );
                    }

                    if (ui::menu::item(string(ui::icon::trash) + "\tClean", "Ctrl+Shift+F7", !is_building && current_mode == ui::navigation::mode::edit)) {
                        current_operation = terminal_operation::clean;
                        terminal::execute({
                            "rm -rf build"
                            }
                        );
                    }

                    if (is_building) {
                        ImGui::EndDisabled();
                    }

                    ImGui::BeginDisabled(!is_building);
                    if (ui::menu::item(string(ui::icon::stop) + "\tCancel Build", "Esc", true)) {
                        terminal::cancel();
                    }
                    ImGui::EndDisabled();

                    ui::menu::end();
                }

                if (ui::menu::begin("  Debug  ")) {
                    ImGui::BeginDisabled(terminal::is_running());
                    if (ui::menu::item(string(ui::icon::play) + "\tStart Debugging", "F5", current_mode == ui::navigation::mode::edit)) {
                        auto* mach = get_active_machine();
                        if (mach) {
                            current_operation = terminal_operation::run;
                            runtime::run_machine(*mach, runtime);
                        }
                        navigate_to_most_recent(ui::navigation::mode::run);
                    }
                    ImGui::EndDisabled();

                    if (ui::menu::item(string(ui::icon::stop) + "\tStop Debugging", "Shift+F5", terminal::is_running() || current_mode == ui::navigation::mode::run)) {
                        auto* mach = get_active_machine();
                        if (mach && terminal::is_running()) {
                            runtime::stop_machine(*mach, runtime);
                        }
                        navigate_to_most_recent(ui::navigation::mode::edit);
                    }
                    ui::menu::end();
                }

                ui::main_menu::end();
            }

            // Check terminal completion OUTSIDE menu block so it runs every frame
            if (terminal::finished() && current_operation != terminal_operation::none) {
                const char* op_name = "Operation";
                switch (current_operation) {
                    case terminal_operation::build: op_name = "Build"; break;
                    case terminal_operation::rebuild: op_name = "Rebuild"; break;
                    case terminal_operation::clean: op_name = "Clean"; break;
                    case terminal_operation::run: op_name = "Run"; break;
                    default: break;
                }
                if (terminal::success()) {
                    ui::good << op_name << " succeeded!" << ui::endl;
                }
                else {
                    ui::cerr << op_name << " failed!" << ui::endl;
                }
                current_operation = terminal_operation::none;
            }
        }
    };

} // namespace controls
