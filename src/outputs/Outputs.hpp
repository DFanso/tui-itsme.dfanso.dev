#pragma once
#include <ftxui/dom/elements.hpp>
#include <map>
#include <optional>
#include <string>

#include "core/Command.hpp"

namespace itsme::outputs {

struct RenderContext {
  int width = 80;
  int hour = 12;  // local hour 0-23, drives the welcome greeting
};

ftxui::Element renderText(const core::TextOutput& out);

// revealed = number of code points of the text lines shown (-1 = everything).
ftxui::Element renderWelcome(const RenderContext& ctx, int revealed = -1);
int welcomeTypewriterLength();

ftxui::Element renderWhoami();
ftxui::Element renderAbout();
ftxui::Element renderEducation();
ftxui::Element renderCertifications();
ftxui::Element renderContact();
ftxui::Element renderWeather();
ftxui::Element renderPing();
ftxui::Element renderTime(const std::string& timeString);

}  // namespace itsme::outputs
