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

#include <IconFontCppHeaders/IconsFontAwesome6.h>


namespace ui {
	namespace icon {
		using type = const char*;

		inline constexpr type save = ICON_FA_FLOPPY_DISK;
		inline constexpr type edit = ICON_FA_PENCIL " Edit";
		inline constexpr type insert = ICON_FA_ARROW_RIGHT " Insert";
		inline constexpr type trash = ICON_FA_TRASH;
		inline constexpr type play = ICON_FA_PLAY;
		inline constexpr type stop = ICON_FA_STOP;
		inline constexpr type build = ICON_FA_ARROW_DOWN_SHORT_WIDE;
		inline constexpr type rebuild = ICON_FA_ROTATE_RIGHT;

		inline constexpr type machine = ICON_FA_SERVER;
		inline constexpr type controller = ICON_FA_GEAR;
		inline constexpr type element = ICON_FA_MICROCHIP;
		inline constexpr type function = ICON_FA_CODE;
		inline constexpr type driver = ICON_FA_GEARS;
		inline constexpr type plug = ICON_FA_SQUARE_PLUS;
		inline constexpr type socket = ICON_FA_SQUARE_MINUS;
		inline constexpr type signal = ICON_FA_BOLT;

		inline constexpr type square = ICON_FA_SQUARE;
	}
}