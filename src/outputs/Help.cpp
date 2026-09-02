#include "core/Commands.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderHelp() {
  Elements rows = {
      heading("Terminal Portfolio Help"),
      blank(),
      t("USAGE:", Tone::Blue),
      indent(t("command [arguments]", Tone::Fg), 4),
      indent(t("Tip: press Tab to autocomplete a command.", Tone::Muted), 4),
      blank(),
      t("AVAILABLE COMMANDS:", Tone::Blue),
  };
  for (const auto& c : core::commands()) {
    if (c.hidden) continue;
    rows.push_back(
        indent(hbox({t(c.name, Tone::Yellow) | size(WIDTH, EQUAL, 16), t(c.description, Tone::Fg)}), 4));
  }
  rows.push_back(blank());
  rows.push_back(t("Use arrow keys ↑↓ to navigate command history", Tone::Muted));
  rows.push_back(t("Press Ctrl+L or type 'clear' to clear screen", Tone::Muted));
  rows.push_back(t("Press Ctrl+C or Ctrl+D to exit", Tone::Muted));
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
