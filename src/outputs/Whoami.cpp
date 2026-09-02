#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderWhoami() {
  const auto& p = data::profile();
  return vbox({
      text(p.name) | color(tone(Tone::Blue)) | bold,
      t(p.tagline, Tone::Muted),
  });
}

}  // namespace itsme::outputs
