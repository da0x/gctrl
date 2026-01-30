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

#include <map>
#include <string>
#include <optional>
#include <cstdint>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    using process_id_t = DWORD;
#else
    #include <sys/types.h>
    using process_id_t = pid_t;
#endif

namespace runtime {

    enum class status {
        stopped,    // Not running
        building,   // Compilation in progress
        running,    // Process active
        error       // Build or runtime error
    };

    inline const char* status_to_string(status s) {
        switch (s) {
            case status::stopped:  return "Stopped";
            case status::building: return "Building";
            case status::running:  return "Running";
            case status::error:    return "Error";
            default:               return "Unknown";
        }
    }

    struct machine_state {
        status current_status = status::stopped;
        std::string error_message;
        std::optional<process_id_t> process_id;

        bool is_running() const {
            return current_status == status::running;
        }

        bool is_building() const {
            return current_status == status::building;
        }

        bool is_stopped() const {
            return current_status == status::stopped;
        }

        bool has_error() const {
            return current_status == status::error;
        }
    };

    class tracker {
    public:
        std::map<uint64_t, machine_state> machines;

        bool is_running(uint64_t machine_id) const {
            auto it = machines.find(machine_id);
            if (it == machines.end()) return false;
            return it->second.is_running();
        }

        bool is_building(uint64_t machine_id) const {
            auto it = machines.find(machine_id);
            if (it == machines.end()) return false;
            return it->second.is_building();
        }

        bool is_any_running() const {
            for (const auto& [id, state] : machines) {
                if (state.is_running()) return true;
            }
            return false;
        }

        bool is_any_building() const {
            for (const auto& [id, state] : machines) {
                if (state.is_building()) return true;
            }
            return false;
        }

        status get_status(uint64_t machine_id) const {
            auto it = machines.find(machine_id);
            if (it == machines.end()) return status::stopped;
            return it->second.current_status;
        }

        void set_status(uint64_t machine_id, status s, const std::string& error = "") {
            machines[machine_id].current_status = s;
            if (s == status::error) {
                machines[machine_id].error_message = error;
            } else {
                machines[machine_id].error_message.clear();
            }
        }

        void set_process_id(uint64_t machine_id, process_id_t pid) {
            machines[machine_id].process_id = pid;
        }

        void clear_process_id(uint64_t machine_id) {
            machines[machine_id].process_id = std::nullopt;
        }

        std::optional<process_id_t> get_process_id(uint64_t machine_id) const {
            auto it = machines.find(machine_id);
            if (it == machines.end()) return std::nullopt;
            return it->second.process_id;
        }

        const machine_state& get_state(uint64_t machine_id) {
            return machines[machine_id];
        }

        std::string get_error(uint64_t machine_id) const {
            auto it = machines.find(machine_id);
            if (it == machines.end()) return "";
            return it->second.error_message;
        }
    };

} // namespace runtime
