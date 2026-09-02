#pragma once
#include <optional>
#include <string>
#include <string_view>

#include "github/Model.hpp"

namespace itsme::github {

const char* graphqlQuery();
std::string languageColor(std::string_view language);

std::optional<GitHubStatsData> parseGraphQL(std::string_view body);
std::optional<GitHubStatsData> parseREST(std::string_view userJson, std::string_view reposJson);
std::optional<ProjectRepoStats> parseRepo(std::string_view body);

}  // namespace itsme::github
