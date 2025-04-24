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

#include <Windows.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include <implot.h>
#include "ui/theme.hpp"
#include <windows.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_syswm.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#include <shellscalingapi.h>
#pragma comment(lib, "Shcore.lib")
#include <fstream>
#include "resources/resource.h"
#include <VersionHelpers.h>
#include "ui/font.hpp"
#include "ui/icons.hpp"
#include "ui/dpi.hpp"

namespace ui {

    class app {
    public:
        SDL_Window* window;
        SDL_GLContext gl_context;
        bool fullscreen = false;

        app(const char* title) {
            SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
                return;

            window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
            gl_context = SDL_GL_CreateContext(window);
            SDL_GL_MakeCurrent(window, gl_context);
            SDL_GL_SetSwapInterval(1);

            glewExperimental = GL_TRUE;
            if (glewInit() != GLEW_OK)
                return;


            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            ImGui::CreateContext();
            ImPlot::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

            ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
            ImGui_ImplOpenGL3_Init("#version 130");

            float scaling_factor = scaling::get_scaling_factor_from_sdl(window);
            io.FontGlobalScale = scaling_factor;

            const ImWchar glyph_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
            ui::font::load(ui::font::type::ui, "C:\\Windows\\Fonts\\segoeui.ttf", 16.0f);
            ui::font::load_from_resource(ui::font::type::ui, IDR_FONT_AWESOME, 12.0f, true, glyph_ranges);
            ui::font::load(ui::font::type::ui_large, "C:\\Windows\\Fonts\\segoeui.ttf", 28.0f);
            ui::font::load_from_resource(ui::font::type::code, IDR_FONT_CASCADIA_CODE, 16.0f);
            ui::font::load_from_resource(ui::font::type::code, IDR_FONT_AWESOME, 12.0f, true, glyph_ranges);
            ui::font::load_from_resource(ui::font::type::console, IDR_FONT_CONSOLA, 16.0f);

            io.Fonts->Build();
            apply_theme();
        }

        ~app() {
            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->Clear();

            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImPlot::DestroyContext();
            ImGui::DestroyContext();
            SDL_GL_DeleteContext(gl_context);
            SDL_DestroyWindow(window);
            SDL_Quit();
        }

        bool frame() {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F12) {
                    return false;
                }
                else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F11) {
                    toggle_fullscreen();
                }
                ImGui_ImplSDL2_ProcessEvent(&event);
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);

//          ImGui::ShowMetricsWindow();

            return true;
        }

        void render() {
            ImGui::Render();
            glViewport(0, 0, 1280, 720);
            glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
                SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
            }

            SDL_GL_SwapWindow(window);
        }

    private:
        void apply_theme() {
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        void toggle_fullscreen() {
            fullscreen = !fullscreen;

            if (fullscreen) {
                SDL_DisplayMode current;
                SDL_GetCurrentDisplayMode(0, &current);

                SDL_SetWindowFullscreen(window, 0);
                SDL_SetWindowBordered(window, SDL_FALSE);
                SDL_SetWindowSize(window, current.w, current.h);
                SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            }
            else {
                SDL_SetWindowBordered(window, SDL_TRUE);
                SDL_SetWindowSize(window, 1280, 720);
                SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            }
        }
    };

} // namespace ui
