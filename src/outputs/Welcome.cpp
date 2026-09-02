#include <array>

#include "core/Strings.hpp"
#include "core/Version.hpp"
#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

namespace {
constexpr std::array<const char*, 6> kBanner = {
    " ██████╗ ███████╗ █████╗ ███╗   ██╗███████╗ ██████╗ ",
    " ██╔══██╗██╔════╝██╔══██╗████╗  ██║██╔════╝██╔═══██╗",
    " ██║  ██║█████╗  ███████║██╔██╗ ██║███████╗██║   ██║",
    " ██║  ██║██╔══╝  ██╔══██║██║╚██╗██║╚════██║██║   ██║",
    " ██████╔╝██║     ██║  ██║██║ ╚████║███████║╚██████╔╝",
    " ╚═════╝ ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═══╝╚══════╝ ╚═════╝ ",
};

std::string greeting(int hour) {
  if (hour < 12) return "Good morning, visitor!";
  if (hour < 18) return "Good afternoon, visitor!";
  return "Good evening, visitor!";
}

struct Line {
  std::string text;
  Tone tone;
};

std::vector<Line> textLines(int hour) {
  return {
      {greeting(hour), Tone::Green},
      {data::profile().tagline, Tone::Muted},
      {"────────────────────────────────────", Tone::Muted},
      {"Welcome to my terminal portfolio. (TUI v" + std::string(itsme::version()) + ")", Tone::Fg},
      {"Type 'help' to see available commands, or try 'github' for stats.", Tone::Muted},
  };
}
}  // namespace

int welcomeTypewriterLength() {
  int n = 0;
  for (const auto& l : textLines(12)) n += static_cast<int>(core::utf8Chars(l.text).size());
  return n;
}

Element renderWelcome(const RenderContext& ctx, int revealed) {
  Elements rows;
  if (ctx.width >= 60) {
    for (const char* row : kBanner) rows.push_back(text(row) | color(tone(Tone::Blue)) | bold);
  } else {
    rows.push_back(text("DFANSO") | color(tone(Tone::Blue)) | bold);
  }
  rows.push_back(blank());

  int budget = revealed;
  for (const auto& l : textLines(ctx.hour)) {
    if (revealed < 0) {
      rows.push_back(t(l.text, l.tone));
      continue;
    }
    if (budget <= 0) break;
    auto chars = core::utf8Chars(l.text);
    std::string shown;
    const int take = static_cast<int>(chars.size()) < budget ? static_cast<int>(chars.size()) : budget;
    for (int i = 0; i < take; ++i) shown += chars[static_cast<std::size_t>(i)];
    budget -= take;
    rows.push_back(t(shown, l.tone));
  }
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
