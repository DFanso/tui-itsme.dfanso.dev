#include <catch2/catch_test_macros.hpp>
#include <mutex>
#include <vector>

#include "../Fixtures.hpp"
#include "github/Client.hpp"

using namespace itsme::github;

namespace {
struct FakeHttp {
  std::mutex mutex;  // fetchProjectStats calls the HttpFn from several threads
  std::vector<std::string> urls;
  std::vector<std::vector<std::string>> headers;
  long graphqlStatus = 200;
  bool restOk = true;

  HttpFn fn() {
    return [this](const std::string& url, const std::vector<std::string>& hdrs,
                  const std::optional<std::string>& body) -> std::optional<HttpResponse> {
      {
        std::lock_guard<std::mutex> lock(mutex);
        urls.push_back(url);
        headers.push_back(hdrs);
      }
      if (url == "https://api.github.com/graphql") {
        REQUIRE(body.has_value());
        return HttpResponse{graphqlStatus, graphqlStatus == 200 ? readFixture("github_graphql.json") : "{}"};
      }
      if (!restOk) return std::nullopt;
      if (url == "https://api.github.com/users/dfansoo")
        return HttpResponse{200, readFixture("github_rest_user.json")};
      if (url.rfind("https://api.github.com/users/dfansoo/repos", 0) == 0)
        return HttpResponse{200, readFixture("github_rest_repos.json")};
      if (url == "https://api.github.com/repos/DFanso/k3s") return HttpResponse{200, readFixture("repo_stats.json")};
      return HttpResponse{404, "{}"};
    };
  }
};
}  // namespace

TEST_CASE("with a token, GraphQL is used and the bearer header is sent") {
  FakeHttp http;
  Client c(std::string("tok"), http.fn());
  auto d = c.fetchStats();
  REQUIRE(d.has_value());
  CHECK(d->hasFullData);
  REQUIRE(http.urls.size() == 1);
  bool hasAuth = false;
  for (auto& h : http.headers[0])
    if (h == "Authorization: Bearer tok") hasAuth = true;
  CHECK(hasAuth);
}

TEST_CASE("GraphQL failure falls back to REST; no token goes straight to REST") {
  FakeHttp http;
  http.graphqlStatus = 401;
  Client c(std::string("bad"), http.fn());
  auto d = c.fetchStats();
  REQUIRE(d.has_value());
  CHECK_FALSE(d->hasFullData);
  CHECK(http.urls.size() == 3);

  FakeHttp http2;
  Client c2(std::nullopt, http2.fn());
  REQUIRE(c2.fetchStats().has_value());
  CHECK(http2.urls.size() == 2);
  CHECK_FALSE(c2.hasToken());
}

TEST_CASE("both sources failing yields nullopt") {
  FakeHttp http;
  http.graphqlStatus = 500;
  http.restOk = false;
  Client c(std::string("tok"), http.fn());
  CHECK_FALSE(c.fetchStats().has_value());
}

TEST_CASE("fetchProjectStats maps per repo, null on failure") {
  FakeHttp http;
  Client c(std::nullopt, http.fn());
  auto m = c.fetchProjectStats({"DFanso/k3s", "DFanso/missing"});
  REQUIRE(m.size() == 2);
  REQUIRE(m["DFanso/k3s"].has_value());
  CHECK(m["DFanso/k3s"]->watchers == 4);
  CHECK_FALSE(m["DFanso/missing"].has_value());
}
