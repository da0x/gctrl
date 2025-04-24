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

#include "ui/ui.hpp"

namespace ui {
    namespace instance {

        template <typename T>
        inline void title(const T& i) {
            ui::font::push(ui::font::type::code);

            ui::text(i.icon());
            ui::same_line();

            ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_typename);
            ui::text(i.prototype_typename());
            ui::style::color::pop();
            ui::same_line();

            ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_instance);
            ui::text(i.instance_name());
            ui::style::color::pop();

            ui::font::pop();
        }

        template <typename T>
        struct title_functor {
            const T& instance;

            title_functor(const T& i)
                : instance(i) {}

            void operator()() const {

                ui::font::push(ui::font::type::code);

                ui::text(instance.icon());
                ui::same_line();

                ui::style::color::push(ui::colors::text, ui::theme::vs2022::code_instance);
                ui::text(instance.instance_name());
                ui::style::color::pop();

                ui::font::pop();
            }
        };
        
    } // namespace instance
}
