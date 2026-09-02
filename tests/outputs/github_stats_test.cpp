#include <catch2/catch_test_macros.hpp>

#include "../Fixtures.hpp"
#include "../RenderHelper.hpp"
#include "github/Parse.hpp"
#include "outputs/GitHubStats.hpp"

using namespace itsme::outputs;

TEST_CASE("github stats states") {
  GitHubView loading;
  CHECK(renderPlain(renderGitHubStats(loading, RenderContext{100, 12})).find("Fetching GitHub stats...") !=
        std::string::npos);

  GitHubView failed;
  failed.status = GitHubView::Status::Failed;
  CHECK(renderPlain(renderGitHubStats(failed, RenderContext{100, 12})).find("Unable to fetch GitHub data") !=
        std::string::npos);

  GitHubView ready;
  ready.status = GitHubView::Status::Ready;
  ready.data = itsme::github::parseGraphQL(readFixture("github_graphql.json"));
  auto s = renderPlain(renderGitHubStats(ready, RenderContext{100, 12}), 100, 40);
  CHECK(s.find("GitHub Statistics") != std::string::npos);
  CHECK(s.find("@DFanso") != std::string::npos);
  CHECK(s.find("812") != std::string::npos);
  CHECK(s.find("Commits") != std::string::npos);
  CHECK(s.find("Go 67%") != std::string::npos);
  CHECK(s.find("contributions.heatmap") != std::string::npos);
  CHECK(s.find("900 contributions this year") != std::string::npos);
  CHECK(s.find("k3s") != std::string::npos);

  auto narrow = renderPlain(renderGitHubStats(ready, RenderContext{60, 12}), 60, 40);
  CHECK(narrow.find("contributions.heatmap") == std::string::npos);
}
