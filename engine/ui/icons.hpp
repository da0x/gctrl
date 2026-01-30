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

		// Icons only
		inline constexpr type trash = ICON_FA_TRASH;
		inline constexpr type plus = ICON_FA_PLUS;
		inline constexpr type minus = ICON_FA_MINUS;
		inline constexpr type play = ICON_FA_PLAY;
		inline constexpr type stop = ICON_FA_STOP;
		inline constexpr type hammer = ICON_FA_HAMMER;
		inline constexpr type circle = ICON_FA_CIRCLE;
		inline constexpr type square = ICON_FA_SQUARE;
		inline constexpr type check = ICON_FA_CHECK;
		inline constexpr type close = ICON_FA_XMARK;
		inline constexpr type save = ICON_FA_FLOPPY_DISK;
		inline constexpr type copy = ICON_FA_COPY;
		inline constexpr type paste = ICON_FA_PASTE;
		inline constexpr type cut = ICON_FA_SCISSORS;
		inline constexpr type undo = ICON_FA_ROTATE_LEFT;
		inline constexpr type redo = ICON_FA_ROTATE_RIGHT;
		inline constexpr type rebuild = ICON_FA_ROTATE_RIGHT;
		inline constexpr type search = ICON_FA_MAGNIFYING_GLASS;
		inline constexpr type swap = ICON_FA_ARROW_RIGHT_ARROW_LEFT;
		inline constexpr type send = ICON_FA_PAPER_PLANE;
		inline constexpr type eye = ICON_FA_EYE;
		inline constexpr type eye_slash = ICON_FA_EYE_SLASH;
		inline constexpr type link = ICON_FA_LINK;
		inline constexpr type unlink = ICON_FA_LINK_SLASH;

		// Arrows
		inline constexpr type arrow_right = ICON_FA_ARROW_RIGHT;
		inline constexpr type arrow_left = ICON_FA_ARROW_LEFT;
		inline constexpr type arrow_up = ICON_FA_ARROW_UP;
		inline constexpr type arrow_down = ICON_FA_ARROW_DOWN;
		inline constexpr type circle_left = ICON_FA_CIRCLE_LEFT;
		inline constexpr type circle_right = ICON_FA_CIRCLE_RIGHT;

		// Status
		inline constexpr type warning = ICON_FA_TRIANGLE_EXCLAMATION;
		inline constexpr type info = ICON_FA_CIRCLE_INFO;
		inline constexpr type circle_check = ICON_FA_CIRCLE_CHECK;
		inline constexpr type circle_xmark = ICON_FA_CIRCLE_XMARK;
		inline constexpr type bug = ICON_FA_BUG;

		// Files/folders
		inline constexpr type folder = ICON_FA_FOLDER;
		inline constexpr type folder_open = ICON_FA_FOLDER_OPEN;
		inline constexpr type file = ICON_FA_FILE;
		inline constexpr type file_code = ICON_FA_FILE_CODE;
		inline constexpr type file_lines = ICON_FA_FILE_LINES;

		// Domain objects
		inline constexpr type machine = ICON_FA_SERVER;
		inline constexpr type controller = ICON_FA_GEAR;
		inline constexpr type element = ICON_FA_MICROCHIP;
		inline constexpr type function = ICON_FA_CODE;
		inline constexpr type driver = ICON_FA_GEARS;
		inline constexpr type plug = ICON_FA_SQUARE_PLUS;
		inline constexpr type socket = ICON_FA_SQUARE_MINUS;
		inline constexpr type signal = ICON_FA_BOLT;
		inline constexpr type network = ICON_FA_DIAGRAM_PROJECT;
		inline constexpr type dashboard = ICON_FA_CHART_LINE;
		inline constexpr type panel = ICON_FA_SLIDERS;
		inline constexpr type network_wired = ICON_FA_NETWORK_WIRED;

		// Misc
		inline constexpr type settings = ICON_FA_GEAR;
		inline constexpr type wifi = ICON_FA_WIFI;
		inline constexpr type terminal = ICON_FA_TERMINAL;
		inline constexpr type clock = ICON_FA_CLOCK;
		inline constexpr type memory = ICON_FA_MEMORY;
		inline constexpr type database = ICON_FA_DATABASE;
		inline constexpr type calculator = ICON_FA_CALCULATOR;

		// Buttons (icon + text, ready to use)
		inline constexpr type remove = ICON_FA_TRASH " Delete";
		inline constexpr type insert = ICON_FA_PLUS " Insert";
		inline constexpr type build = ICON_FA_HAMMER " Build";
		inline constexpr type run = ICON_FA_PLAY " Run";
		inline constexpr type halt = ICON_FA_STOP " Stop";
		inline constexpr type expand = ICON_FA_EXPAND " Expand";
		inline constexpr type collapse = ICON_FA_COMPRESS " Collapse";
		inline constexpr type back = ICON_FA_CIRCLE_LEFT " Back";
		inline constexpr type forward = ICON_FA_CIRCLE_RIGHT " Forward";
		inline constexpr type constants = ICON_FA_CALCULATOR " Constants";
		inline constexpr type funcs = ICON_FA_CODE " Functions";
		inline constexpr type edit = ICON_FA_PENCIL " Edit";
	}
}
