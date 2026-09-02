#include <catch2/catch_test_macros.hpp>

#include "core/InputHelpers.hpp"

using namespace itsme::core;
static const std::vector<std::string> kNames = {"help", "hack", "about", "projects", "ping"};

TEST_CASE("getSuggestions returns prefix matches, excluding exact match") {
  CHECK(getSuggestions("", kNames).empty());
  CHECK(getSuggestions("h", kNames) == std::vector<std::string>{"help", "hack"});
  CHECK(getSuggestions("HE", kNames) == std::vector<std::string>{"help"});
  CHECK(getSuggestions("help", kNames).empty());
}

TEST_CASE("completeInput completes only a unique prefix") {
  CHECK(completeInput("ab", kNames) == std::optional<std::string>{"about"});
  CHECK_FALSE(completeInput("h", kNames).has_value());
  CHECK_FALSE(completeInput("zz", kNames).has_value());
  CHECK(completeInput("PING", kNames) == std::optional<std::string>{"ping"});
}

TEST_CASE("navigateHistory mirrors the site's arrow semantics") {
  std::vector<std::string> h = {"a", "b", "c"};
  auto up = navigateHistory(h, 3, HistoryDir::Up);
  CHECK(up.index == 2);
  CHECK(up.value == "c");
  up = navigateHistory(h, 0, HistoryDir::Up);
  CHECK(up.index == 0);
  CHECK(up.value == "a");
  auto down = navigateHistory(h, 1, HistoryDir::Down);
  CHECK(down.index == 2);
  CHECK(down.value == "c");
  down = navigateHistory(h, 2, HistoryDir::Down);
  CHECK(down.index == 3);
  CHECK(down.value == "");
  auto empty = navigateHistory({}, 0, HistoryDir::Up);
  CHECK(empty.index == 0);
  CHECK(empty.value == "");
}

TEST_CASE("levenshtein and suggestClosest") {
  CHECK(levenshtein("", "") == 0);
  CHECK(levenshtein("kitten", "sitting") == 3);
  CHECK(levenshtein("abc", "") == 3);
  CHECK(suggestClosest("hlep", kNames) == std::optional<std::string>{"help"});
  CHECK(suggestClosest("PROJECT", kNames) == std::optional<std::string>{"projects"});
  CHECK_FALSE(suggestClosest("xyzxyz", kNames).has_value());
  CHECK_FALSE(suggestClosest("a", {}).has_value());
}
