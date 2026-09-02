#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderSkills() {
  const auto& cats = data::skillCategories();
  Elements rows = {heading("Technical Skills")};
  for (std::size_t i = 0; i < cats.size(); ++i) {
    rows.push_back(
        hbox({branch(i + 1 == cats.size()), t("ls ", Tone::Yellow), t("~/" + cats[i].name + "/", Tone::Blue)}));
    rows.push_back(indent(tagRow(cats[i].skills), 4));
    if (i + 1 < cats.size()) rows.push_back(blank());
  }
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
