#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace itsme::core {

std::vector<std::string> getSuggestions(std::string_view input, const std::vector<std::string>& names);
std::optional<std::string> completeInput(std::string_view input, const std::vector<std::string>& names);

enum class HistoryDir { Up, Down };
struct HistoryNav {
  std::size_t index;
  std::string value;
};
HistoryNav navigateHistory(const std::vector<std::string>& history, std::size_t index, HistoryDir dir);

int levenshtein(std::string_view a, std::string_view b);
std::optional<std::string> suggestClosest(std::string_view input, const std::vector<std::string>& names);

}  // namespace itsme::core
