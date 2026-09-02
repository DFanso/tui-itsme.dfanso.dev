#pragma once
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <optional>
#include <string>

#include "core/AsyncValue.hpp"
#include "core/TerminalState.hpp"
#include "github/Model.hpp"
#include "outputs/Outputs.hpp"

namespace itsme::app {

// Per-block mutable state owned by the App (async results, animation progress).
struct BlockRuntime {
  std::string timeString;       // `time`
  int typewriterRevealed = -1;  // `welcome`, -1 = fully shown
  int spinnerFrame = 0;
  std::shared_ptr<core::AsyncValue<std::optional<github::GitHubStatsData>>> githubFetch;  // null = offline
  std::shared_ptr<core::AsyncValue<github::ProjectStatsMap>> projectsFetch;
};

ftxui::Element renderBlock(const core::Block& block, const BlockRuntime& rt, const outputs::RenderContext& ctx);

}  // namespace itsme::app
