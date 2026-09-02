#include "github/Parse.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <nlohmann/json.hpp>

namespace itsme::github {
using json = nlohmann::json;

const char* graphqlQuery() {
  return R"(query($username: String!) {
  user(login: $username) {
    name login avatarUrl bio
    followers { totalCount }
    following { totalCount }
    repositories(first: 100, ownerAffiliations: OWNER, orderBy: {field: STARGAZERS, direction: DESC}) {
      totalCount
      nodes { name stargazerCount forkCount primaryLanguage { name color } }
    }
    pinnedItems(first: 6, types: REPOSITORY) {
      nodes { ... on Repository { name description url stargazerCount forkCount primaryLanguage { name color } } }
    }
    contributionsCollection {
      totalCommitContributions totalPullRequestContributions totalIssueContributions totalRepositoryContributions
      contributionCalendar { totalContributions weeks { contributionDays { contributionCount date weekday } } }
    }
  }
})";
}

std::string languageColor(std::string_view language) {
  static const std::map<std::string, std::string, std::less<>> colors = {
      {"TypeScript", "#3178c6"}, {"JavaScript", "#f1e05a"}, {"Python", "#3572A5"}, {"Go", "#00ADD8"},
      {"Rust", "#dea584"},       {"Java", "#b07219"},       {"C#", "#178600"},     {"C++", "#f34b7d"},
      {"C", "#555555"},          {"PHP", "#4F5D95"},        {"Ruby", "#701516"},   {"Swift", "#F05138"},
      {"Kotlin", "#A97BFF"},     {"Dart", "#00B4AB"},       {"Shell", "#89e051"},  {"HTML", "#e34c26"},
      {"CSS", "#563d7c"},        {"Vue", "#41b883"},        {"Svelte", "#ff3e00"}, {"Astro", "#ff5a03"},
      {"HCL", "#844FBA"},        {"Dockerfile", "#384d54"},
  };
  auto it = colors.find(language);
  return it == colors.end() ? "#8b8b8b" : it->second;
}

namespace {

std::optional<std::string> optString(const json& j, const char* key) {
  if (!j.contains(key) || !j[key].is_string()) return std::nullopt;
  return j[key].get<std::string>();
}

long optLong(const json& j, const char* key) {
  if (!j.contains(key) || !j[key].is_number()) return 0;
  return j[key].get<long>();
}

struct LangInput {
  std::optional<std::string> name;
  std::optional<std::string> color;
};

// Port of calculateLanguageStats: count, keep first-seen order, stable sort desc, top 8.
std::vector<LanguageStat> languageStatsFrom(const std::vector<LangInput>& repos) {
  struct Acc {
    std::string name;
    int count;
    std::string color;
  };
  std::vector<Acc> acc;
  for (const auto& r : repos) {
    if (!r.name) continue;
    auto it = std::find_if(acc.begin(), acc.end(), [&](const Acc& a) { return a.name == *r.name; });
    if (it == acc.end())
      acc.push_back({*r.name, 1, r.color.value_or("#8b8b8b")});
    else
      ++it->count;
  }
  int total = 0;
  for (const auto& a : acc) total += a.count;
  std::stable_sort(acc.begin(), acc.end(), [](const Acc& a, const Acc& b) { return a.count > b.count; });
  if (acc.size() > 8) acc.resize(8);
  std::vector<LanguageStat> out;
  for (const auto& a : acc)
    out.push_back({a.name, total ? static_cast<int>(std::lround(100.0 * a.count / total)) : 0, a.color});
  return out;
}

}  // namespace

std::optional<GitHubStatsData> parseGraphQL(std::string_view body) {
  try {
    const json root = json::parse(body);
    if (!root.is_object() || !root.contains("data") || !root["data"].is_object() ||
        !root["data"].contains("user") || root["data"]["user"].is_null())
      return std::nullopt;
    const json& user = root["data"]["user"];

    GitHubStatsData d;
    d.hasFullData = true;

    const json& repoNodes = user.at("repositories").at("nodes");
    std::vector<LangInput> langs;
    for (const auto& r : repoNodes) {
      LangInput li;
      if (r.contains("primaryLanguage") && r["primaryLanguage"].is_object()) {
        li.name = optString(r["primaryLanguage"], "name");
        li.color = optString(r["primaryLanguage"], "color");
      }
      langs.push_back(li);
      d.stats.stars += optLong(r, "stargazerCount");
      d.stats.forks += optLong(r, "forkCount");
    }
    d.languageStats = languageStatsFrom(langs);

    const json& contrib = user.at("contributionsCollection");
    const json& cal = contrib.at("contributionCalendar");
    d.stats.commits = optLong(contrib, "totalCommitContributions");
    d.stats.prs = optLong(contrib, "totalPullRequestContributions");
    d.stats.issues = optLong(contrib, "totalIssueContributions");
    d.stats.repos = optLong(user.at("repositories"), "totalCount");
    d.stats.followers = optLong(user.at("followers"), "totalCount");
    d.stats.contributions = optLong(cal, "totalContributions");

    const json& pinned = user.at("pinnedItems").at("nodes");
    d.isPinned = !pinned.empty();
    const json& source = d.isPinned ? pinned : repoNodes;
    const std::size_t limit = d.isPinned ? source.size() : std::min<std::size_t>(6, source.size());
    for (std::size_t i = 0; i < limit; ++i) {
      const json& r = source[i];
      TopRepo tr;
      tr.name = optString(r, "name").value_or("");
      tr.url = optString(r, "url").value_or("");
      tr.description = optString(r, "description");
      if (r.contains("primaryLanguage") && r["primaryLanguage"].is_object()) {
        tr.language = optString(r["primaryLanguage"], "name");
        tr.languageColor = optString(r["primaryLanguage"], "color");
      }
      tr.stars = optLong(r, "stargazerCount");
      tr.forks = optLong(r, "forkCount");
      d.topRepos.push_back(std::move(tr));
    }

    ContributionCalendar c;
    c.total = optLong(cal, "totalContributions");
    for (const auto& w : cal.at("weeks")) {
      ContributionWeek week;
      for (const auto& day : w.at("contributionDays"))
        week.days.push_back({static_cast<int>(optLong(day, "contributionCount")),
                             optString(day, "date").value_or(""), static_cast<int>(optLong(day, "weekday"))});
      c.weeks.push_back(std::move(week));
    }
    d.calendar = std::move(c);
    return d;
  } catch (const json::exception&) {
    return std::nullopt;
  }
}

std::optional<GitHubStatsData> parseREST(std::string_view userJson, std::string_view reposJson) {
  try {
    const json user = json::parse(userJson);
    const json repos = json::parse(reposJson);
    if (!user.is_object() || !repos.is_array()) return std::nullopt;

    GitHubStatsData d;
    d.hasFullData = false;
    std::vector<LangInput> langs;
    for (const auto& r : repos) {
      LangInput li;
      li.name = optString(r, "language");
      if (li.name) li.color = languageColor(*li.name);
      langs.push_back(li);
      d.stats.stars += optLong(r, "stargazers_count");
      d.stats.forks += optLong(r, "forks_count");
    }
    d.languageStats = languageStatsFrom(langs);
    d.stats.repos = optLong(user, "public_repos");
    d.stats.followers = optLong(user, "followers");

    for (const auto& r : repos) {
      if (r.contains("fork") && r["fork"].is_boolean() && r["fork"].get<bool>()) continue;
      if (d.topRepos.size() >= 6) break;
      TopRepo tr;
      tr.name = optString(r, "name").value_or("");
      tr.url = optString(r, "html_url").value_or("");
      tr.description = optString(r, "description");
      tr.language = optString(r, "language");
      if (tr.language) tr.languageColor = languageColor(*tr.language);
      tr.stars = optLong(r, "stargazers_count");
      tr.forks = optLong(r, "forks_count");
      d.topRepos.push_back(std::move(tr));
    }
    return d;
  } catch (const json::exception&) {
    return std::nullopt;
  }
}

std::optional<ProjectRepoStats> parseRepo(std::string_view body) {
  try {
    const json j = json::parse(body);
    if (!j.is_object()) return std::nullopt;
    return ProjectRepoStats{optLong(j, "stargazers_count"), optLong(j, "forks_count"),
                            optLong(j, "subscribers_count")};
  } catch (const json::exception&) {
    return std::nullopt;
  }
}

}  // namespace itsme::github
