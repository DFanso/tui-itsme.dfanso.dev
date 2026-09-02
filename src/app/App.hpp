#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <random>
#include <string>
#include <unordered_map>

#include "app/BlockRenderer.hpp"
#include "core/LineEditor.hpp"
#include "core/TerminalState.hpp"
#include "outputs/Outputs.hpp"

namespace itsme::app {

struct Options {
  bool noBoot = false;
  bool noColor = false;
};

class App {
 public:
  explicit App(Options opts);
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
  ftxui::Element renderInputLine();
  outputs::RenderContext context() const;
  void requestRedraw();
  void requestExit();

  Options opts_;
  core::TerminalState state_;
  core::LineEditor editor_;
  std::unordered_map<int, BlockRuntime> runtime_;
  float scroll_ = 1.0f;  // 1.0 = bottom
  int width_ = 80;
  ftxui::ScreenInteractive* screen_ = nullptr;
  std::mt19937 rng_{std::random_device{}()};
};

}  // namespace itsme::app
