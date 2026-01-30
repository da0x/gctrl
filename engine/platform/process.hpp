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
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>
#include <queue>
#include <future>

namespace platform {
namespace process {

    // Output stream callback type
    using stream_callback = std::function<void(const std::string&)>;

    // Process execution state
    struct state {
        std::atomic<bool> running{false};
        std::atomic<bool> cancel_flag{false};
        std::atomic<bool> success_flag{false};
        std::atomic<bool> finished_flag{false};
        std::future<bool> future;
        std::mutex cmd_mutex;
        std::queue<std::string> command_queue;
    };

    // Get the global process state
    state& get_state();

    // Log the last platform-specific error
    void log_last_error(const std::string& context);

    // Execute the next command in the queue
    void process_next_command();

    // Execute a list of commands sequentially
    void execute(const std::vector<std::string>& commands);

    // Cancel the currently running process
    void cancel();

    // Check if a process is currently running
    bool is_running();

    // Check if the last command succeeded
    bool success();

    // Check if the command queue finished processing
    bool finished();

    // Get the command currently being executed (empty if none)
    std::string current();

} // namespace process
} // namespace platform
