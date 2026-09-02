#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderEducation() {
  Elements rows = {heading("Education")};
  for (const auto& e : data::education()) {
    rows.push_back(hbox({branch(true), t(e.degree, Tone::Yellow)}));
    Elements meta = {t(e.institution, Tone::Green), t(" | ", Tone::Muted), t(e.location, Tone::Fg)};
    if (!e.grade.empty()) {
      meta.push_back(t(" | ", Tone::Muted));
      meta.push_back(t(e.grade, Tone::Red));
    }
    rows.push_back(indent(hbox(std::move(meta)), 4));
    rows.push_back(indent(t(e.period, Tone::Muted), 4));
    rows.push_back(blank());
  }
  rows.pop_back();
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
