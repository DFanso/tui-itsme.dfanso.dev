#pragma once
#include <string>
#include <vector>

namespace itsme::effects {

class BootSequence {
 public:
  static constexpr int kDurationMs = 1100;
  void advance(int ms) { elapsed_ += ms; }
  bool done() const { return elapsed_ >= kDurationMs; }
  std::vector<std::string> visibleLines() const;

 private:
  int elapsed_ = 0;
};

}  // namespace itsme::effects
