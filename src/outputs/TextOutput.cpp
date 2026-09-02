#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"

namespace itsme::outputs {
using namespace ftxui;

Element renderText(const core::TextOutput& out) {
  Elements lines;
  for (const auto& line : out.lines) lines.push_back(para(line, out.tone));
  return vbox(std::move(lines));
}

}  // namespace itsme::outputs
