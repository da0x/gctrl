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

#include "platform/font.hpp"
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <limits.h>

namespace fs = std::filesystem;

namespace platform {
namespace font {

    std::vector<std::string> get_font_search_paths() {
        std::vector<std::string> paths;

        // User fonts
        const char* home = std::getenv("HOME");
        if (home) {
            paths.push_back(std::string(home) + "/.local/share/fonts");
            paths.push_back(std::string(home) + "/.fonts");
        }

        // System fonts
        paths.push_back("/usr/share/fonts");
        paths.push_back("/usr/local/share/fonts");

        return paths;
    }

    static std::string find_font(const std::vector<std::string>& candidates) {
        for (const auto& base_path : get_font_search_paths()) {
            if (!fs::exists(base_path)) continue;

            try {
                for (auto& entry : fs::recursive_directory_iterator(
                    base_path, fs::directory_options::skip_permission_denied)) {
                    for (const auto& font : candidates) {
                        if (entry.path().filename() == font) {
                            return entry.path().string();
                        }
                    }
                }
            } catch (...) {
                // Skip directories we can't access
            }
        }
        return "";
    }

    std::string get_system_ui_font() {
        // Check common paths directly first (fast)
        static const char* known_paths[] = {
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/TTF/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
            "/usr/share/fonts/noto/NotoSans-Regular.ttf"
        };

        for (const auto& path : known_paths) {
            if (fs::exists(path)) return path;
        }

        // Fall back to search if known paths don't exist
        std::vector<std::string> candidates = {
            "DejaVuSans.ttf",
            "LiberationSans-Regular.ttf",
            "Ubuntu-R.ttf"
        };
        return find_font(candidates);
    }

    std::string get_system_monospace_font() {
        // Check common paths directly first (fast)
        static const char* known_paths[] = {
            "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
            "/usr/share/fonts/TTF/LiberationMono-Regular.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
            "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf"
        };

        for (const auto& path : known_paths) {
            if (fs::exists(path)) return path;
        }

        std::vector<std::string> candidates = {
            "DejaVuSansMono.ttf",
            "LiberationMono-Regular.ttf"
        };
        return find_font(candidates);
    }

    static std::string get_executable_dir() {
        char path[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
        if (len != -1) {
            path[len] = '\0';
            return fs::path(path).parent_path().string();
        }
        return ".";
    }

    std::optional<std::vector<uint8_t>> load_font_resource(resource_id id) {
        // On Linux, load from bundled font files relative to executable
        std::string font_file;
        switch (id) {
            case CASCADIA_CODE:
                font_file = "fonts/cascadia-code.ttf";
                break;
            case FONT_AWESOME:
                font_file = "fonts/fa-solid-900.otf";
                break;
            case CONSOLA:
                font_file = "fonts/consola.ttf";
                break;
            default:
                return std::nullopt;
        }

        std::string exe_dir = get_executable_dir();
        std::string font_path = exe_dir + "/../" + font_file;

        std::ifstream file(font_path, std::ios::binary | std::ios::ate);
        if (!file) return std::nullopt;

        auto size = file.tellg();
        std::vector<uint8_t> buffer(size);
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), size);

        if (!file) return std::nullopt;

        return buffer;
    }

} // namespace font
} // namespace platform
