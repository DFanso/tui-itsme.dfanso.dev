#include "core/InputHelpers.hpp"

#include <algorithm>
#include <limits>

#include "core/Strings.hpp"

namespace itsme::core {

std::vector<std::string> getSuggestions(std::string_view input, const std::vector<std::string>& names) {
  std::vector<std::string> out;
  if (input.empty()) return out;
  const std::string lower = toLower(input);
  for (const auto& n : names)
    if (startsWith(n, lower) && n != lower) out.push_back(n);
  return out;
}

std::optional<std::string> completeInput(std::string_view input, const std::vector<std::string>& names) {
  const std::string lower = toLower(input);
  std::vector<std::string> matches;
  for (const auto& n : names)
    if (startsWith(n, lower)) matches.push_back(n);
  if (matches.size() == 1) return matches.front();
  return std::nullopt;
}

HistoryNav navigateHistory(const std::vector<std::string>& history, std::size_t index, HistoryDir dir) {
  if (dir == HistoryDir::Up) {
    const std::size_t newIndex = index > 0 ? index - 1 : 0;
    return {newIndex, newIndex < history.size() ? history[newIndex] : std::string()};
  }
  const std::size_t newIndex = (index + 1 < history.size()) ? index + 1 : history.size();
  return {newIndex, newIndex < history.size() ? history[newIndex] : std::string()};
}

int levenshtein(std::string_view a, std::string_view b) {
  std::vector<int> prev(b.size() + 1), cur(b.size() + 1);
  for (std::size_t j = 0; j <= b.size(); ++j) prev[j] = static_cast<int>(j);
  for (std::size_t i = 1; i <= a.size(); ++i) {
    cur[0] = static_cast<int>(i);
    for (std::size_t j = 1; j <= b.size(); ++j) {
      const int sub = prev[j - 1] + (a[i - 1] == b[j - 1] ? 0 : 1);
      cur[j] = std::min({sub, prev[j] + 1, cur[j - 1] + 1});
    }
    std::swap(prev, cur);
  }
  return prev[b.size()];
}

std::optional<std::string> suggestClosest(std::string_view input, const std::vector<std::string>& names) {
  const std::string lower = toLower(input);
  std::optional<std::string> best;
  int bestDistance = std::numeric_limits<int>::max();
  for (const auto& n : names) {
    const int d = levenshtein(lower, n);
    if (d < bestDistance) {
      bestDistance = d;
      best = n;
    }
  }
  if (best && bestDistance <= 2) return best;
  return std::nullopt;
}

}  // namespace itsme::core
