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

#include "platform/process.hpp"
#include "ui/terminal.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <thread>
#include <cstring>
#include <cerrno>

namespace platform {
namespace process {

    static state global_state;
    static pid_t current_pid = -1;

    state& get_state() {
        return global_state;
    }

    void log_last_error(const std::string& context) {
        ui::cerr << context << ". Error: " << strerror(errno) << ui::endl;
    }

    static bool run_single_command(const std::string& command) {
        int stdout_pipe[2], stderr_pipe[2];

        if (pipe(stdout_pipe) < 0 || pipe(stderr_pipe) < 0) {
            log_last_error("Failed to create pipes");
            return false;
        }

        pid_t pid = fork();
        if (pid < 0) {
            log_last_error("Failed to fork process");
            close(stdout_pipe[0]); close(stdout_pipe[1]);
            close(stderr_pipe[0]); close(stderr_pipe[1]);
            return false;
        }

        if (pid == 0) {
            // Child process
            close(stdout_pipe[0]);
            close(stderr_pipe[0]);
            dup2(stdout_pipe[1], STDOUT_FILENO);
            dup2(stderr_pipe[1], STDERR_FILENO);
            close(stdout_pipe[1]);
            close(stderr_pipe[1]);

            execl("/bin/sh", "sh", "-c", command.c_str(), nullptr);
            _exit(127);
        }

        // Parent process
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        {
            std::lock_guard<std::mutex> lock(global_state.cmd_mutex);
            current_pid = pid;
        }

        auto read_pipe = [](int fd, ui::terminal_stream& stream) {
            char buffer[128];
            ssize_t bytes_read;
            while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytes_read] = '\0';
                stream << buffer;
                stream.flush();
            }
        };

        std::thread stdout_thread(read_pipe, stdout_pipe[0], std::ref(ui::cout));
        std::thread stderr_thread(read_pipe, stderr_pipe[0], std::ref(ui::cerr));

        stdout_thread.join();
        stderr_thread.join();

        close(stdout_pipe[0]);
        close(stderr_pipe[0]);

        int status;
        waitpid(pid, &status, 0);

        {
            std::lock_guard<std::mutex> lock(global_state.cmd_mutex);
            current_pid = -1;
        }

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code != 0) {
                ui::cerr << "Command failed with exit code: " << exit_code << ui::endl;
                return false;
            }
        } else {
            ui::cerr << "Command terminated abnormally" << ui::endl;
            return false;
        }

        return true;
    }

    void process_next_command() {
        global_state.finished_flag = false;
        global_state.running = true;
        global_state.success_flag = true;

        global_state.future = std::async(std::launch::async, []() -> bool {
            while (!global_state.cancel_flag && !global_state.command_queue.empty()) {
                std::string command;
                {
                    std::lock_guard<std::mutex> lock(global_state.cmd_mutex);
                    if (global_state.command_queue.empty()) break;
                    command = global_state.command_queue.front();
                    global_state.command_queue.pop();
                }

                ui::good << "$ " << command << ui::endl;

                if (!run_single_command(command)) {
                    global_state.success_flag = false;
                }
            }

            global_state.running = false;
            global_state.finished_flag = true;
            return global_state.success_flag;
        });
    }

    void execute(const std::vector<std::string>& commands) {
        std::lock_guard<std::mutex> lock(global_state.cmd_mutex);
        if (global_state.running) {
            throw std::runtime_error("Cannot schedule commands while another group is running.");
        }

        global_state.command_queue = std::queue<std::string>(std::deque<std::string>(commands.begin(), commands.end()));
        global_state.cancel_flag = false;
        global_state.success_flag = true;
        global_state.finished_flag = false;
        process_next_command();
    }

    void cancel() {
        std::lock_guard<std::mutex> lock(global_state.cmd_mutex);
        if (!global_state.running) {
            throw std::runtime_error("Cannot cancel a command that is not running.");
        }
        global_state.cancel_flag = true;
        if (current_pid > 0) {
            kill(current_pid, SIGTERM);
        }
    }

    bool is_running() {
        return global_state.running;
    }

    bool success() {
        return global_state.success_flag;
    }

    bool finished() {
        if (global_state.finished_flag) {
            global_state.finished_flag = false;
            return true;
        }
        return false;
    }

} // namespace process
} // namespace platform
