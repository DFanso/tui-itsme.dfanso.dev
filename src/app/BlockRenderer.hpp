#pragma once
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <string>

#include "core/TerminalState.hpp"
#include "github/Model.hpp"
#include "outputs/Outputs.hpp"

namespace itsme::app {

// Per-block mutable state owned by the App (async results, animation progress).
struct BlockRuntime {
  std::string timeString;                                   // `time`
  int typewriterRevealed = -1;                              // `welcome`, -1 = fully shown
  std::shared_ptr<const github::ProjectStatsMap> projects;  // `projects`, null until fetched
};

ftxui::Element renderBlock(const core::Block& block, const BlockRuntime& rt, const outputs::RenderContext& ctx);

}  // namespace itsme::app
