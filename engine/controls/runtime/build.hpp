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

#include "controls/runtime/state.hpp"
#include "controls/machine/object.hpp"
#include "controls/network/object.hpp"
#include "controls/code/generator.hpp"
#include "controls/debug/thread.hpp"
#include "platform/platform.hpp"
#include "platform/process.hpp"
#include "ui/terminal.hpp"
#include <filesystem>
#include <thread>
#include <functional>

namespace runtime {

    // Callback for when build completes
    using build_callback = std::function<void(uint64_t machine_id, bool success)>;
    inline build_callback on_build_complete;

    // Callback for when process finishes
    using run_callback = std::function<void(uint64_t machine_id, bool success)>;
    inline run_callback on_run_complete;

    // Per-machine process tracking
    struct machine_process {
        std::thread thread;
        std::atomic<bool> running{false};
        std::atomic<bool> cancel_requested{false};
#if defined(_WIN32) || defined(_WIN64)
        HANDLE process_handle = nullptr;
#else
        pid_t process_id = 0;
#endif
    };

    inline std::map<uint64_t, machine_process> machine_processes;

    inline std::string get_executable_path(const machine::object& machine) {
#if GCTRL_PLATFORM_WINDOWS
        return std::filesystem::current_path().string() + "/build/bin/Debug/" + machine.name + ".exe";
#else
        return std::filesystem::current_path().string() + "/build/bin/" + machine.name;
#endif
    }

    // Configure debug communication based on machine's UDP drivers
    inline void configure_debug_for_machine(const machine::object& machine) {
        auto endpoints = network::object::get_udp_endpoints(machine);
        if (!endpoints.empty()) {
            // Use first UDP driver's settings
            const auto& ep = endpoints[0];

            // GCtrl listens where machine sends
            controls::debug::init(ep.listen_ip, ep.listen_port, true, machine.name);

            // GCtrl sends where machine listens
            debug::configure_send(ep.send_ip, static_cast<uint16_t>(ep.send_port));

            ui::cout << "Debug configured for " << machine.name << ": listen on " << ep.listen_ip << ":" << ep.listen_port
                     << ", send to " << ep.send_ip << ":" << ep.send_port << ui::endl;
        }
    }

    inline void build_machine(machine::object& machine, tracker& runtime) {
        uint64_t machine_id = machine.id();

        if (runtime.is_building(machine_id) || runtime.is_running(machine_id)) {
            ui::cerr << "Machine " << machine.name << " is already building or running" << ui::endl;
            return;
        }

        runtime.set_status(machine_id, status::building);
        ui::cout << "Building machine: " << machine.name << ui::endl;

        // Always regenerate code before building
        ui::cout << "Generating code..." << ui::endl;
        machine.generate();
        ui::good << "Code generation complete." << ui::endl;

        // Use terminal to build (this is synchronous with the global process queue)
        terminal::execute({
            "cmake -S gctrl -B build",
            "cmake --build build"
        });
    }

    inline void run_machine(machine::object& machine, tracker& runtime) {
        uint64_t machine_id = machine.id();

        if (runtime.is_running(machine_id)) {
            ui::cerr << "Machine " << machine.name << " is already running" << ui::endl;
            return;
        }

        std::string exe_path = get_executable_path(machine);

        if (!std::filesystem::exists(exe_path)) {
            runtime.set_status(machine_id, status::error, "Executable not found: " + exe_path);
            ui::cerr << "Executable not found: " << exe_path << ui::endl;
            return;
        }

        // Configure debug communication based on machine's UDP drivers
        configure_debug_for_machine(machine);

        runtime.set_status(machine_id, status::running);
        ui::cout << "Running machine: " << machine.name << ui::endl;

        // Execute the machine process
        terminal::execute({ exe_path });
    }

    inline void stop_machine(machine::object& machine, tracker& runtime) {
        uint64_t machine_id = machine.id();

        if (!runtime.is_running(machine_id) && !runtime.is_building(machine_id)) {
            return;
        }

        ui::cout << "Stopping machine: " << machine.name << ui::endl;
        terminal::cancel();
        runtime.set_status(machine_id, status::stopped);
    }

    inline void build_and_run_machine(machine::object& machine, tracker& runtime) {
        uint64_t machine_id = machine.id();

        if (runtime.is_building(machine_id) || runtime.is_running(machine_id)) {
            ui::cerr << "Machine " << machine.name << " is already building or running" << ui::endl;
            return;
        }

        runtime.set_status(machine_id, status::building);
        ui::cout << "Building machine: " << machine.name << ui::endl;

        // Always regenerate code before building
        ui::cout << "Generating code..." << ui::endl;
        machine.generate();
        ui::good << "Code generation complete." << ui::endl;

        std::string exe_path = get_executable_path(machine);

        // Build and run in sequence
        terminal::execute({
            "cmake -S gctrl -B build",
            "cmake --build build",
            exe_path
        });
    }

    // Update runtime state based on terminal status - call this from engine::render()
    inline void update_machine_state(uint64_t machine_id, tracker& runtime) {
        auto& state = runtime.get_state(machine_id);

        if (state.current_status == status::building || state.current_status == status::running) {
            if (terminal::finished()) {
                if (terminal::success()) {
                    if (state.current_status == status::building) {
                        ui::good << "Build succeeded!" << ui::endl;
                    }
                    runtime.set_status(machine_id, status::stopped);
                } else {
                    runtime.set_status(machine_id, status::error, "Process failed");
                    ui::cerr << "Process failed!" << ui::endl;
                }
            }
        }
    }

} // namespace runtime
