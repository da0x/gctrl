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

#include <imgui.h>
#include <imgui_node_editor.h>
#include <string>

#include "ui/theme.hpp"
#include "ui/app.hpp"
#include "ui/theme_editor.hpp"
#include "ui/icons.hpp"
#include "ui/chart.hpp"
#include "ui/menu.hpp"

namespace ui {

    using std::string;
    typedef ImGuiCol_ color_t;

    namespace colors {
        ui::color_t text = ImGuiCol_Text;
        ui::color_t text_disabled = ImGuiCol_TextDisabled;
        ui::color_t window_bg = ImGuiCol_WindowBg;
        ui::color_t child_bg = ImGuiCol_ChildBg;
        ui::color_t popup_bg = ImGuiCol_PopupBg;
        ui::color_t border = ImGuiCol_Border;
        ui::color_t border_shadow = ImGuiCol_BorderShadow;
        ui::color_t frame_bg = ImGuiCol_FrameBg;
        ui::color_t frame_bg_hovered = ImGuiCol_FrameBgHovered;
        ui::color_t frame_bg_active = ImGuiCol_FrameBgActive;
        ui::color_t title_bg = ImGuiCol_TitleBg;
        ui::color_t title_bg_active = ImGuiCol_TitleBgActive;
        ui::color_t title_bg_collapsed = ImGuiCol_TitleBgCollapsed;
        ui::color_t menu_bar_bg = ImGuiCol_MenuBarBg;
        ui::color_t scrollbar_bg = ImGuiCol_ScrollbarBg;
        ui::color_t scrollbar_grab = ImGuiCol_ScrollbarGrab;
        ui::color_t scrollbar_grab_hovered = ImGuiCol_ScrollbarGrabHovered;
        ui::color_t scrollbar_grab_active = ImGuiCol_ScrollbarGrabActive;
        ui::color_t check_mark = ImGuiCol_CheckMark;
        ui::color_t slider_grab = ImGuiCol_SliderGrab;
        ui::color_t slider_grab_active = ImGuiCol_SliderGrabActive;
        ui::color_t button = ImGuiCol_Button;
        ui::color_t button_hovered = ImGuiCol_ButtonHovered;
        ui::color_t button_active = ImGuiCol_ButtonActive;
        ui::color_t header = ImGuiCol_Header;
        ui::color_t header_hovered = ImGuiCol_HeaderHovered;
        ui::color_t header_active = ImGuiCol_HeaderActive;
        ui::color_t separator = ImGuiCol_Separator;
        ui::color_t separator_hovered = ImGuiCol_SeparatorHovered;
        ui::color_t separator_active = ImGuiCol_SeparatorActive;
        ui::color_t resize_grip = ImGuiCol_ResizeGrip;
        ui::color_t resize_grip_hovered = ImGuiCol_ResizeGripHovered;
        ui::color_t resize_grip_active = ImGuiCol_ResizeGripActive;
        ui::color_t tab = ImGuiCol_Tab;
        ui::color_t tab_hovered = ImGuiCol_TabHovered;
        ui::color_t tab_selected = ImGuiCol_TabSelected;
        ui::color_t tab_selected_overline = ImGuiCol_TabSelectedOverline;
        ui::color_t tab_dimmed = ImGuiCol_TabDimmed;
        ui::color_t tab_dimmed_selected = ImGuiCol_TabDimmedSelected;
        ui::color_t tab_dimmed_selected_overline = ImGuiCol_TabDimmedSelectedOverline;
        ui::color_t docking_preview = ImGuiCol_DockingPreview;
        ui::color_t docking_empty_bg = ImGuiCol_DockingEmptyBg;
        ui::color_t plot_lines = ImGuiCol_PlotLines;
        ui::color_t plot_lines_hovered = ImGuiCol_PlotLinesHovered;
        ui::color_t plot_histogram = ImGuiCol_PlotHistogram;
        ui::color_t plot_histogram_hovered = ImGuiCol_PlotHistogramHovered;
        ui::color_t table_header_bg = ImGuiCol_TableHeaderBg;
        ui::color_t table_border_strong = ImGuiCol_TableBorderStrong;
        ui::color_t table_border_light = ImGuiCol_TableBorderLight;
        ui::color_t table_row_bg = ImGuiCol_TableRowBg;
        ui::color_t table_row_bg_alt = ImGuiCol_TableRowBgAlt;
        ui::color_t text_link = ImGuiCol_TextLink;
        ui::color_t text_selected_bg = ImGuiCol_TextSelectedBg;
        ui::color_t drag_drop_target = ImGuiCol_DragDropTarget;
        ui::color_t nav_highlight = ImGuiCol_NavHighlight;
        ui::color_t nav_windowing_highlight = ImGuiCol_NavWindowingHighlight;
        ui::color_t nav_windowing_dim_bg = ImGuiCol_NavWindowingDimBg;
        ui::color_t modal_window_dim_bg = ImGuiCol_ModalWindowDimBg;
    }

    typedef ImGuiStyleVar var_t;

    namespace vars {
        const var_t alpha = ImGuiStyleVar_Alpha;
        const var_t disabled_alpha = ImGuiStyleVar_DisabledAlpha;
        const var_t window_padding = ImGuiStyleVar_WindowPadding;
        const var_t window_rounding = ImGuiStyleVar_WindowRounding;
        const var_t window_border_size = ImGuiStyleVar_WindowBorderSize;
        const var_t window_min_size = ImGuiStyleVar_WindowMinSize;
        const var_t window_title_align = ImGuiStyleVar_WindowTitleAlign;
        const var_t child_rounding = ImGuiStyleVar_ChildRounding;
        const var_t child_border_size = ImGuiStyleVar_ChildBorderSize;
        const var_t popup_rounding = ImGuiStyleVar_PopupRounding;
        const var_t popup_border_size = ImGuiStyleVar_PopupBorderSize;
        const var_t frame_padding = ImGuiStyleVar_FramePadding;
        const var_t frame_rounding = ImGuiStyleVar_FrameRounding;
        const var_t frame_border_size = ImGuiStyleVar_FrameBorderSize;
        const var_t item_spacing = ImGuiStyleVar_ItemSpacing;
        const var_t item_inner_spacing = ImGuiStyleVar_ItemInnerSpacing;
        const var_t indent_spacing = ImGuiStyleVar_IndentSpacing;
        const var_t cell_padding = ImGuiStyleVar_CellPadding;
        const var_t scrollbar_size = ImGuiStyleVar_ScrollbarSize;
        const var_t scrollbar_rounding = ImGuiStyleVar_ScrollbarRounding;
        const var_t grab_min_size = ImGuiStyleVar_GrabMinSize;
        const var_t grab_rounding = ImGuiStyleVar_GrabRounding;
        const var_t tab_rounding = ImGuiStyleVar_TabRounding;
        const var_t tab_border_size = ImGuiStyleVar_TabBorderSize;
        const var_t tab_bar_border_size = ImGuiStyleVar_TabBarBorderSize;
        const var_t tab_bar_overline_size = ImGuiStyleVar_TabBarOverlineSize;
        const var_t table_angled_headers_angle = ImGuiStyleVar_TableAngledHeadersAngle;
        const var_t table_angled_headers_text_align = ImGuiStyleVar_TableAngledHeadersTextAlign;
        const var_t button_text_align = ImGuiStyleVar_ButtonTextAlign;
        const var_t selectable_text_align = ImGuiStyleVar_SelectableTextAlign;
        const var_t separator_text_border_size = ImGuiStyleVar_SeparatorTextBorderSize;
        const var_t separator_text_align = ImGuiStyleVar_SeparatorTextAlign;
        const var_t separator_text_padding = ImGuiStyleVar_SeparatorTextPadding;
        const var_t docking_separator_size = ImGuiStyleVar_DockingSeparatorSize;
    }

    enum class direction { ltr, rtl };

    inline void begin(const char* title) {
        ImGui::Begin(title, nullptr, ImGuiWindowFlags_DockNodeHost);
    }

    inline void begin(const char* title, ImGuiWindowFlags flags) {
        ImGui::Begin(title, nullptr, flags | ImGuiWindowFlags_DockNodeHost);
    }

    inline void end() {
        ImGui::End();
    }

    inline void dummy(float x, float y) {
        ImGui::Dummy(ImVec2(x, y));
    }

    inline void same_line() {
        ImGui::SameLine();
    }

    namespace style {
        namespace color {

            template <typename T>
            inline void push(color_t color_name, const T& value) {
                ImGui::PushStyleColor(color_name, value);
            }

            inline void pop() {
                ImGui::PopStyleColor();
            }
        }
        namespace var {

            template <typename T>
            inline void push(var_t var_name, const T& value) {
                ImGui::PushStyleVar(var_name, value);
            }

            inline void pop() {
                ImGui::PopStyleVar();
            }
        }
    }

    inline void columns(int n) {
        ImGui::Columns(n, nullptr, false);
    }

    inline void next_column() {
        ImGui::NextColumn();
    }

    namespace group {

        inline void begin() {
            ImGui::BeginGroup();
        }

        inline void end() {
            ImGui::EndGroup();
        }
    }

    namespace disabled {
        inline void begin() {
            ImGui::BeginDisabled();
        }

        inline void end() {
            ImGui::EndDisabled();
        }
    }

    inline void separator(const std::string& value = std::string()) {
        if (value.empty()) {
            ImGui::Separator();
        }
        else {
            ImGui::SeparatorText(value.c_str());
        }
    }

    inline void text(const std::string& value) {
        ImGui::Text(value.c_str());
    }

    inline ImVec2 text_size(const std::string& value) {
        return ImGui::CalcTextSize(value.c_str());
    }

    inline void tooltip(const std::string& value) {
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(value.c_str());
        }
    }

    inline bool selectable(const std::string& value, bool is_selected = false) {
        return ImGui::Selectable(value.c_str(), is_selected);
    }

    inline bool selectable(const std::string& value, bool* is_selected, ImGuiSelectableFlags flags = 0) {
        return ImGui::Selectable(value.c_str(), is_selected, flags);
    }

    inline bool button(const char* title) {
        return ImGui::Button(title);
    }

    inline bool bordered_button(const char* title) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(255, 255, 255, 255));

        bool result = ui::button(title);

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
        return result;
    }


    inline int combo(const char* label, int current_value, const std::map<int, std::string>& items) {
        std::vector<const char*> item_labels;
        for (const auto& item : items) {
            item_labels.push_back(item.second.c_str());
        }

        int current_index = 0;
        for (const auto& item : items) {
            if (item.first == current_value) break;
            current_index++;
        }

        if (ImGui::Combo(label, &current_index, item_labels.data(), static_cast<int>(item_labels.size()))) {
            auto it = std::next(items.begin(), current_index);
            return it->first;
        }

        return current_value;
    }

    namespace input {

        inline void text(const char* title, string& value) {
            char buffer[1024*10];
            strncpy(buffer, value.c_str(), sizeof(buffer));
            buffer[sizeof(buffer) - 1] = 0;

            if (ImGui::InputText(title, buffer, sizeof(buffer))) {
                value = buffer;
            }
        }

        inline void multiline_text(const char* title, string& value) {
            std::vector<char> desc_buffer(1024 * 100);
            std::strncpy(desc_buffer.data(), value.c_str(), desc_buffer.size());
            if (ImGui::InputTextMultiline(title, desc_buffer.data(), desc_buffer.size())) {
                value = std::string(desc_buffer.data());
            }
        }

        namespace engineering {
            inline float to_float(const std::string& str) {
                size_t pos;
                float value = std::stof(str, &pos);

                if (pos < str.size() && str[pos] == 'e') {
                    int exponent = std::stoi(str.substr(pos + 1));
                    return value * static_cast<float>(std::pow(10.0f, exponent));
                }

                return value;
            }

            inline std::string to_string(float value) {
                if (value == 0.0f) {
                    return "0e+0";
                }

                int exponent = static_cast<int>(std::floor(std::log10(std::abs(value)) / 3.0f)) * 3;
                float eng_value = static_cast<float>(value / std::pow(10.0f, exponent));

                std::ostringstream oss;
                oss << std::setprecision(6) << eng_value << "e" << (exponent >= 0 ? "+" : "") << exponent;
                return oss.str();
            }
        }

        void float_(const char* label, float& value) {
            std::string eng_str = engineering::to_string(value);
            std::vector<char> buffer(256);
            std::strncpy(buffer.data(), eng_str.c_str(), buffer.size());
            if (ImGui::InputText(label, buffer.data(), buffer.size())) {
                try {
                    value = engineering::to_float(buffer.data());
                }
                catch (const std::invalid_argument&) {}
            }
        }
    }

    namespace pin {

        namespace ed = ax::NodeEditor;

        enum type {
            input = ed::PinKind::Input,
            output = ed::PinKind::Output
        };

        inline void begin(uint64_t id, type pin_type) {
            ed::BeginPin(id, static_cast<ed::PinKind>(pin_type));
        }

        inline void end() {
            ed::EndPin();
        }
    }

    namespace id {

        inline void push(uint64_t id) {
            ImGui::PushID(static_cast<int>(id));
        }

        inline void push(const char* id) {
            ImGui::PushID(id);
        }

        inline void push(const void* id) {
            ImGui::PushID(id);
        }

        inline void pop() {
            ImGui::PopID();
        }

        template <typename T>
        inline unsigned int get(T id) {
            return ImGui::GetID(id);
        }

    }  // namespace id

    namespace node {

        namespace ed = ax::NodeEditor;

        std::map<uint64_t, std::string> editor_states;
        uint64_t current_context_id = 0;

        bool save_settings_callback(const char* data, size_t size, ax::NodeEditor::SaveReasonFlags reason, void* user_pointer) {
            if (user_pointer) {
                std::string* storage = static_cast<std::string*>(user_pointer);
                storage->assign(data, size);
                return true;
            }
            return false;
        }

        size_t load_settings_callback(char* data, void* user_pointer) {
            if (user_pointer) {
                const std::string* storage = static_cast<const std::string*>(user_pointer);
                size_t size = storage->size();
                if (data) {
                    memcpy(data, storage->data(), size);
                }
                return size;
            }
            return 0;
        }

        void context_begin(uint64_t id) {
            if (id == current_context_id) return;

            if (current_context_id != 0) {
                auto& current_state = editor_states[current_context_id];
                if (auto* ctx = ed::GetCurrentEditor()) {
                    ed::Config config = {};
                    config.SaveSettings = save_settings_callback;
                    config.UserPointer = &current_state;
                    save_settings_callback(nullptr, 0, {}, &current_state);
                    ed::DestroyEditor(ctx);
                }
            }

            current_context_id = id;

            ed::Config config = {};
            config.SettingsFile = nullptr;
            config.SaveSettings = save_settings_callback;
            config.LoadSettings = load_settings_callback;
            config.UserPointer = &editor_states[id];

            ed::SetCurrentEditor(ed::CreateEditor(&config));
        }

        void context_end() {}

        void begin(const std::string& id) {
            ed::Begin(id.c_str());
        }

        void end() {
            ed::End();
        }

        void begin_node(uint64_t id) {
            ed::BeginNode(ed::NodeId(id));
        }

        void end_node() {
            ed::EndNode();
        }

        nlohmann::json settings() {
            nlohmann::json json_data;
            for (const auto& [key, value] : editor_states) {
                json_data[std::to_string(key)] = nlohmann::json::parse(value, nullptr, false);
            }
            return json_data;
        }

        void settings(const nlohmann::json& ne_data) {
            if (ne_data.is_null()) return;

            for (const auto& [key, value] : ne_data.items()) {
                uint64_t id = std::stoull(key);
                editor_states[id] = value.dump();
            }
        }

#undef max 

        namespace padding {

            enum class Type { None, Input, Output };

            class Pad {
            private:
                float max_input_width = 0.0f;
                float max_output_width = 0.0f;
                float connector_size = 15.0f;
                float spacing = 20.0f;
                float padding = 40.0f;
                float title_width = 0.0f;

            public:
                Pad& operator<<(const std::string& text) {
                    if (title_width == 0.0f) {
                        title_width = ImGui::CalcTextSize(text.c_str()).x;
                    }
                    return *this;
                }

                Pad& operator<<(Type type) {
                    return *this;
                }

                Pad& add_input(const std::string& name) {
                    float input_width = ImGui::CalcTextSize(name.c_str()).x;
                    max_input_width = std::max(input_width, max_input_width);
                    return *this;
                }

                Pad& add_output(const std::string& name) {
                    float output_width = ImGui::CalcTextSize(name.c_str()).x;
                    max_output_width = std::max(output_width, max_output_width);
                    return *this;
                }

                float total_width() const {
                    float min_total_width = title_width + 2 * padding;
                    return std::max(min_total_width, max_input_width + max_output_width + 2 * connector_size + spacing);
                }

                float title_padding() const {
                    return std::max((total_width() - title_width) * 0.5f, padding);
                }

                float spacing_between_io() const {
                    float available_space = total_width() - (max_input_width + max_output_width + 2 * connector_size);
                    return available_space > 0.0f ? available_space : spacing;
                }

                float dummy_output_padding(const std::string& output_name) const {
                    float output_width = ImGui::CalcTextSize(output_name.c_str()).x;
                    return max_output_width - output_width;
                }
            };
        }
    }


    namespace main {
        namespace menu {
            inline bool begin() {
                return ImGui::BeginMainMenuBar();
            }

            inline void end() {
                ImGui::EndMainMenuBar();
            }
        }
    }

    namespace menu {

        inline bool begin(const std::string& label) {
            return ImGui::BeginMenu(label.c_str());
        }

        inline void end() {
            ImGui::EndMenu();
        }

        inline bool item(const std::string& label, const std::string& shortcut = "", bool enabled = true) {
            return ImGui::MenuItem(label.c_str(), shortcut.empty() ? nullptr : shortcut.c_str(), false, enabled);
        }

    }

    namespace popup {

        inline bool begin(const char* title) {
            ui::style::color::push(ui::colors::border, ui::theme::vs2022::dark_yellow);
            bool begin = ImGui::BeginPopup(title);
            if (!begin) {
                ui::style::color::pop();
            }
            return begin;
        }

        inline void end() {
            ui::style::color::pop();
            ImGui::EndPopup();
        }

        inline void open(const char* title) {
            ImGui::OpenPopup(title);
        }

        inline void close() {
            ImGui::CloseCurrentPopup();
        }
    }
}

#include <iostream>
#include <string>
#include <map>

namespace ui {
    namespace local {
        int64_t id(const std::string& class_name, int64_t unique_number) {
            static std::map<std::pair<std::string, int64_t>, int64_t> id_map;
            std::pair<std::string, int64_t> key = { class_name, unique_number };
            if (id_map.find(key) == id_map.end()) {
                id_map[key] = 0;
            }
            else {
                id_map[key]++;
            }
            return id_map[key];
        }
    }
}

namespace ui {

    inline void settings(const nlohmann::json& data) {
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;

        if (data.is_string()) {
            std::string ini_data = data.get<std::string>();
            ImGui::LoadIniSettingsFromMemory(ini_data.c_str(), ini_data.size());
        }
    }

    inline std::string settings() {
        size_t ini_size;
        const char* ini_data = ImGui::SaveIniSettingsToMemory(&ini_size);
        return std::string(ini_data, ini_size);
    }

} // namespace ui


#include <imgui.h>
#include <imgui_node_editor.h>

namespace ui {
    namespace plot {
        GLuint frame_buffer = 0;
        GLuint texture = 0;
        float width = 512;
        float height = 512;

        void initialize() {
            if (frame_buffer == 0) {
                glGenFramebuffers(1, &frame_buffer);
                glGenTextures(1, &texture);

                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                    throw std::runtime_error("framebuffer not complete");

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
        }

        void begin() {
            initialize();
            glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
            glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        void end() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            ImGui::Image((void*)(intptr_t)texture, ImVec2(width, height));
        }
    }
}

#include <map>
#include <cstdint>
#include <string>
#include <imgui.h>
namespace ui {
    namespace tree {

        namespace context {
            struct tree_context {
                std::map<uint64_t, bool> node_expanded;
                std::map<uint64_t, bool> node_selected;
                uint64_t last_selected = 0;
            };

            inline std::map<std::string, tree_context> contexts;
            inline tree_context* current_context = nullptr;

            inline void begin(const char* context_id) {
                if (contexts.find(context_id) == contexts.end()) {
                    contexts[context_id] = tree_context();
                }
                current_context = &contexts[context_id];
            }

            inline void end() {
                current_context = nullptr;
            }
        }

        inline bool child(uint64_t id, const std::function<void()>& render_title) {
            if (context::current_context == nullptr) return false;
            bool is_selected = (context::current_context->last_selected == id);
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_SpanAvailWidth |
                (is_selected ? ImGuiTreeNodeFlags_Selected : 0);
            bool is_open = ImGui::TreeNodeEx(("##" + std::to_string(id)).c_str(), flags);
            if (ImGui::IsItemClicked()) {
                context::current_context->last_selected = id;
            }
            ui::same_line();
            render_title();
            if (!is_open) {
                context::current_context->node_expanded[id] = false;
                return false;
            }
            context::current_context->node_expanded[id] = true;
            return true;
        }

        inline void child_end() {
            if (context::current_context != nullptr) {
                ImGui::TreePop();
            }
        }

        inline void leaf(uint64_t id, const std::function<void()>& render_title) {
            if (context::current_context == nullptr) return;
            bool is_selected = (context::current_context->last_selected == id);
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            bool is_open = ImGui::TreeNodeEx(("##" + std::to_string(id)).c_str(), flags);
            if (ImGui::IsItemClicked()) {
                context::current_context->last_selected = id;
            }
            ui::same_line();
            render_title();
            if (!is_open) {
                context::current_context->node_expanded[id] = false;
            }
            context::current_context->node_expanded[id] = true;
        }




        inline bool is_selected(uint64_t id) {
            if (context::current_context == nullptr) return false;
            return context::current_context->last_selected == id;
        }

    } // namespace tree
} // namespace ui


#include "ui/terminal.hpp"
