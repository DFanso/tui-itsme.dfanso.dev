#include "outputs/Theme.hpp"

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <string>

namespace itsme::outputs {
using ftxui::Color;

namespace {
bool g_trueColor = true;

Color pick(Color rgb, Color fallback16) { return g_trueColor ? rgb : fallback16; }
}  // namespace

void setTrueColor(bool enabled) { g_trueColor = enabled; }
bool trueColor() { return g_trueColor; }

Color tone(core::Tone t) {
  using core::Tone;
  switch (t) {
    case Tone::Fg:
      return pick(Color::RGB(0xc0, 0xca, 0xf5), Color::White);
    case Tone::Muted:
      return pick(Color::RGB(0xa9, 0xb1, 0xd6), Color::GrayLight);
    case Tone::Blue:
      return pick(Color::RGB(0x7a, 0xa2, 0xf7), Color::Blue);
    case Tone::Purple:
      return pick(Color::RGB(0xbb, 0x9a, 0xf7), Color::Magenta);
    case Tone::Green:
      return pick(Color::RGB(0x9e, 0xce, 0x6a), Color::Green);
    case Tone::Red:
      return pick(Color::RGB(0xf7, 0x76, 0x8e), Color::Red);
    case Tone::Yellow:
      return pick(Color::RGB(0xe0, 0xaf, 0x68), Color::Yellow);
    case Tone::Cyan:
      return pick(Color::RGB(0x7d, 0xcf, 0xff), Color::Cyan);
    case Tone::Teal:
      return pick(Color::RGB(0x73, 0xda, 0xca), Color::CyanLight);
    case Tone::Orange:
      return pick(Color::RGB(0xff, 0x9e, 0x64), Color::YellowLight);
  }
  return Color::Default;
}

Color hexColor(std::string_view hex, Color fallback) {
  if (!g_trueColor || hex.size() != 7 || hex[0] != '#') return fallback;
  for (std::size_t i = 1; i < 7; ++i)
    if (!std::isxdigit(static_cast<unsigned char>(hex[i]))) return fallback;
  auto byte = [&](std::size_t pos) {
    return static_cast<std::uint8_t>(std::strtol(std::string(hex.substr(pos, 2)).c_str(), nullptr, 16));
  };
  return Color::RGB(byte(1), byte(3), byte(5));
}

Color matrixGreen() { return pick(Color::RGB(0x00, 0xff, 0x41), Color::GreenLight); }
Color bgColor() { return pick(Color::RGB(0x1a, 0x1b, 0x26), Color::Black); }

}  // namespace itsme::outputs
