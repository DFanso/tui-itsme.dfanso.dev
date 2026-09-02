#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <unordered_map>

#include "app/BlockRenderer.hpp"
#include "core/LineEditor.hpp"
#include "core/TerminalState.hpp"
#include "github/Client.hpp"
#include "outputs/Outputs.hpp"

namespace itsme::app {

struct Options {
  bool noBoot = false;
  bool noColor = false;
};

class App {
 public:
  // `client` may be null: the github/projects commands then render offline fallbacks.
  App(Options opts, std::shared_ptr<const github::Client> client);
  virtual ~App() = default;

  int run();                     // blocks until exit
  ftxui::Component component();  // root component (tests render it directly)
  void submit(const std::string& line);
  const core::TerminalState& state() const { return state_; }
  void resize(int width) { width_ = width; }

 protected:
  virtual bool onEvent(const ftxui::Event& e);
  virtual ftxui::Element render();
  virtual void onBlockAdded(const core::Block& block);
  virtual void performAction(core::Action action, int blockId);
  void startFetches(const core::Block& block);
  ftxui::Element renderInputLine();
  outputs::RenderContext context() const;
  void requestRedraw();
  void requestExit();

  // Shared with worker threads; the screen pointer is cleared before the loop exits.
  struct Redraw {
    std::mutex mutex;
    ftxui::ScreenInteractive* screen = nullptr;
    void post();
  };

  Options opts_;
  core::TerminalState state_;
  core::LineEditor editor_;
  std::unordered_map<int, BlockRuntime> runtime_;
  float scroll_ = 1.0f;  // 1.0 = bottom
  int width_ = 80;
  ftxui::ScreenInteractive* screen_ = nullptr;
  std::mt19937 rng_{std::random_device{}()};
  std::shared_ptr<const github::Client> client_;
  std::shared_ptr<Redraw> redraw_ = std::make_shared<Redraw>();
};

}  // namespace itsme::app
