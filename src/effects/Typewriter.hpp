#pragma once

namespace itsme::effects {

class Typewriter {
 public:
  explicit Typewriter(int totalChars, int msPerChar = 15) : total_(totalChars), msPerChar_(msPerChar) {}
  void advance(int ms);
  int revealed() const { return revealed_; }
  bool done() const { return revealed_ >= total_; }
  void finish() { revealed_ = total_; }

 private:
  int total_;
  int msPerChar_;
  int revealed_ = 0;
  int carry_ = 0;  // leftover ms below one character
};

}  // namespace itsme::effects
