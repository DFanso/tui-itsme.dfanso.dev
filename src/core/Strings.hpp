#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace itsme::core {

inline std::string trim(std::string_view s) {
  auto notSpace = [](unsigned char c) { return !std::isspace(c); };
  auto b = std::find_if(s.begin(), s.end(), notSpace);
  auto e = std::find_if(s.rbegin(), s.rend(), notSpace).base();
  return b < e ? std::string(b, e) : std::string();
}

inline std::string toLower(std::string_view s) {
  std::string out(s);
  std::transform(out.begin(), out.end(), out.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return out;
}

inline bool startsWith(std::string_view s, std::string_view prefix) {
  return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

inline bool contains(std::string_view s, std::string_view needle) {
  return s.find(needle) != std::string_view::npos;
}

// Splits a UTF-8 string into one std::string per code point.
inline std::vector<std::string> utf8Chars(std::string_view s) {
  std::vector<std::string> out;
  for (std::size_t i = 0; i < s.size();) {
    unsigned char c = static_cast<unsigned char>(s[i]);
    std::size_t len = c < 0x80 ? 1 : (c >> 5) == 0x6 ? 2 : (c >> 4) == 0xE ? 3 : (c >> 3) == 0x1E ? 4 : 1;
    if (i + len > s.size()) len = s.size() - i;
    out.emplace_back(s.substr(i, len));
    i += len;
  }
  return out;
}

}  // namespace itsme::core
