#include "outputs/GitHubStats.hpp"

#include <algorithm>

#include "outputs/Common.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

const char* const kSpinnerFrames[10] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};

namespace {
Element stat(long value, const char* label) {
  return vbox({text(std::to_string(value)) | color(tone(Tone::Fg)) | bold, t(label, Tone::Muted)}) |
         size(WIDTH, EQUAL, 16);
}

// Intensity level 0-4, scaled to the busiest day like GitHub's own graph.
int heatLevel(int count, int maxCount) {
  if (count <= 0) return 0;
  if (maxCount <= 0) return 4;
  const double ratio = static_cast<double>(count) / maxCount;
  return ratio <= 0.25 ? 1 : ratio <= 0.5 ? 2 : ratio <= 0.75 ? 3 : 4;
}

Color heatColor(int level) {
  switch (level) {
    case 0: return trueColor() ? Color::RGB(0x2a, 0x2c, 0x3a) : Color::GrayDark;
    case 1: return hexColor("#0e4429", Color::Green);
    case 2: return hexColor("#006d32", Color::Green);
    case 3: return hexColor("#26a641", Color::GreenLight);
    default: return hexColor("#39d353", Color::GreenLight);
  }
}

Element languageBar(const std::vector<github::LanguageStat>& langs) {
  constexpr int kWidth = 40;
  Elements segments;
  int used = 0;
  for (std::size_t i = 0; i < langs.size(); ++i) {
    int cells = (i + 1 == langs.size()) ? kWidth - used : std::max(1, kWidth * langs[i].percentage / 100);
    cells = std::max(0, std::min(cells, kWidth - used));
    used += cells;
    std::string bar;
    for (int c = 0; c < cells; ++c) bar += "█";
    segments.push_back(text(bar) | color(hexColor(langs[i].color)));
  }
  Elements legend;
  for (const auto& l : langs)
    legend.push_back(hbox({text("● ") | color(hexColor(l.color)),
                           t(l.name + " " + std::to_string(l.percentage) + "%  ", Tone::Fg)}));
  return vbox({hbox(std::move(segments)), hflow(std::move(legend))});
}

Element heatmap(const github::ContributionCalendar& cal, int width) {
  const int maxWeeks = std::max(0, std::min<int>(static_cast<int>(cal.weeks.size()), width - 6));
  const std::size_t first = cal.weeks.size() - static_cast<std::size_t>(maxWeeks);
  int maxCount = 0;
  for (const auto& week : cal.weeks)
    for (const auto& d : week.days) maxCount = std::max(maxCount, d.count);
  Elements rows;
  for (int weekday = 0; weekday < 7; ++weekday) {
    Elements cells;
    for (std::size_t w = first; w < cal.weeks.size(); ++w) {
      int count = -1;
      for (const auto& d : cal.weeks[w].days)
        if (d.weekday == weekday) count = d.count;
      cells.push_back(count < 0 ? text(" ") : text("■") | color(heatColor(heatLevel(count, maxCount))));
    }
    rows.push_back(hbox(std::move(cells)));
  }
  Elements legend = {t("Less ", Tone::Muted)};
  for (int level : {0, 1, 2, 3, 4}) legend.push_back(text("■") | color(heatColor(level)));
  legend.push_back(t(" More", Tone::Muted));
  rows.push_back(hbox(std::move(legend)));
  return vbox(std::move(rows));
}
}  // namespace

Element renderGitHubStats(const GitHubView& view, const RenderContext& ctx) {
  Elements rows = {heading("GitHub Statistics")};
  if (view.status == GitHubView::Status::Loading) {
    rows.push_back(hbox({t(std::string(kSpinnerFrames[view.spinnerFrame % 10]) + " ", Tone::Blue),
                         t("Fetching GitHub stats...", Tone::Muted)}));
    return vbox(std::move(rows));
  }
  if (view.status == GitHubView::Status::Failed || !view.data) {
    rows.push_back(hbox(
        {branch(true), t("Unable to fetch GitHub data. API may be rate limited or unreachable.", Tone::Red)}));
    return vbox(std::move(rows));
  }
  const auto& d = *view.data;
  const auto& s = d.stats;

  Elements who = {t(std::string("@") + github::kUsername, Tone::Blue)};
  if (d.hasFullData) who.push_back(t("  (Last Year)", Tone::Green));
  rows.push_back(hbox(std::move(who)));
  rows.push_back(blank());

  Elements grid1, grid2;
  if (d.hasFullData) {
    grid1 = {stat(s.commits, "Commits"), stat(s.prs, "Pull Requests"), stat(s.issues, "Issues"),
             stat(s.contributions, "Contributions")};
  }
  grid2 = {stat(s.stars, "Total Stars"), stat(s.forks, "Total Forks"), stat(s.repos, "Repositories"),
           stat(s.followers, "Followers")};
  if (!grid1.empty()) {
    rows.push_back(indent(hbox(std::move(grid1)), 4));
    rows.push_back(blank());
  }
  rows.push_back(indent(hbox(std::move(grid2)), 4));
  rows.push_back(blank());

  if (!d.languageStats.empty()) {
    rows.push_back(hbox({branch(false), t("cat ", Tone::Yellow), t("languages", Tone::Blue)}));
    rows.push_back(indent(languageBar(d.languageStats), 4));
    rows.push_back(blank());
  }

  if (d.calendar && ctx.width >= 80) {
    rows.push_back(hbox({branch(false), t("cat ", Tone::Yellow), t("contributions.heatmap", Tone::Blue)}));
    rows.push_back(indent(heatmap(*d.calendar, ctx.width - 8), 4));
    rows.push_back(indent(t(std::to_string(s.contributions) + " contributions this year", Tone::Green), 4));
    rows.push_back(blank());
  }

  rows.push_back(hbox({branch(true), t("cat ", Tone::Yellow), t(d.isPinned ? "pinned-repos" : "top-repos", Tone::Blue)}));
  for (const auto& r : d.topRepos) {
    Elements line = {t(r.name, Tone::Blue) | bold};
    if (r.language) {
      line.push_back(text("  ● ") | color(hexColor(r.languageColor.value_or("#8b8b8b"))));
      line.push_back(t(*r.language, Tone::Muted));
    }
    line.push_back(t("  ★ " + std::to_string(r.stars), Tone::Yellow));
    line.push_back(t("  ⑂ " + std::to_string(r.forks), Tone::Cyan));
    rows.push_back(indent(hbox(std::move(line)), 4));
    if (r.description && !r.description->empty()) rows.push_back(indent(para(*r.description, Tone::Muted), 6));
    if (!r.url.empty()) rows.push_back(indent(t(r.url, Tone::Muted) | dim, 6));
  }
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
