#include "core/Commands.hpp"
#include "core/Strings.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderLs() {
  Elements rows = {heading("Directory listing of ~/portfolio")};
  auto emit = [&](bool dirs) {
    for (const auto& c : core::commands()) {
      if (!c.lsEntry) continue;
      const bool isDir = core::startsWith(c.lsEntry->perms, "d");
      if (isDir != dirs) continue;
      rows.push_back(hbox({
          t(c.lsEntry->perms, Tone::Muted),
          text("  "),
          t(c.lsEntry->name, isDir ? Tone::Blue : Tone::Yellow) | size(WIDTH, EQUAL, 18),
          t(c.lsEntry->note, Tone::Muted),
      }));
    }
  };
  emit(true);
  emit(false);
  rows.push_back(blank());
  rows.push_back(t("Type the command to open an entry (e.g., 'about', 'projects')", Tone::Muted));
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
