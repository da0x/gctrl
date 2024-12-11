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
#include <sstream>
#include <filesystem>
#include <queue>
#include <imgui.h>
#include "ui/ui.hpp"
#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <future>
#include <atomic>
#include <windows.h>
#include <map>
#include <queue>
#include <string>
#include <stdexcept>
#include <string>
#include <map>

namespace ui {

    enum class color {
        normal,
        red,
        green,
        orange
    };

    namespace colors {
        inline ImVec4 to_imvec4(color stream_color) {
            switch (stream_color) {
            case color::normal: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            case color::red: return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
            case color::green: return ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
            case color::orange: return ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
            default: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            }
        }
    }


    inline std::ostream& endl(std::ostream& os);

    class terminal_stream : public std::ostream {
    public:
        terminal_stream(color stream_color)
            : std::ostream(nullptr), stream_color(stream_color) {}

        template<typename T>
        terminal_stream& operator<<(const T& value) {
            buffer << value;
            return *this;
        }

        terminal_stream& operator<<(std::ostream& (*pf)(std::ostream&)) {
            if (pf == static_cast<std::ostream & (*)(std::ostream&)>(endl)) {
                terminal_log.push_back({ "", stream_color });
                flush();
            }
            return *this;
        }

        void flush() {
            if (!buffer.str().empty()) {
                if (!terminal_log.empty() && terminal_log.back().second == stream_color) {
                    terminal_log.back().first += buffer.str();
                }
                else {
                    terminal_log.push_back({ buffer.str(), stream_color });
                }
                buffer.str("");
                buffer.clear();
            }
        }

        static void render() {
            ui::begin("Terminal");

            if (ui::button("Clear")) {
                terminal_log.clear();
            }
            ui::same_line();
            if (ui::button("Copy")) {
                ImGui::LogToClipboard();
                for (const auto& [line, stream_color] : terminal_log) {
                    ImGui::LogText(line.c_str());
                }
                ImGui::LogFinish();
            }

            ui::separator();

            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(30, 30, 30, 255));
            ImGui::BeginChild("terminal_output", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysHorizontalScrollbar);

            ui::font::push(ui::font::type::console);
            for (const auto& [line, stream_color] : terminal_log) {
                ImVec4 color_vec = colors::to_imvec4(stream_color);
                ImGui::PushStyleColor(ImGuiCol_Text, color_vec);
                ImGui::TextUnformatted(line.c_str());
                ImGui::PopStyleColor();
            }

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
            ui::font::pop();

            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            ui::end();
        }

    private:
        std::ostringstream buffer;
        color stream_color;
        static inline std::vector<std::pair<std::string, color>> terminal_log;
    };

    inline std::ostream& endl(std::ostream& os) {
        auto* stream = dynamic_cast<terminal_stream*>(&os);
        if (stream) {
            stream->flush();
        }
        return os;
    }



    inline terminal_stream cout(color::normal);
    inline terminal_stream cerr(color::red);
    inline terminal_stream good(color::green);
    inline terminal_stream warn(color::orange);

    namespace terminal {
        inline void render() {
            terminal_stream::render();
        }
    }

} // namespace ui

namespace terminal {

    namespace details {
        std::atomic<bool> running = false;
        std::atomic<bool> cancel_flag = false;
        std::atomic<bool> success_flag = false;
        std::atomic<bool> finished_flag = false;
        std::future<bool> future;
        std::mutex cmd_mutex;
        std::queue<std::string> command_queue;
        HANDLE process_handle = nullptr;

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
            finished_flag = false;
            if (cancel_flag || command_queue.empty()) {
                running = false;
                finished_flag = true;
                return;
            }

            std::string command = command_queue.front();
            command_queue.pop();
            running = true;
            success_flag = false;

            ui::good << "$ " << command << ui::endl;

            future = std::async(std::launch::async, [command]() -> bool {
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

                    std::lock_guard<std::mutex> lock(cmd_mutex);
                    process_handle = nullptr;
                    running = false;
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
                        std::lock_guard<std::mutex> lock(cmd_mutex);
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

                    success_flag = true;
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

    } // namespace details

    void execute(const std::vector<std::string>& commands) {
        std::lock_guard<std::mutex> lock(details::cmd_mutex);
        if (details::running) {
            throw std::runtime_error("Cannot schedule commands while another group is running.");
        }

        details::command_queue = std::queue<std::string>(std::deque<std::string>(commands.begin(), commands.end()));
        details::cancel_flag = false;
        details::success_flag = true;
        details::finished_flag = false;
        details::process_next_command();
    }

    void cancel() {
        std::lock_guard<std::mutex> lock(details::cmd_mutex);
        if (!details::running) {
            throw std::runtime_error("Cannot cancel a command that is not running.");
        }
        details::cancel_flag = true;
        if (details::process_handle) {
            TerminateProcess(details::process_handle, 1);
        }
    }

    bool is_running() {
        return details::running;
    }

    bool success() {
        return details::success_flag;
    }

    bool finished() {
        if (details::finished_flag) {
            details::finished_flag = false;
            return true;
        }
        return false;
    }

} // namespace exec

