#include "github/Client.hpp"

#include <cstdlib>
#include <future>
#include <nlohmann/json.hpp>
#include <utility>

#include "github/Parse.hpp"

namespace itsme::github {

namespace {
const std::vector<std::string> kRestHeaders = {"Accept: application/vnd.github.v3+json",
                                               "User-Agent: Portfolio-Site"};
}

Client::Client(std::optional<std::string> token, HttpFn http) : token_(std::move(token)), http_(std::move(http)) {}

std::optional<GitHubStatsData> Client::fetchStats() const {
  if (token_) {
    nlohmann::json body = {{"query", graphqlQuery()}, {"variables", {{"username", kUsername}}}};
    auto resp = http_("https://api.github.com/graphql",
                      {"Authorization: Bearer " + *token_, "Content-Type: application/json",
                       "User-Agent: Portfolio-Site"},
                      body.dump());
    if (resp && resp->status == 200)
      if (auto data = parseGraphQL(resp->body)) return data;
  }
  auto user = http_(std::string("https://api.github.com/users/") + kUsername, kRestHeaders, std::nullopt);
  auto repos = http_(std::string("https://api.github.com/users/") + kUsername +
                         "/repos?sort=stars&direction=desc&per_page=100",
                     kRestHeaders, std::nullopt);
  if (user && repos && user->status == 200 && repos->status == 200) return parseREST(user->body, repos->body);
  return std::nullopt;
}

ProjectStatsMap Client::fetchProjectStats(const std::vector<std::string>& repos) const {
  std::vector<std::string> headers = kRestHeaders;
  if (token_) headers.push_back("Authorization: Bearer " + *token_);

  std::vector<std::pair<std::string, std::future<std::optional<ProjectRepoStats>>>> futures;
  for (const auto& repo : repos) {
    futures.emplace_back(repo, std::async(std::launch::async, [this, repo, headers]() -> std::optional<ProjectRepoStats> {
                           auto resp = http_("https://api.github.com/repos/" + repo, headers, std::nullopt);
                           if (!resp || resp->status != 200) return std::nullopt;
                           return parseRepo(resp->body);
                         }));
  }
  ProjectStatsMap out;
  for (auto& [repo, fut] : futures) out[repo] = fut.get();
  return out;
}

std::optional<std::string> tokenFromEnv() {
  const char* v = std::getenv("GITHUB_TOKEN");
  if (v == nullptr || *v == '\0') return std::nullopt;
  return std::string(v);
}

}  // namespace itsme::github
