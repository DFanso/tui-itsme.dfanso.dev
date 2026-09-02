#pragma once
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace itsme::github {

inline constexpr const char* kUsername = "DFanso";

struct Stats {
  long commits = 0, prs = 0, issues = 0, repos = 0, stars = 0, forks = 0, followers = 0, contributions = 0;
};

struct LanguageStat {
  std::string name;
  int percentage = 0;
  std::string color;  // "#rrggbb"
};

struct TopRepo {
  std::string name;
  std::string url;
  std::optional<std::string> description;
  std::optional<std::string> language;
  std::optional<std::string> languageColor;
  long stars = 0;
  long forks = 0;
};

struct ContributionDay {
  int count = 0;
  std::string date;
  int weekday = 0;
};
struct ContributionWeek {
  std::vector<ContributionDay> days;
};
struct ContributionCalendar {
  long total = 0;
  std::vector<ContributionWeek> weeks;
};

struct GitHubStatsData {
  bool hasFullData = false;
  Stats stats;
  std::vector<LanguageStat> languageStats;
  std::vector<TopRepo> topRepos;
  bool isPinned = false;
  std::optional<ContributionCalendar> calendar;
};

struct ProjectRepoStats {
  long stars = 0, forks = 0, watchers = 0;
};
using ProjectStatsMap = std::map<std::string, std::optional<ProjectRepoStats>>;

}  // namespace itsme::github
