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
#include "controls/driver/object.hpp"
#include "controls/driver/editor.hpp"
#include "controls/function/editor.hpp"
#include "controls/element/editor.hpp"
#include "controls/port/editor.hpp"
#include "controls/debug/viewer.hpp"
#include "controls/build/build.hpp"
#include "controls/list.hpp"
#include "ui/ui.hpp"
#include "ui/icons.hpp"
#include "ui/navigation.hpp"
#include <imgui.h>


namespace controls {

    class engine {
    public:
        std::filesystem::path engine_file;
        ui::navigation::mode current_mode;
        bool ui_settings_loaded = false;
        bool ne_settings_loaded = false;

        const signal::object::list signals;
        function::object::list functions;
        element::object::list elements;
        port::object::list ports;
        driver::object::list drivers;
        controller::object::list controllers;
        machine::object::list machines;

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
            engine_data["drivers"] = serialize(drivers);
            engine_data["controllers"] = serialize(controllers);
            engine_data["machines"] = serialize(machines);
            engine_data["ui_settings"] = ui::settings();
            engine_data["ne_settings"] = ui::node::settings();

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
                drivers = deserialize<driver::object>(engine_data["drivers"], ports);
                controllers = deserialize<controller::object>(engine_data["controllers"], elements, ports);
                machines = deserialize<machine::object>(engine_data["machines"], controllers, drivers);

                if (engine_data.contains("ui_settings") && !ui_settings_loaded) {
                    ui::settings(engine_data["ui_settings"]);
                    ui_settings_loaded = true;
                }
                if (engine_data.contains("ne_settings") && !ne_settings_loaded) {
                    ui::node::settings(engine_data["ne_settings"]);
                    ne_settings_loaded = true;
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

        void render() {
            render_menu();
            if (current_mode == ui::navigation::mode::edit) {
                render_edit_mode();
            }
            else if (current_mode == ui::navigation::mode::run) {
                render_run_mode();
            }

            ui::terminal::render();
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

        void list_library(
            machine::object::list& machine_db,
            controller::object::list& controller_db,
            driver::object::list& driver_db,
            function::object::list& function_db,
            element::object::list& element_db,
            port::object::list& port_db) {
            auto entry = ui::navigation::current_entry();
            switch (static_cast<ui::navigation::type>(entry.type)) {
            case ui::navigation::type::machine:
                render_list(ui::icon::machine, "Machines", "+ Add Machine", machine_db, ui::navigation::type::machine);
                break;
            case ui::navigation::type::controller:
                render_list(ui::icon::controller, "Controllers", "+ Add Controller", controller_db, ui::navigation::type::controller);
                break;
            case ui::navigation::type::driver:
                render_list(ui::icon::driver, "Drivers", "+ Add Driver", driver_db, ui::navigation::type::driver);
                break;
            case ui::navigation::type::function:
                render_list(ui::icon::function, "Functions", "+ Add Function", function_db, ui::navigation::type::function);
                break;
            case ui::navigation::type::element:
                render_list(ui::icon::element, "Elements", "+ Add Element", element_db, ui::navigation::type::element);
                break;
            case ui::navigation::type::port:
                render_list(ui::icon::plug, "Ports", "+ Add Port", port_db, ui::navigation::type::port);
                break;
            default:
                break;
            }
        }

        void render_edit_mode() {
            list_library(machines, controllers, drivers, functions, elements, ports);
            auto entry = ui::navigation::current_entry();
            if (entry.index >= 0) {
                switch (static_cast<ui::navigation::type>(entry.type)) {
                case ui::navigation::type::machine:
                    machine::render_editor(*std::next(machines.begin(), entry.index), drivers, controllers, elements, ports, functions);
                    break;
                case ui::navigation::type::controller:
                    controller::render_editor(*std::next(controllers.begin(), entry.index), elements, ports);
                    break;
                case ui::navigation::type::driver:
                    driver::render_editor(*std::next(drivers.begin(), entry.index), ports);
                    break;
                case ui::navigation::type::function:
                    function::render_editor(*std::next(functions.begin(), entry.index), functions, signals);
                    break;
                case ui::navigation::type::element:
                    element::render_editor(*std::next(elements.begin(), entry.index), functions, signals);
                    break;
                case ui::navigation::type::port:
                    port::render_editor(*std::next(ports.begin(), entry.index), signals);
                    break;
                }
            }

            ui::chart::show_my_window();
        }

        void render_run_mode() {

            std::lock_guard<std::mutex> lock(debug::state.data_mutex);
            debug::viewer::render(machines, debug::state.signal_data);
        }

        void view_menu(const std::string& label, const std::string& shortcut, ui::navigation::type navigation_type) {
            if (ui::menu::item(label, shortcut, current_mode == ui::navigation::mode::edit)) {
                navigate_to_most_recent(navigation_type);
            }
        }

        void render_menu() {
            if (ui::main::menu::begin()) {
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
                    ui::menu::end();
                }

                if (ui::menu::begin("  View  ")) {
                    view_menu(string(ui::icon::machine) + "\tMachines", "Ctrl+F1", ui::navigation::type::machine);
                    view_menu(string(ui::icon::controller) + "\tControllers", "Ctrl+F2", ui::navigation::type::controller);
                    view_menu(string(ui::icon::driver) + "\tDrivers", "Ctrl+F3", ui::navigation::type::driver);
                    view_menu(string(ui::icon::plug) + "\tPorts", "Ctrl+F4", ui::navigation::type::port);
                    view_menu(string(ui::icon::element) + "\tElements", "Ctrl+F5", ui::navigation::type::element);
                    view_menu(string(ui::icon::function) + "\tFunctions", "Ctrl+F6", ui::navigation::type::function);
                    ui::menu::end();
                }

                if (ui::menu::begin("  Build  ")) {
                    build::create_build_directory();

                    bool is_building = terminal::is_running();

                    if (is_building) {
                        ImGui::BeginDisabled(true);
                    }

                    if (ui::menu::item(string(ui::icon::build) + "\tBuild", "F7", !is_building && current_mode == ui::navigation::mode::edit)) {
                        terminal::execute({
                            "cmake -S . -B build",
                            "cmake --build build"
                            }
                        );
                    }

                    if (ui::menu::item(string(ui::icon::rebuild) + "\tRebuild", "Shift+F7", !is_building && current_mode == ui::navigation::mode::edit)) {
                        terminal::execute({
                            "cmake --build build --target clean",
                            "cmake -S . -B build",
                            "cmake --build build"
                            }
                        );
                    }

                    if (ui::menu::item(string(ui::icon::trash) + "\tClean", "Ctrl+Shift+F7", !is_building && current_mode == ui::navigation::mode::edit)) {
                        terminal::execute({
                            "cmake --build build --target clean"
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

                if (terminal::finished()) {
                    if (terminal::success()) {
                        ui::good << "Build succeeded!" << ui::endl;
                    }
                    else {
                        ui::cerr << "Build failed!" << ui::endl;
                    }
                }

                if (ui::menu::begin("  Debug  ")) {
                    ImGui::BeginDisabled(terminal::is_running());
                    if (ui::menu::item(string(ui::icon::play) + "\tStart Debugging", "F5", current_mode == ui::navigation::mode::edit)) {
                        terminal::execute({ std::filesystem::current_path().string() + "/build/bin/Debug/thermostat.exe" });
                        navigate_to_most_recent(ui::navigation::mode::run);
                    }
                    ImGui::EndDisabled();

                    if (ui::menu::item(string(ui::icon::stop) + "\tStop Debugging", "Shift+F5", terminal::is_running() || current_mode == ui::navigation::mode::run)) {
                        if (terminal::is_running()) {
                            terminal::cancel();
                        }
                        navigate_to_most_recent(ui::navigation::mode::edit);
                    }
                    ui::menu::end();
                }

                ui::main::menu::end();


            }
        }
    };

} // namespace controls
