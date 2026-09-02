#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderExperience() {
  const auto& companies = data::companies();
  Elements rows = {heading("Experience History")};
  for (std::size_t ci = 0; ci < companies.size(); ++ci) {
    const auto& company = companies[ci];
    rows.push_back(hbox({branch(ci + 1 == companies.size()), text(company.company) | color(tone(Tone::Blue)) | bold,
                         t(" · ", Tone::Muted), t(company.totalPeriod, Tone::Muted)}));
    for (std::size_t ri = 0; ri < company.roles.size(); ++ri) {
      const auto& role = company.roles[ri];
      rows.push_back(indent(hbox({t(ri + 1 == company.roles.size() ? "└─ " : "├─ ", Tone::Muted),
                                  t("[" + role.type + "] ", role.typeTone),
                                  text(role.title) | color(tone(Tone::Green)) | bold}),
                            4));
      rows.push_back(indent(t(role.period, Tone::Muted), 8));
      for (const auto& resp : role.responsibilities)
        rows.push_back(indent(hbox({t("│ ", Tone::Muted), para(resp, Tone::Fg) | flex}), 8));
      rows.push_back(indent(tagRow(role.tech), 8));
      rows.push_back(blank());
    }
  }
  rows.pop_back();
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
