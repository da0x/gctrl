#pragma once

#include <imgui.h>
#include <chrono>
#include <string>
#include <filesystem>
#include <vector>

namespace ui {

    enum class file_dialog_sort_order {
        up,
        down,
        none
    };

    static bool file_dialog_open = false;
    static char file_dialog_error[500] = "";

    // Common helper function for displaying directories
    void display_directories(const std::string& current_path, size_t& folder_select_index, std::string& current_folder, std::string& new_path) {
        // Use available space intelligently, but not all of it
        float available_height = ImGui::GetContentRegionAvail().y - 100; // Adjust this value as needed to leave room for other content
        if (available_height < 150) { // Minimum height threshold to prevent shrinking too much
            available_height = 150;
        }

        ImGui::BeginChild("directories##1", ImVec2(0, available_height), true, ImGuiWindowFlags_HorizontalScrollbar);

        if (ImGui::Selectable("..", false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            if (ImGui::IsMouseDoubleClicked(0)) {
                new_path = std::filesystem::path(current_path).parent_path().string();
                current_folder = "";
            }
        }

        int index = 0;
        for (const auto& entry : std::filesystem::directory_iterator(current_path)) {
            if (entry.is_directory()) {
                if (ImGui::Selectable(entry.path().stem().string().c_str(), index == folder_select_index, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                    current_folder = entry.path().stem().string();
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        new_path = entry.path().string();
                        current_folder = "";
                        folder_select_index = 0;
                        ImGui::SetScrollHereY(0.0f);
                    }
                    else {
                        folder_select_index = index;
                    }
                }
                ++index;
            }
        }
        ImGui::EndChild();
    }



    // Common helper function for displaying files
    void display_files(const std::string& current_path, size_t& file_select_index, std::string& current_file, const std::vector<std::string>& extensions = {}) {
        std::vector<std::filesystem::directory_entry> files;
        try {
            for (const auto& entry : std::filesystem::directory_iterator(current_path)) {
                if (!entry.is_directory()) {
                    if (extensions.empty() || std::find(extensions.begin(), extensions.end(), entry.path().extension().string()) != extensions.end()) {
                        files.push_back(entry);
                    }
                }
            }
        }
        catch (...) {
            strcpy_s(file_dialog_error, "error: unable to read directory.");
        }

        // Use available space intelligently for the files display
        float available_height = ImGui::GetContentRegionAvail().y - 100; // Adjust this value as needed to leave room for other content
        if (available_height < 150) { // Minimum height threshold
            available_height = 150;
        }

        ImGui::BeginChild("files##1", ImVec2(0, available_height), true, ImGuiWindowFlags_HorizontalScrollbar);
        for (size_t index = 0; index < files.size(); ++index) {
            const auto& file = files[index];
            if (ImGui::Selectable(file.path().filename().string().c_str(), index == file_select_index, ImGuiSelectableFlags_AllowDoubleClick)) {
                file_select_index = index;
                current_file = file.path().filename().string();
            }
        }
        ImGui::EndChild();
    }

    // Function for selecting a file
    bool select_file(char* buffer, unsigned int buffer_size, const std::vector<std::string>& extensions = {}) {
        static std::string current_path = std::filesystem::current_path().string();
        static size_t file_select_index = 0;
        static std::string current_file = "";

        if (strlen(file_dialog_error) > 0) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", file_dialog_error);
        }

        // Top: Display current path
        ImGui::Text("Path: %s", current_path.c_str());

        // Two columns: Left for directories, right for files
        ImGui::Columns(2, nullptr, false);
        display_directories(current_path, file_select_index, current_file, current_path);
        ImGui::NextColumn();
        display_files(current_path, file_select_index, current_file, extensions);
        ImGui::Columns(1);

        // Copy selected file path to the buffer if a file is selected
        if (!current_file.empty()) {
            std::string full_path = current_path + (current_path.back() == '\\' ? "" : "\\") + current_file;
            strncpy(buffer, full_path.c_str(), buffer_size - 1);
            buffer[buffer_size - 1] = '\0'; // Ensure null-termination
        }

        // Display selected file path in the input field
        ImGui::Separator();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##selected_file", buffer, buffer_size, ImGuiInputTextFlags_ReadOnly);

        // Action buttons
        if (ImGui::Button("Open")) {
            if (current_file.empty()) {
                strcpy_s(file_dialog_error, "error: you must select a file!");
            }
            else {
                file_dialog_open = false;
                return true;
            }
        }
        return false;
    }


    // Function for selecting a folder
    bool select_folder(char* buffer, unsigned int buffer_size) {
        static std::string current_path = std::filesystem::current_path().string();
        static size_t folder_select_index = 0;
        static std::string current_folder = "";

        if (strlen(file_dialog_error) > 0) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", file_dialog_error);
        }

        // Top: Display current path
        ImGui::Text("Path: %s", current_path.c_str());

        // One column for directories
        display_directories(current_path, folder_select_index, current_folder, current_path);


        // Copy selected file path to the buffer if a file is selected
        if (!current_folder.empty()) {
            std::string full_path = current_path + (current_path.back() == '\\' ? "" : "\\") + current_folder;
            strncpy(buffer, full_path.c_str(), buffer_size - 1);
            buffer[buffer_size - 1] = '\0'; // Ensure null-termination
        }

        // Bottom: Display selected folder path
        ImGui::Separator();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##selected_folder", buffer, buffer_size);

        // Action buttons
        if (ImGui::Button("Open")) {
            if (current_folder.empty()) {
                // Default to current path if no folder is selected
                strcpy_s(buffer, buffer_size, current_path.c_str());
            }
            else {
                auto path = current_path + (current_path.back() == '\\' ? "" : "\\") + current_folder;
                strcpy_s(buffer, buffer_size, path.c_str());
            }
            strcpy_s(file_dialog_error, "");
            file_dialog_open = false;
            return true;
        }
        return false;
    }
}
