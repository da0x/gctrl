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

#include "text_editor.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <imgui.h>
#include "ui/icons.hpp"

namespace ui {
    namespace code {

        inline const std::vector<std::pair<std::string, std::string>> constants = {
            {"e", "Euler's Number"},
            {"log2e", "Log base 2 of e"},
            {"log10e", "Log base 10 of e"},
            {"pi", "Pi (π)"},
            {"inv_pi", "1 / Pi (1/π)"},
            {"inv_sqrtpi", "1 / √Pi (1/√π)"},
            {"ln2", "Natural Logarithm of 2 (ln 2)"},
            {"ln10", "Natural Logarithm of 10 (ln 10)"},
            {"sqrt2", "Square root of 2 (√2)"},
            {"sqrt3", "Square root of 3 (√3)"},
            {"inv_sqrt3", "1 / Square root of 3 (1/√3)"},
            {"egamma", "Euler-Mascheroni constant (γ)"},
            {"phi", "The golden ratio (Φ)"}
        };

        inline const std::vector<std::pair<std::string, std::vector<std::pair<std::string, std::string>>>> functions = {
            {"Trigonometric Functions", {
                {"cos(angle)", "Compute cosine of an angle in radians"},
                {"sin(angle)", "Compute sine of an angle in radians"},
                {"tan(angle)", "Compute tangent of an angle in radians"},
                {"acos(value)", "Compute arc cosine (inverse cosine)"},
                {"asin(value)", "Compute arc sine (inverse sine)"},
                {"atan(value)", "Compute arc tangent (inverse tangent)"},
                {"atan2(y, x)", "Compute arc tangent using two coordinates (y, x)"}
            }},
            {"Hyperbolic Functions", {
                {"cosh(angle)", "Compute hyperbolic cosine of an angle"},
                {"sinh(angle)", "Compute hyperbolic sine of an angle"},
                {"tanh(angle)", "Compute hyperbolic tangent of an angle"},
                {"acosh(value)", "Compute inverse hyperbolic cosine"},
                {"asinh(value)", "Compute inverse hyperbolic sine"},
                {"atanh(value)", "Compute inverse hyperbolic tangent"}
            }},
            {"Exponential and Logarithmic Functions", {
                {"exp(exponent)", "Compute exponential function e^exponent"},
                {"frexp(value, &exp)", "Break value into significand and exponent"},
                {"ldexp(mantissa, exp)", "Generate value from mantissa and exponent"},
                {"log(value)", "Compute natural logarithm (base e)"},
                {"log10(value)", "Compute common logarithm (base 10)"},
                {"mod(value, &int_part)", "Break value into fractional and integral parts"},
                {"exp2(exponent)", "Compute binary exponential function 2^exponent"},
                {"expm1(value)", "Compute e^value - 1 efficiently"},
                {"ilogb(value)", "Compute integer binary logarithm of value"},
                {"log1p(value)", "Compute natural logarithm of 1 + value"},
                {"log2(value)", "Compute binary logarithm (base 2)"},
                {"logb(value)", "Compute floating-point base logarithm"},
                {"scalbn(value, exp)", "Scale value by power of two (value * 2^exp)"},
                {"scalbln(value, exp)", "Scale value by power of two (long version)"}
            }},
            {"Power Functions", {
                {"pow(base, exponent)", "Raise base to the power of exponent"},
                {"sqrt(value)", "Compute square root of value"},
                {"cbrt(value)", "Compute cubic root of value"},
                {"hypot(x, y)", "Compute the hypotenuse given sides x and y"}
            }},
            {"Error and Gamma Functions", {
                {"erf(value)", "Compute error function of value"},
                {"erfc(value)", "Compute complementary error function"},
                {"tgamma(value)", "Compute gamma function of value"},
                {"lgamma(value)", "Compute natural logarithm of the gamma function"}
            }},
            {"Rounding and Remainder Functions", {
                {"ceil(value)", "Round value up to the nearest integer"},
                {"floor(value)", "Round value down to the nearest integer"},
                {"fmod(dividend, divisor)", "Compute remainder of division"},
                {"trunc(value)", "Truncate value to its integral part"},
                {"round(value)", "Round value to nearest integer"},
                {"lround(value)", "Round value to nearest integer and cast to long"},
                {"llround(value)", "Round value to nearest integer and cast to long long"},
                {"rint(value)", "Round value to nearest integer according to current rounding mode"},
                {"lrint(value)", "Round value and cast to long integer"},
                {"llrint(value)", "Round value and cast to long long integer"},
                {"nearbyint(value)", "Round value to nearest integer without raising exceptions"},
                {"remainder(numerator, denominator)", "Compute remainder using IEC 60559 rules"},
                {"remquo(numerator, denominator, &quotient)", "Compute remainder and store quotient part"}
            }},
            {"Floating-Point Manipulation Functions", {
                {"copysign(magnitude, sign)", "Copy sign from sign to magnitude"},
                {"nan(\"tag\")", "Generate quiet NaN with a specific tag"},
                {"nextafter(value, next)", "Get next representable value toward next"},
                {"nexttoward(value, next)", "Get next representable value toward long double next"}
            }},
            {"Minimum, Maximum, Difference Functions", {
                {"dim(value1, value2)", "Positive difference between value1 and value2"},
                {"max(value1, value2)", "Get maximum of value1 and value2"},
                {"min(value1, value2)", "Get minimum of value1 and value2"}
            }},
            {"Other Functions", {
                {"abs(value)", "Compute absolute value of a floating-point number"},
                {"abs(value)", "Compute absolute value of an integer"},
                {"ma(multiplicand, multiplier, addend)", "Compute (multiplicand * multiplier) + addend"}
            }}
        };

        inline std::unordered_map<unsigned int, TextEditor> editor_map;

        inline void code_editor(std::string& content) {
            unsigned int current_id = ui::id::get("");

            if (editor_map.find(current_id) == editor_map.end()) {
                editor_map[current_id].SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
                editor_map[current_id].SetText(content);
            }

            TextEditor& editor = editor_map[current_id];

            if (ui::button(ui::icon::constants)) {
                ui::popup::open("Constants Menu");
            }

            if (ui::popup::begin("Constants Menu")) {
                for (const auto& constant : constants) {
                    if (ui::menu::item(constant.second)) {
                        editor.InsertText(constant.first);
                    }
                }
                ui::popup::end();
            }
            ui::same_line();
            if (ui::button(ui::icon::funcs)) {
                ui::popup::open("Functions Menu");
            }

            if (ui::popup::begin("Functions Menu")) {
                for (const auto& group : functions) {
                    if (ui::menu::begin(group.first.c_str())) {
                        for (const auto& func : group.second) {
                            if (ui::menu::item(func.second.c_str())) {
                                editor.InsertText(func.first.c_str());
                            }
                        }
                        ui::menu::end();
                    }
                }
                ui::popup::end();
            }

            if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                ui::popup::open("Constants Menu");
            }

            ui::font::push(ui::font::type::code);
            editor.Render(std::to_string(current_id).c_str());
            ui::font::pop();
            content = editor.GetText();
        }

        inline void read_only_editor(const std::string& header_content) {
            TextEditor editor;
            editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
            editor.SetText(header_content);
            editor.SetReadOnly(true);
            editor.Render("Generated Header File (Read-Only)");
        }

    }  // namespace code
}  // namespace ui
