#include "app/App.hpp"

#include <ftxui/dom/elements.hpp>

#include "app/Clock.hpp"
#include "app/Opener.hpp"
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
const Event kTick = Event::Special("itsme-tick");
constexpr auto kFastTick = std::chrono::milliseconds(50);
constexpr auto kIdleTick = std::chrono::milliseconds(1000);
}  // namespace

App::App(Options opts, std::shared_ptr<const github::Client> client)
    : opts_(opts), state_(core::initialState()), client_(std::move(client)) {
  outputs::setTrueColor(!opts_.noColor);
  if (!opts_.noBoot) {
    boot_.emplace();
    typewriter_.emplace(outputs::welcomeTypewriterLength());
    runtime_[0].typewriterRevealed = 0;
  }
}

void App::Redraw::post() { postEvent(Event::Custom); }

void App::Redraw::postEvent(const Event& e) {
  std::lock_guard<std::mutex> lock(mutex);
  if (screen) screen->PostEvent(e);
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
  auto redraw = redraw_;
  ticker_ = std::make_unique<effects::Ticker>(animating() ? kFastTick : kIdleTick,
                                              [redraw] { redraw->postEvent(kTick); });
  lastTick_ = std::chrono::steady_clock::now();
  screen.Loop(component());
  ticker_.reset();
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

bool App::animating() const {
  if (overlay_ != Overlay::None) return true;
  if (boot_ && !boot_->done()) return true;
  if (typewriter_ && !typewriter_->done()) return true;
  for (const auto& [id, rt] : runtime_)
    if (rt.githubFetch && !rt.githubFetch->ready()) return true;
  return false;
}

void App::updateTickerRate() {
  if (ticker_) ticker_->setInterval(animating() ? kFastTick : kIdleTick);
}

void App::onTick(int elapsedMs) {
  if (overlay_ == Overlay::Matrix && matrix_) {
    const int w = screen_ ? screen_->dimx() : width_;
    const int h = screen_ ? screen_->dimy() : 24;
    if (w != matrix_->width() || h != matrix_->height()) matrix_->resize(w, h);
    matrix_->advance(elapsedMs);
    return;
  }
  if (overlay_ == Overlay::Hack && hack_) {
    hack_->advance(elapsedMs);
    if (hack_->finished()) closeOverlay();
    return;
  }
  if (boot_ && !boot_->done()) {
    boot_->advance(elapsedMs);
    return;
  }
  if (typewriter_ && !typewriter_->done()) {
    typewriter_->advance(elapsedMs);
    runtime_[0].typewriterRevealed = typewriter_->done() ? -1 : typewriter_->revealed();
  }
  for (auto& [id, rt] : runtime_)
    if (rt.githubFetch && !rt.githubFetch->ready()) rt.spinnerFrame = (rt.spinnerFrame + 1) % 10;
}

void App::submit(const std::string& line) {
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  const core::Action action = core::submit(state_, line, dist(rng_));
  scroll_ = 1.0f;
  if (action == core::Action::Clear) {
    runtime_.clear();
    updateTickerRate();
    return;
  }
  const core::Block& added = state_.blocks.back();
  onBlockAdded(added);
  if (action != core::Action::None) performAction(action, added.id);
  updateTickerRate();
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

void App::performAction(core::Action action, int /*blockId*/) {
  if (action == core::Action::OpenResume) {
    auto& lines = state_.blocks.back().execution.text->lines;
    const std::string& url = data::profile().resumeUrl;
    lines.push_back(url);
    if (isSshSession())
      lines.push_back("(open the link above in your browser)");
    else if (!openUrl(url))
      lines.push_back("(could not launch a browser; open the link above manually)");
    return;
  }
  const int w = screen_ ? screen_->dimx() : width_;
  const int h = screen_ ? screen_->dimy() : 24;
  if (action == core::Action::Matrix) {
    matrix_.emplace(static_cast<unsigned>(rng_()));
    matrix_->resize(w, h);
    overlay_ = Overlay::Matrix;
  } else if (action == core::Action::Hack) {
    hack_.emplace();
    overlay_ = Overlay::Hack;
  }
}

void App::closeOverlay() {
  overlay_ = Overlay::None;
  matrix_.reset();
  hack_.reset();
}

bool App::onEvent(const Event& e) {
  if (e == kTick) {
    const auto now = std::chrono::steady_clock::now();
    const int elapsed =
        static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTick_).count());
    lastTick_ = now;
    onTick(elapsed);
    updateTickerRate();
    return true;
  }
  if (overlay_ != Overlay::None) {
    if (e == Event::Custom || e.is_mouse()) return true;
    closeOverlay();
    updateTickerRate();
    return true;
  }
  if (boot_ && !boot_->done()) return true;  // swallow input during boot
  if (typewriter_ && !typewriter_->done() && !e.is_mouse() && e != Event::Custom) {
    typewriter_->finish();
    runtime_[0].typewriterRevealed = -1;
  }
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
  if (boot_ && !boot_->done()) {
    Elements lines;
    for (const auto& l : boot_->visibleLines()) lines.push_back(t(l, Tone::Green));
    return vbox(std::move(lines)) | bgcolor(outputs::bgColor()) | flex;
  }
  if (overlay_ == Overlay::Matrix && matrix_) return matrix_->render();
  if (overlay_ == Overlay::Hack && hack_) return hack_->render(width_);
  const auto ctx = context();
  Elements blocks;
  for (const auto& b : state_.blocks) blocks.push_back(renderBlock(b, runtime_[b.id], ctx));
  Element output = vbox(std::move(blocks)) | focusPositionRelative(0.0f, scroll_) | yframe | flex;
  return vbox({renderTitleBar(clockHHMM(localNow())), output, renderInputLine()}) | bgcolor(outputs::bgColor());
}

}  // namespace itsme::app
