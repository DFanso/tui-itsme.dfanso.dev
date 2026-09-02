#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderWeather() {
  return vbox({
      t("Weather information is not available in the terminal.", Tone::Blue),
      t("Try looking outside your window! 🌤️", Tone::Muted),
  });
}

Element renderPing() {
  auto line = [](const char* seq, const char* ms) {
    return hbox({t("64 bytes from dfanso.dev", Tone::Blue),
                 t(std::string(" : icmp_seq=") + seq + " ttl=64 time= ", Tone::Muted), t(ms, Tone::Green)});
  };
  return vbox({
      heading("PING dfanso.dev (192.168.1.1)"),
      line("1", "0.045 ms"),
      line("2", "0.038 ms"),
      line("3", "0.042 ms"),
      line("4", "0.039 ms"),
      blank(),
      hbox({t("--- ", Tone::Muted), t("dfanso.dev ping statistics ", Tone::Fg), t("---", Tone::Muted)}),
      t("4 packets transmitted, 4 received, 0% packet loss, time 3ms", Tone::Fg),
      t("rtt min/avg/max = 0.038/0.041/0.045 ms", Tone::Fg),
  });
}

Element renderTime(const std::string& timeString) { return t(timeString, Tone::Green); }

}  // namespace itsme::outputs
