#pragma once
#include <ftxui/dom/elements.hpp>
#include <random>
#include <string>
#include <vector>

namespace itsme::effects {

class MatrixRain {
 public:
  explicit MatrixRain(unsigned seed = 42);
  void resize(int w, int h);
  void advance(int ms);
  ftxui::Element render() const;
  int width() const { return w_; }
  int height() const { return h_; }
  std::string currentQuote() const;

 private:
  struct Column {
    float head = 0;
    float speed = 1;
    int length = 8;
  };
  void step();
  void resetColumn(Column& c, bool aboveScreen);
  const std::string& randomGlyph();

  std::mt19937 rng_;
  int w_ = 0, h_ = 0;
  std::vector<Column> cols_;
  std::vector<std::string> cells_;  // w_*h_ glyphs
  int stepCarry_ = 0;
  int quoteElapsed_ = 0;
  std::size_t quoteIndex_ = 0;
};

}  // namespace itsme::effects
