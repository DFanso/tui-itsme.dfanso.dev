#include <catch2/catch_test_macros.hpp>

#include "../Fixtures.hpp"
#include "github/Parse.hpp"

using namespace itsme::github;

TEST_CASE("parseGraphQL maps the full-data branch") {
  auto d = parseGraphQL(readFixture("github_graphql.json"));
  REQUIRE(d.has_value());
  CHECK(d->hasFullData);
  CHECK(d->stats.commits == 812);
  CHECK(d->stats.prs == 40);
  CHECK(d->stats.issues == 9);
  CHECK(d->stats.repos == 30);
  CHECK(d->stats.stars == 18);
  CHECK(d->stats.forks == 3);
  CHECK(d->stats.followers == 42);
  CHECK(d->stats.contributions == 900);
  REQUIRE(d->languageStats.size() == 2);
  CHECK(d->languageStats[0].name == "Go");
  CHECK(d->languageStats[0].percentage == 67);
  CHECK(d->languageStats[0].color == "#00ADD8");
  CHECK(d->languageStats[1].name == "TypeScript");
  CHECK(d->languageStats[1].percentage == 33);
  CHECK(d->isPinned);
  REQUIRE(d->topRepos.size() == 1);
  CHECK(d->topRepos[0].url == "https://github.com/DFanso/k3s");
  CHECK(d->topRepos[0].description == std::optional<std::string>{"K3s CI/CD"});
  REQUIRE(d->calendar.has_value());
  CHECK(d->calendar->total == 900);
  REQUIRE(d->calendar->weeks.size() == 2);
  CHECK(d->calendar->weeks[0].days[1].count == 3);
  CHECK(d->calendar->weeks[1].days[0].date == "2026-08-30");
}

TEST_CASE("parseGraphQL returns nullopt for null user or garbage") {
  CHECK_FALSE(parseGraphQL(readFixture("github_graphql_nouser.json")).has_value());
  CHECK_FALSE(parseGraphQL("not json").has_value());
  CHECK_FALSE(parseGraphQL("{}").has_value());
}

TEST_CASE("parseREST maps the fallback branch") {
  auto d = parseREST(readFixture("github_rest_user.json"), readFixture("github_rest_repos.json"));
  REQUIRE(d.has_value());
  CHECK_FALSE(d->hasFullData);
  CHECK(d->stats.repos == 25);
  CHECK(d->stats.followers == 40);
  CHECK(d->stats.stars == 25);
  CHECK(d->stats.forks == 3);
  CHECK(d->stats.commits == 0);
  CHECK_FALSE(d->calendar.has_value());
  CHECK_FALSE(d->isPinned);
  REQUIRE(d->topRepos.size() == 4);  // forks excluded
  CHECK(d->topRepos[0].name == "k3s");
  CHECK(d->topRepos[0].languageColor == std::optional<std::string>{"#00ADD8"});
  CHECK_FALSE(d->topRepos[3].language.has_value());
  REQUIRE(d->languageStats.size() == 3);  // Go, Python, TypeScript (forks counted, as on the site)
  CHECK(d->languageStats[0].name == "Go");
  CHECK(d->languageStats[0].percentage == 50);
  CHECK(parseREST("{}", "[]").has_value());  // empty repos still parse
  CHECK_FALSE(parseREST("nope", "[]").has_value());
}

TEST_CASE("parseRepo and languageColor") {
  auto s = parseRepo(readFixture("repo_stats.json"));
  REQUIRE(s.has_value());
  CHECK(s->stars == 10);
  CHECK(s->forks == 2);
  CHECK(s->watchers == 4);
  CHECK_FALSE(parseRepo("[]").has_value());
  CHECK(languageColor("Go") == "#00ADD8");
  CHECK(languageColor("Brainfuck") == "#8b8b8b");
  CHECK(std::string(graphqlQuery()).find("contributionCalendar") != std::string::npos);
}
