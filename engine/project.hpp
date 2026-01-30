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
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <da0x/uuid.hpp>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include "ui/ui.hpp"
#include "imgui-file-dialog/imgui_file_dialog.hpp"

namespace project {

    struct recent_project {
        std::string id;
        std::string path;
        std::string last_opened;

        recent_project(const std::string& p)
            : id(uuid::v4::generate()),
            path(p) {
            update_last_opened();
        }

        void update_last_opened() {
            auto now = std::chrono::system_clock::now();
            std::time_t last_opened_time = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&last_opened_time), "%Y-%m-%d %H:%M:%S");
            last_opened = ss.str();
        }

        operator bool() const {
            return !path.empty();
        }

        static std::string get_current_time_string() {
            auto now = std::chrono::system_clock::now();
            std::time_t last_opened_time = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&last_opened_time), "%Y-%m-%d %H:%M:%S");
            return ss.str();
        }
    };

    using json = nlohmann::json;

    inline std::vector<recent_project> load_recent_projects() {
        std::vector<recent_project> recent_projects;
        const char* home_dir = std::getenv("HOME");
        if (!home_dir) {
#ifdef _WIN32
            home_dir = std::getenv("USERPROFILE");
#else
            return recent_projects;
#endif
        }
        std::ifstream file(std::string(home_dir) + "/.gctrlrc.json");
        if (file.is_open()) {
            json j;
            file >> j;
            for (const auto& project : j["recent_projects"]) {
                recent_project rp(project["path"]);
                rp.id = project["id"];
                rp.last_opened = project["last_opened"];
                recent_projects.push_back(rp);
            }
        }
        return recent_projects;
    }

    inline void save_recent_projects(const std::vector<recent_project>& recent_projects) {
        const char* home_dir = std::getenv("HOME");
        if (!home_dir) {
#ifdef _WIN32
            home_dir = std::getenv("USERPROFILE");
#else
            return;
#endif
        }
        std::ofstream file(std::string(home_dir) + "/.gctrlrc.json", std::ios::trunc);
        json j;
        for (const auto& project : recent_projects) {
            j["recent_projects"].push_back({
                {"id", project.id},
                {"path", project.path},
                {"last_opened", project.last_opened}
                });
        }
        file << j.dump(4);
    }

    inline void update_recent_projects(const std::string& new_project_path, std::vector<recent_project>& recent_projects) {
        auto it = std::find_if(recent_projects.begin(), recent_projects.end(), [&new_project_path](const recent_project& rp) {
            return rp.path == new_project_path;
            });

        if (it != recent_projects.end()) {
            // Move existing project to front
            recent_project proj = *it;
            proj.update_last_opened();
            recent_projects.erase(it);
            recent_projects.insert(recent_projects.begin(), proj);
        }
        else {
            // Insert new project at front
            recent_projects.insert(recent_projects.begin(), recent_project(new_project_path));
        }

        if (recent_projects.size() > 10) {
            recent_projects.pop_back();
        }

        save_recent_projects(recent_projects);
    }

    inline void show_project_list(std::filesystem::path& selected_project_path, std::vector<recent_project>& recent_projects) {
        ui::separator("Open recent");
        ImGui::BeginChild("Project List", ImVec2(0, 0), false);
        for (size_t i = 0; i < recent_projects.size(); ++i) {
            auto& project = recent_projects[i];
            std::filesystem::path project_path(project.path);

            ui::font::push(ui::font::type::ui_large);
            if (ImGui::Selectable(project_path.filename().string().c_str(), false, ImGuiSelectableFlags_DontClosePopups)) {
                selected_project_path = project_path;
                std::filesystem::current_path(project_path.parent_path());
                update_recent_projects(project.path, recent_projects);
            }
            ui::font::pop();

            ImGui::TextColored(ui::theme::vs2022::text_disabled, project.path.c_str());
            ImGui::TextColored(ui::theme::vs2022::text_disabled, ("Last opened: " + project.last_opened).c_str());

            ImGui::SameLine(ImGui::GetWindowWidth() - 50);
            if (ui::button((std::string(ui::icon::trash) + "##" + std::to_string(i)).c_str())) {
                recent_projects.erase(recent_projects.begin() + i);
                save_recent_projects(recent_projects);
                --i;
            }

            ui::separator();
        }
        ImGui::EndChild();
    }

    inline void show_action_buttons() {

        ui::separator("Get started");
        if (ui::button("New Project")) {
            ui::popup::open("Select Folder for New Project");
        }
        if (ui::button("Open Project")) {
            ui::popup::open("Select .ctrl Project File");
        }
        if (ui::button("Clone Repository")) {
            ui::popup::open("Clone Project");
        }
    }

    inline void handle_new_project_selection(std::filesystem::path& selected_project_path, std::vector<recent_project>& recent_projects) {
        static char project_name[256] = "";
        static char path[500] = "";
        if (ImGui::BeginPopupModal("Select Folder for New Project", nullptr, ImGuiWindowFlags_None)) {
            ImGui::InputText("Project Name", project_name, sizeof(project_name));
            ui::separator("Select Folder");
            if (ui::select_folder(path, sizeof(path))) {

                if (strlen(path) > 0 && strlen(project_name) > 0) {
                    std::filesystem::path new_project_path = std::filesystem::path(path) / (std::string(project_name) + ".ctrl");
                    json new_project_json;
                    new_project_json["name"] = project_name;
                    new_project_json["created"] = recent_project::get_current_time_string();
                    std::ofstream out_file(new_project_path);
                    out_file << new_project_json.dump(4);

                    selected_project_path = new_project_path;
                    std::filesystem::current_path(selected_project_path.parent_path());
                    update_recent_projects(selected_project_path.string(), recent_projects);
                    ImGui::CloseCurrentPopup();
                }
            }
            ui::same_line();
            if (ui::button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    inline void handle_open_project_selection(std::filesystem::path& selected_project_path, std::vector<recent_project>& recent_projects) {
        static char path[500] = "";
        if (ImGui::BeginPopupModal("Select .ctrl Project File", nullptr, ImGuiWindowFlags_None)) {
            ImGui::TextUnformatted("Please select a .ctrl project file:");
            if (ui::select_file(path, sizeof(path), { ".ctrl" })) {
                if (strlen(path) > 0) {
                    std::filesystem::path open_project_path(path);
                    if (open_project_path.extension() == ".ctrl") {
                        selected_project_path = open_project_path;
                        std::filesystem::current_path(selected_project_path.parent_path());
                        update_recent_projects(selected_project_path.string(), recent_projects);
                        ImGui::CloseCurrentPopup();
                        ImGui::LoadIniSettingsFromDisk("imgui.ini");
                    }
                    else {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: Selected file must have a .ctrl extension!");
                    }
                }
            }
            ui::same_line();
            if (ui::button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    inline void show_project_modal(std::filesystem::path& selected_project_path) {
        static std::vector<recent_project> recent_projects = load_recent_projects();

        ImGui::Begin("Project Manager");

        ui::text("Warning: This is software is in alpha stage. This is an unstable build. Please use external source control to save your work.");
        ui::separator();

        ImGui::Columns(2, nullptr, true);
        ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.7f);

        show_project_list(selected_project_path, recent_projects);

        ImGui::NextColumn();

        show_action_buttons();

        handle_new_project_selection(selected_project_path, recent_projects);
        handle_open_project_selection(selected_project_path, recent_projects);

        ImGui::Columns(1);
        ImGui::End();
    }
}
