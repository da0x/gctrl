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
#include "ui/ui.hpp"
#include <windows.h>
#include <thread>

namespace platform {
namespace process {

    static state global_state;
    static HANDLE process_handle = nullptr;

    state& get_state() {
        return global_state;
    }

    void log_last_error(const std::string& context) {
        DWORD error_code = GetLastError();
        LPSTR error_msg = nullptr;

        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&error_msg,
            0,
            nullptr);

        ui::cerr << context << ". Error " << error_code << ": " << (error_msg ? error_msg : "Unknown error") << ui::endl;

        if (error_msg) {
            LocalFree(error_msg);
        }
    }

    void process_next_command() {
        global_state.finished_flag = false;
        if (global_state.cancel_flag || global_state.command_queue.empty()) {
            global_state.running = false;
            global_state.finished_flag = true;
            return;
        }

        std::string command = global_state.command_queue.front();
        global_state.command_queue.pop();
        global_state.running = true;
        global_state.success_flag = false;

        ui::good << "$ " << command << ui::endl;

        global_state.future = std::async(std::launch::async, [command]() -> bool {
            HANDLE hStdOutRead, hStdOutWrite;
            HANDLE hStdErrRead, hStdErrWrite;
            STARTUPINFOA si = { sizeof(STARTUPINFOA) };
            PROCESS_INFORMATION pi = {};
            SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };

            auto cleanup = [&]() {
                CloseHandle(hStdOutRead);
                CloseHandle(hStdErrRead);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);

                std::lock_guard<std::mutex> lock(global_state.cmd_mutex);
                process_handle = nullptr;
                global_state.running = false;
            };

            try {
                if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0) ||
                    !CreatePipe(&hStdErrRead, &hStdErrWrite, &sa, 0)) {
                    log_last_error("Failed to create pipes for command output");
                    cleanup();
                    return false;
                }

                si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
                si.hStdOutput = hStdOutWrite;
                si.hStdError = hStdErrWrite;
                si.wShowWindow = SW_HIDE;

                if (!CreateProcessA(nullptr,
                    const_cast<char*>(command.c_str()),
                    nullptr,
                    nullptr,
                    TRUE,
                    CREATE_NO_WINDOW,
                    nullptr,
                    nullptr,
                    &si,
                    &pi)) {
                    log_last_error("Failed to start command");
                    cleanup();
                    return false;
                }

                CloseHandle(hStdOutWrite);
                CloseHandle(hStdErrWrite);

                {
                    std::lock_guard<std::mutex> lock(global_state.cmd_mutex);
                    process_handle = pi.hProcess;
                }

                auto read_pipe = [](HANDLE pipe, ui::terminal_stream& stream) {
                    char buffer[128];
                    DWORD bytesRead;

                    while (ReadFile(pipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr)) {
                        if (bytesRead > 0) {
                            buffer[bytesRead] = '\0';
                            stream << buffer;
                            stream.flush();
                        }
                    }
                };

                std::thread stdout_thread(read_pipe, hStdOutRead, std::ref(ui::cout));
                std::thread stderr_thread(read_pipe, hStdErrRead, std::ref(ui::cerr));

                stdout_thread.join();
                stderr_thread.join();

                DWORD exit_code = 0;
                if (!GetExitCodeProcess(pi.hProcess, &exit_code)) {
                    log_last_error("Failed to get exit code");
                    cleanup();
                    return false;
                }

                if (exit_code != 0) {
                    ui::cerr << "Command failed with exit code: " << exit_code << ui::endl;
                    cleanup();
                    return false;
                }

                global_state.success_flag = true;
            }
            catch (...) {
                ui::cerr << "Unexpected error occurred during command execution" << ui::endl;
                cleanup();
                throw;
            }

            cleanup();
            process_next_command();
            return true;
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
        if (process_handle) {
            TerminateProcess(process_handle, 1);
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
