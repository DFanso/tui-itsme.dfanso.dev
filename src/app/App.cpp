#include "app/App.hpp"

#include <ftxui/dom/elements.hpp>

#include "app/Clock.hpp"
#include "app/Prompt.hpp"
#include "app/TitleBar.hpp"
#include "core/Commands.hpp"
#include "core/InputHelpers.hpp"
#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Theme.hpp"

namespace itsme::app {
using namespace ftxui;
using core::Tone;
using outputs::t;

namespace {
const Event kCtrlL = Event::Special(std::string("\x0c"));
const Event kCtrlD = Event::Special(std::string("\x04"));
}  // namespace

App::App(Options opts, std::shared_ptr<const github::Client> client)
    : opts_(opts), state_(core::initialState()), client_(std::move(client)) {
  outputs::setTrueColor(!opts_.noColor);
}

void App::Redraw::post() {
  std::lock_guard<std::mutex> lock(mutex);
  if (screen) screen->PostEvent(Event::Custom);
}

Component App::component() {
  return CatchEvent(Renderer([this] { return render(); }), [this](const Event& e) { return onEvent(e); });
}

int App::run() {
  auto screen = ScreenInteractive::Fullscreen();
  screen_ = &screen;
  {
    std::lock_guard<std::mutex> lock(redraw_->mutex);
    redraw_->screen = &screen;
  }
  screen.Loop(component());
  {
    std::lock_guard<std::mutex> lock(redraw_->mutex);
    redraw_->screen = nullptr;
  }
  screen_ = nullptr;
  return 0;
}

void App::requestRedraw() { redraw_->post(); }

void App::requestExit() {
  if (screen_) screen_->Exit();
}

outputs::RenderContext App::context() const {
  outputs::RenderContext ctx;
  ctx.width = width_;
  ctx.hour = localNow().hour;
  return ctx;
}

void App::submit(const std::string& line) {
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  const core::Action action = core::submit(state_, line, dist(rng_));
  scroll_ = 1.0f;
  if (action == core::Action::Clear) {
    runtime_.clear();
    return;
  }
  const core::Block& added = state_.blocks.back();
  onBlockAdded(added);
  if (action != core::Action::None) performAction(action, added.id);
}

void App::onBlockAdded(const core::Block& block) {
  if (block.execution.kind != core::ExecKind::Component) return;
  if (block.execution.componentName == "time") runtime_[block.id].timeString = clockHHMMSS(localNow());
  startFetches(block);
}

void App::startFetches(const core::Block& block) {
  if (!client_) return;
  auto client = client_;
  auto redraw = redraw_;
  const auto& name = block.execution.componentName;
  if (name == "github") {
    runtime_[block.id].githubFetch = core::AsyncValue<std::optional<github::GitHubStatsData>>::start(
        [client] { return client->fetchStats(); }, [redraw] { redraw->post(); });
  } else if (name == "projects") {
    std::vector<std::string> repos;
    for (const auto& p : data::projects())
      if (p.github) repos.push_back(*p.github);
    runtime_[block.id].projectsFetch = core::AsyncValue<github::ProjectStatsMap>::start(
        [client, repos] { return client->fetchProjectStats(repos); }, [redraw] { redraw->post(); });
  }
}

void App::performAction(core::Action /*action*/, int /*blockId*/) {
  // Matrix/Hack overlays and resume opening are wired in later tasks.
}

bool App::onEvent(const Event& e) {
  if (e == Event::Return) {
    const std::string line = editor_.text();
    editor_.clear();
    submit(line);
    return true;
  }
  if (e == kCtrlL) {
    core::clearBlocks(state_);
    runtime_.clear();
    return true;
  }
  if (e == kCtrlD) {
    requestExit();
    return true;
  }
  if (e == Event::ArrowUp || e == Event::ArrowDown) {
    auto nav = core::navigateHistory(state_.history, state_.historyIndex,
                                     e == Event::ArrowUp ? core::HistoryDir::Up : core::HistoryDir::Down);
    state_.historyIndex = nav.index;
    editor_.set(nav.value);
    return true;
  }
  if (e == Event::Tab) {
    if (auto c = core::completeInput(editor_.text(), core::commandNames())) editor_.set(*c);
    return true;
  }
  if (e == Event::Backspace) {
    editor_.backspace();
    return true;
  }
  if (e == Event::Delete) {
    editor_.del();
    return true;
  }
  if (e == Event::ArrowLeft) {
    editor_.left();
    return true;
  }
  if (e == Event::ArrowRight) {
    editor_.right();
    return true;
  }
  if (e == Event::Home) {
    editor_.home();
    return true;
  }
  if (e == Event::End) {
    editor_.end();
    return true;
  }
  if (e == Event::PageUp) {
    scroll_ = scroll_ > 0.2f ? scroll_ - 0.2f : 0.0f;
    return true;
  }
  if (e == Event::PageDown) {
    scroll_ = scroll_ < 0.8f ? scroll_ + 0.2f : 1.0f;
    return true;
  }
  if (e.is_mouse()) {
    Event mouseEvent = e;  // Event::mouse() is non-const in FTXUI
    if (mouseEvent.mouse().button == Mouse::WheelUp) {
      scroll_ = scroll_ > 0.05f ? scroll_ - 0.05f : 0.0f;
      return true;
    }
    if (mouseEvent.mouse().button == Mouse::WheelDown) {
      scroll_ = scroll_ < 0.95f ? scroll_ + 0.05f : 1.0f;
      return true;
    }
    return false;
  }
  if (e == Event::Custom) return true;
  if (e.is_character()) {
    editor_.insert(e.character());
    scroll_ = 1.0f;
    return true;
  }
  return false;
}

Element App::renderInputLine() {
  const auto names = core::commandNames();
  const auto suggestions = core::getSuggestions(editor_.text(), names);
  std::string ghost;
  if (suggestions.size() == 1 && editor_.at().empty()) ghost = suggestions.front().substr(editor_.text().size());

  const std::string at = editor_.at();
  Element line = hbox({
      renderPrompt(state_.awaitingProjectResponse),
      t(editor_.before(), Tone::Fg),
      text(at.empty() ? " " : at) | inverted,
      t(editor_.after(), Tone::Fg),
      t(ghost, Tone::Muted) | dim,
  });
  if (suggestions.size() > 1) {
    std::string joined;
    for (const auto& s : suggestions) joined += s + "  ";
    return vbox({line, outputs::indent(t(joined, Tone::Muted) | dim, 2)});
  }
  return line;
}

Element App::render() {
  if (screen_) width_ = screen_->dimx();
  const auto ctx = context();
  Elements blocks;
  for (const auto& b : state_.blocks) blocks.push_back(renderBlock(b, runtime_[b.id], ctx));
  Element output = vbox(std::move(blocks)) | focusPositionRelative(0.0f, scroll_) | yframe | flex;
  return vbox({renderTitleBar(clockHHMM(localNow())), output, renderInputLine()}) | bgcolor(outputs::bgColor());
}

}  // namespace itsme::app
