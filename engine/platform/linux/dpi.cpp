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
#include <SDL2/SDL.h>
#include <cstdlib>

namespace platform {
namespace dpi {

    float get_scaling_factor(SDL_Window* window) {
        // Try SDL's built-in DPI detection first
        float ddpi, hdpi, vdpi;
        int display_index = SDL_GetWindowDisplayIndex(window);

        if (SDL_GetDisplayDPI(display_index, &ddpi, &hdpi, &vdpi) == 0) {
            return hdpi / 96.0f;  // 96 DPI is the baseline
        }

        // Fallback: Check GDK_SCALE environment variable (GNOME)
        const char* gdk_scale = std::getenv("GDK_SCALE");
        if (gdk_scale) {
            float scale = std::atof(gdk_scale);
            if (scale > 0.0f) return scale;
        }

        // Fallback: Check QT_SCALE_FACTOR (KDE/Qt apps)
        const char* qt_scale = std::getenv("QT_SCALE_FACTOR");
        if (qt_scale) {
            float scale = std::atof(qt_scale);
            if (scale > 0.0f) return scale;
        }

        return 1.0f;
    }

    void set_dpi_awareness() {
        // On Linux, DPI awareness is handled by the desktop environment
        // We can set SDL hints for better scaling behavior
        SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    }

} // namespace dpi
} // namespace platform
