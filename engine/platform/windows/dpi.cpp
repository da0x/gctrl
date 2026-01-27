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

#include "platform/dpi.hpp"
#include <Windows.h>
#include <shellscalingapi.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#pragma comment(lib, "Shcore.lib")

namespace platform {
namespace dpi {

    float get_scaling_factor(SDL_Window* window) {
        SDL_SysWMinfo wm_info;
        SDL_VERSION(&wm_info.version);
        if (SDL_GetWindowWMInfo(window, &wm_info)) {
            HWND hwnd = wm_info.info.win.window;

            HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
            DEVICE_SCALE_FACTOR scale_factor;
            if (SUCCEEDED(GetScaleFactorForMonitor(monitor, &scale_factor))) {
                return static_cast<float>(scale_factor) / 100.0f;
            }
            else {
                // Fall back to DPI-based scaling
                UINT dpi = GetDpiForWindow(hwnd);
                return dpi / 96.0f;
            }
        }
        return 1.0f;
    }

    void set_dpi_awareness() {
        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    }

} // namespace dpi
} // namespace platform
