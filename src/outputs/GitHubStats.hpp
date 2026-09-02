#pragma once
#include <ftxui/dom/elements.hpp>
#include <optional>

#include "github/Model.hpp"
#include "outputs/Outputs.hpp"

namespace itsme::outputs {

struct GitHubView {
  enum class Status { Loading, Ready, Failed };
  Status status = Status::Loading;
  std::optional<github::GitHubStatsData> data;
  int spinnerFrame = 0;
};

ftxui::Element renderGitHubStats(const GitHubView& view, const RenderContext& ctx);
extern const char* const kSpinnerFrames[10];

}  // namespace itsme::outputs
