#include "core/Version.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderNeofetch() {
  Elements art;
  for (const char* row : {"                  ▄▄▄▄▄▄▄▄▄▄▄", "                ▄▀█▀█▀█▀█▀█▀█▀▄",
                          "               █▀█▀█▀█▀█▀█▀█▀█▀█", "              ▄█▀█▀█▀█▀█▀█▀█▀█▀█▄",
                          "             ▀▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▀"})
    art.push_back(t(row, Tone::Blue));

  auto kv = [](const char* k, std::string v) {
    return hbox({t(k, Tone::Blue), text(" "), t(std::move(v), Tone::Fg)});
  };
  Element info = vbox({
      kv("OS:", std::string("Portfolio TUI v") + itsme::version()),
      kv("Host:", "dfanso.dev"),
      kv("Kernel:", "DevOps 5.0.1"),
      kv("Uptime:", "24/7"),
      kv("Shell:", "Portfolio-CLI"),
      kv("IDE:", "VS Code / Neovim"),
  });
  return hbox({vbox(std::move(art)), text("    "), info});
}

}  // namespace itsme::outputs
