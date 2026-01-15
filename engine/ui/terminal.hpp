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

#include "platform/platform.hpp"
#include "platform/process.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <queue>
#include <imgui.h>
#include "ui/font.hpp"
#include <iostream>
#include <mutex>
#include <future>
#include <atomic>
#include <map>
#include <stdexcept>

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
            ImGui::Begin("Terminal", nullptr, ImGuiWindowFlags_None);

            if (ImGui::Button("Clear")) {
                terminal_log.clear();
            }
            ImGui::SameLine();
            if (ImGui::Button("Copy")) {
                ImGui::LogToClipboard();
                for (const auto& [line, stream_color] : terminal_log) {
                    ImGui::LogText("%s", line.c_str());
                }
                ImGui::LogFinish();
            }

            ImGui::Separator();

            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(30, 30, 30, 255));
            ImGui::BeginChild("terminal_output", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysHorizontalScrollbar);

            font::push(font::type::console);
            for (const auto& [line, stream_color] : terminal_log) {
                ImVec4 color_vec = colors::to_imvec4(stream_color);
                ImGui::PushStyleColor(ImGuiCol_Text, color_vec);
                ImGui::TextUnformatted(line.c_str());
                ImGui::PopStyleColor();
            }

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
            font::pop();

            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            ImGui::End();
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

    // Thin wrappers around platform::process functions
    inline void execute(const std::vector<std::string>& commands) {
        platform::process::execute(commands);
    }

    inline void cancel() {
        platform::process::cancel();
    }

    inline bool is_running() {
        return platform::process::is_running();
    }

    inline bool success() {
        return platform::process::success();
    }

    inline bool finished() {
        return platform::process::finished();
    }

} // namespace terminal

