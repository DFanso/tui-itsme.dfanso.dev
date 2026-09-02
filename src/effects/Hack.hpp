#pragma once
#include <ftxui/dom/elements.hpp>

namespace itsme::effects {

class HackSequence {
 public:
  static constexpr int kAutoExitMs = 14000;
  void advance(int ms) { elapsed_ += ms; }
  int elapsed() const { return elapsed_; }
  bool finished() const { return elapsed_ >= kAutoExitMs; }
  ftxui::Element render(int width) const;

 private:
  int elapsed_ = 0;
};

}  // namespace itsme::effects
