#include "app/BlockRenderer.hpp"

#include "app/Prompt.hpp"
#include "outputs/Common.hpp"
#include "outputs/GitHubStats.hpp"

namespace itsme::app {
using namespace ftxui;
using core::Tone;
using namespace outputs;

namespace {
Element renderComponent(const std::string& name, const BlockRuntime& rt, const RenderContext& ctx) {
  if (name == "welcome") return renderWelcome(ctx, rt.typewriterRevealed);
  if (name == "whoami") return renderWhoami();
  if (name == "about") return renderAbout();
  if (name == "projects") {
    const github::ProjectStatsMap* stats =
        (rt.projectsFetch && rt.projectsFetch->ready()) ? &rt.projectsFetch->get() : nullptr;
    return renderProjects(stats);
  }
  if (name == "skills") return renderSkills();
  if (name == "experience") return renderExperience();
  if (name == "education") return renderEducation();
  if (name == "certifications") return renderCertifications();
  if (name == "contact") return renderContact();
  if (name == "help") return renderHelp();
  if (name == "ls") return renderLs();
  if (name == "neofetch") return renderNeofetch();
  if (name == "time") return renderTime(rt.timeString);
  if (name == "weather") return renderWeather();
  if (name == "ping") return renderPing();
  if (name == "github") {
    GitHubView view;
    view.spinnerFrame = rt.spinnerFrame;
    if (!rt.githubFetch) {
      view.status = GitHubView::Status::Failed;
    } else if (!rt.githubFetch->ready()) {
      view.status = GitHubView::Status::Loading;
    } else {
      view.data = rt.githubFetch->get();
      view.status = view.data ? GitHubView::Status::Ready : GitHubView::Status::Failed;
    }
    return renderGitHubStats(view, ctx);
  }
  return t("(no renderer for '" + name + "')", Tone::Red);
}
}  // namespace

Element renderBlock(const core::Block& block, const BlockRuntime& rt, const RenderContext& ctx) {
  Elements rows;
  if (!block.seeded)
    rows.push_back(hbox({renderPrompt(block.wasAwaitingProjectResponse), t(block.input, Tone::Fg)}));

  const auto& ex = block.execution;
  switch (ex.kind) {
    case core::ExecKind::Component:
      rows.push_back(renderComponent(ex.componentName, rt, ctx));
      break;
    case core::ExecKind::Text:
    case core::ExecKind::Action:
      if (ex.text) rows.push_back(renderText(*ex.text));
      break;
  }
  rows.push_back(blank());
  return vbox(std::move(rows));
}

}  // namespace itsme::app
