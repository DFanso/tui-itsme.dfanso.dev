#pragma once
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "github/Model.hpp"

namespace itsme::github {

struct HttpResponse {
  long status = 0;
  std::string body;
};

using HttpFn = std::function<std::optional<HttpResponse>(const std::string& url,
                                                         const std::vector<std::string>& headers,
                                                         const std::optional<std::string>& postBody)>;

class Client {
 public:
  Client(std::optional<std::string> token, HttpFn http);
  std::optional<GitHubStatsData> fetchStats() const;
  ProjectStatsMap fetchProjectStats(const std::vector<std::string>& repos) const;
  bool hasToken() const { return token_.has_value(); }

 private:
  std::optional<std::string> token_;
  HttpFn http_;
};

std::optional<std::string> tokenFromEnv();

}  // namespace itsme::github
