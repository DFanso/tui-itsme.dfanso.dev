#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderContact() {
  Elements rows = {heading("Contact Information")};
  for (const auto& link : data::contactLinks()) {
    Elements cells = {branch(true), t("[" + link.id + "] ", Tone::Yellow)};
    if (link.url) {
      cells.push_back(t(link.name, Tone::Blue));
      if (*link.url != "mailto:" + link.name) cells.push_back(t("  " + *link.url, Tone::Muted));
    } else {
      cells.push_back(t(link.name, Tone::Fg));
    }
    rows.push_back(hbox(std::move(cells)));
  }
  rows.push_back(blank());
  rows.push_back(hbox({t("Note: ", Tone::Green), t("Select a URL in your terminal to copy it", Tone::Muted)}));
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
