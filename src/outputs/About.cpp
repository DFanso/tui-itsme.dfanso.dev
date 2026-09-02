#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderAbout() {
  auto row = [](Element e) {
    return hbox({text("  "), text("│ ") | color(tone(Tone::Muted)), std::move(e) | flex});
  };

  Element focus = hflow({
      text("DevOps Pipelines  ") | color(tone(Tone::Blue)),
      text("Cloud Infrastructure  ") | color(tone(Tone::Green)),
      text("Backend  ") | color(tone(Tone::Purple)),
      text("AI Automation") | color(tone(Tone::Yellow)),
  });

  return vbox({
      heading("Professional Summary"),
      row(richPara({{"Senior Software Engineer", Tone::Blue},
                    {"at", Tone::Fg},
                    {"CD Extreme OPC,", Tone::Blue},
                    {"holding a", Tone::Fg},
                    {"First-Class Honours", Tone::Yellow},
                    {"degree from the", Tone::Fg},
                    {"University of Plymouth.", Tone::Green},
                    {"Experienced across DevOps pipelines, backend development, cloud infrastructure, and AI-driven "
                     "automation.",
                     Tone::Fg}})),
      row(blank()),
      row(richPara({{"Co-Founder & CTO", Tone::Purple},
                    {"of", Tone::Fg},
                    {"CodeXeed", Tone::Blue},
                    {"and", Tone::Fg},
                    {"KlexD,", Tone::Blue},
                    {"building scalable cloud-native applications and intelligent systems for global clients.",
                     Tone::Fg}})),
      row(blank()),
      row(t("FOCUS", Tone::Muted)),
      row(focus),
      row(blank()),
      row(t("LOCATION", Tone::Muted)),
      row(hbox({text("📍 ") | color(tone(Tone::Red)), t("Sri Lanka (Open to Remote)", Tone::Fg)})),
  });
}

}  // namespace itsme::outputs
