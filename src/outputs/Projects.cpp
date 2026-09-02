#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderProjects(const github::ProjectStatsMap* stats) {
  const auto& projects = data::projects();
  Elements rows = {heading("Featured Projects")};
  for (std::size_t i = 0; i < projects.size(); ++i) {
    const auto& p = projects[i];
    Elements head = {branch(i + 1 == projects.size()), t("cat ", Tone::Yellow),
                     t("projects/" + p.name + "/ ", Tone::Blue), t("type: ", Tone::Muted), t(p.type, Tone::Red)};
    if (stats && p.github) {
      auto it = stats->find(*p.github);
      if (it != stats->end() && it->second) {
        const auto& s = *it->second;
        head.push_back(t("   ★ " + std::to_string(s.stars), Tone::Yellow));
        head.push_back(t("  ⑂ " + std::to_string(s.forks), Tone::Cyan));
        head.push_back(t("  ◉ " + std::to_string(s.watchers), Tone::Purple));  // single-width "eye"
      }
    }
    rows.push_back(hbox(std::move(head)));
    rows.push_back(indent(para(p.description, Tone::Fg) | flex, 4));
    rows.push_back(indent(tagRow(p.tech), 4));
    rows.push_back(indent(t(p.url, Tone::Muted), 4));
    if (i + 1 < projects.size()) rows.push_back(blank());
  }
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
