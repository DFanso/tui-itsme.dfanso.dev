#include "app/TitleBar.hpp"

#include "outputs/Common.hpp"
#include "outputs/Theme.hpp"

namespace itsme::app {
using namespace ftxui;
using core::Tone;
using outputs::t;

Element renderTitleBar(const std::string& clock) {
  Element buttons = hbox({t("● ", Tone::Red), t("● ", Tone::Yellow), t("●", Tone::Green)});
  return vbox({
      hbox({text(" "), buttons, filler(), t("guest@dfanso.dev:~", Tone::Fg), filler(), t(clock, Tone::Muted),
            text(" ")}),
      separator() | color(outputs::tone(Tone::Muted)),
  });
}

}  // namespace itsme::app
