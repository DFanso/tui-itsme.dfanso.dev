#pragma once
#include <ftxui/screen/color.hpp>
#include <string_view>

#include "core/Command.hpp"

namespace itsme::outputs {

void setTrueColor(bool enabled);
bool trueColor();

ftxui::Color tone(core::Tone t);
// "#rrggbb" -> Color. Falls back to `fallback` on bad input or when true color is off.
ftxui::Color hexColor(std::string_view hex, ftxui::Color fallback = ftxui::Color::GrayLight);
ftxui::Color matrixGreen();  // #00ff41
ftxui::Color bgColor();      // #1a1b26

}  // namespace itsme::outputs
