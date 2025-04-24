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
		typedef const char* type;

		type save = ICON_FA_FLOPPY_DISK;
		type edit = ICON_FA_PENCIL " Edit";
		type insert = ICON_FA_ARROW_RIGHT " Insert";
		type trash = ICON_FA_TRASH;
		type play = ICON_FA_PLAY;
		type stop = ICON_FA_STOP;
		type build = ICON_FA_ARROW_DOWN_SHORT_WIDE;
		type rebuild = ICON_FA_ROTATE_RIGHT; 

		type machine = ICON_FA_SERVER;
		type controller = ICON_FA_GEAR;
		type element = ICON_FA_MICROCHIP;
		type function = ICON_FA_CODE;
		type driver = ICON_FA_GEARS;
		type plug = ICON_FA_SQUARE_PLUS;
		type socket = ICON_FA_SQUARE_MINUS;
		type signal = ICON_FA_BOLT;

		type square = ICON_FA_SQUARE;
	}
}