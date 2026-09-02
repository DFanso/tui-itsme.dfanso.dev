#include "effects/Boot.hpp"

#include <array>
#include <utility>

namespace itsme::effects {

namespace {
constexpr std::array<std::pair<int, const char*>, 4> kLines = {{
    {0, "[  OK  ] Booting portfolio OS..."},
    {300, "[  OK  ] Loading kernel modules: devops cloud ai"},
    {600, "[  OK  ] Mounting ~/portfolio"},
    {900, "[  OK  ] Starting Portfolio-CLI shell"},
}};
}  // namespace

std::vector<std::string> BootSequence::visibleLines() const {
  std::vector<std::string> out;
  for (const auto& [at, line] : kLines)
    if (elapsed_ >= at) out.emplace_back(line);
  return out;
}

}  // namespace itsme::effects
