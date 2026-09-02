#include "app/Prompt.hpp"

#include "outputs/Common.hpp"

namespace itsme::app {
using namespace ftxui;
using core::Tone;
using outputs::t;

Element renderPrompt(bool awaitingProjectResponse) {
  if (awaitingProjectResponse)
    return hbox(
        {t("❯ ", Tone::Green), t("Would you like to see more projects? ", Tone::Fg), t("(y/n) ", Tone::Muted)});
  return hbox({t("❯ ", Tone::Green), t("dfanso", Tone::Blue), t("@", Tone::Muted), t("terminal", Tone::Purple),
               t(" in ", Tone::Muted), t("~/portfolio", Tone::Yellow), t(" on ", Tone::Muted),
               t("main ", Tone::Red)});
}

}  // namespace itsme::app
