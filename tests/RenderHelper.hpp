#pragma once
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>
#include <string>

inline std::string stripAnsi(const std::string& in) {
  std::string out;
  for (std::size_t i = 0; i < in.size();) {
    if (in[i] == '\x1b') {
      while (i < in.size() && in[i] != 'm') ++i;
      ++i;
    } else {
      out += in[i++];
    }
  }
  return out;
}

inline std::string renderPlain(ftxui::Element e, int w = 100, int h = 40) {
  auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(w), ftxui::Dimension::Fixed(h));
  ftxui::Render(screen, e);
  return stripAnsi(screen.ToString());
}
