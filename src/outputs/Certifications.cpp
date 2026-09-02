#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderCertifications() {
  Elements rows = {heading("Certifications")};
  for (const auto& c : data::certifications()) {
    rows.push_back(hbox({branch(true), t(c.name, Tone::Yellow)}));
    Elements meta = {t(c.issuer, Tone::Green)};
    if (!c.id.empty()) {
      meta.push_back(t(" | ", Tone::Muted));
      meta.push_back(t(c.id, Tone::Muted));
    }
    rows.push_back(indent(hbox(std::move(meta)), 4));
    const std::string dates = c.date + (c.expiry.empty() ? "" : " • " + c.expiry);
    rows.push_back(indent(t(dates, Tone::Muted), 4));
    rows.push_back(blank());
  }
  rows.pop_back();
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
