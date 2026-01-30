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

#include "platform/platform.hpp"
#include "controls/debug/thread.hpp"
#include "ui/ui.hpp"
#include "project.hpp"
#include "controls/engine.hpp"
#include <filesystem>

namespace fs = std::filesystem;

#if GCTRL_PLATFORM_WINDOWS
void parse_command_line(fs::path& project_path, LPSTR lpCmdLine) {
    std::string cmd_line(lpCmdLine);
    if (!cmd_line.empty()) {
        fs::path path(cmd_line);
        if (fs::exists(path) && path.extension() == ".ctrl") {
            project_path = fs::absolute(path);
            fs::current_path(project_path.parent_path());
        }
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    fs::path project_path;
    parse_command_line(project_path, lpCmdLine);
#else
void parse_command_line(fs::path& project_path, int argc, char* argv[]) {
    if (argc > 1) {
        fs::path path(argv[1]);
        if (fs::exists(path) && path.extension() == ".ctrl") {
            project_path = fs::absolute(path);
            fs::current_path(project_path.parent_path());
        }
    }
}

int main(int argc, char* argv[]) {
    fs::path project_path;
    parse_command_line(project_path, argc, argv);
#endif
    ui::good << "gctrl v0.0.1" << ui::endl;
    ui::good << "Copyright(C) 2024 www.gctrl.org" << ui::endl;
    ui::warn << "This is free software; see the source for copying conditions." << ui::endl;
    ui::warn << "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << ui::endl;

    ui::app app("General Controls Engine");

    ui::theme::vs2022::apply();

    while (app.frame()) {
        project::show_project_modal(project_path);
        app.render();
        if (!project_path.empty()) {
            break;
        }
    }

    if (!project_path.empty()) {
        controls::engine e(project_path);
        // Old debug listener disabled - dashboards now handle signal visualization

        while (app.frame()) {
            e.render();
            app.render();
        }
    }

    return 0;
}