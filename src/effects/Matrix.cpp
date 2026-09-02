#include "effects/Matrix.hpp"

#include <array>

#include "core/Strings.hpp"
#include "outputs/Theme.hpp"

namespace itsme::effects {
using namespace ftxui;

namespace {
const std::vector<std::string>& glyphs() {
  static const std::vector<std::string> g = itsme::core::utf8Chars(
      "ｦｧｨｩｪｫｬｭｮｯｰｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜﾝ0123456789ABCDEF!@#$%^&*");
  return g;
}

constexpr std::array<const char*, 8> kQuotes = {
    "Wake up, Neo...",
    "The Matrix has you.",
    "Follow the white rabbit.",
    "Knock, knock, Neo.",
    "There is no spoon.",
    "Free your mind.",
    "01001000 01100101 01101100 01101100 01101111",
    "// TODO: escape the simulation",
};
constexpr int kStepMs = 50;
constexpr int kQuoteCycleMs = 4200;
constexpr int kQuoteVisibleMs = 3800;
}  // namespace

MatrixRain::MatrixRain(unsigned seed) : rng_(seed) {}

const std::string& MatrixRain::randomGlyph() {
  std::uniform_int_distribution<std::size_t> d(0, glyphs().size() - 1);
  return glyphs()[d(rng_)];
}

void MatrixRain::resetColumn(Column& c, bool aboveScreen) {
  std::uniform_real_distribution<float> speed(0.4f, 1.4f);
  std::uniform_int_distribution<int> length(4, 14);
  std::uniform_real_distribution<float> start(0.0f, static_cast<float>(h_ > 0 ? h_ : 1));
  c.speed = speed(rng_);
  c.length = length(rng_);
  c.head = aboveScreen ? -start(rng_) : start(rng_) - static_cast<float>(h_);
}

void MatrixRain::resize(int w, int h) {
  w_ = w < 1 ? 1 : w;
  h_ = h < 1 ? 1 : h;
  cols_.assign(static_cast<std::size_t>(w_), Column{});
  for (auto& c : cols_) resetColumn(c, true);
  cells_.assign(static_cast<std::size_t>(w_ * h_), " ");
  for (auto& cell : cells_) cell = randomGlyph();
}

void MatrixRain::step() {
  std::uniform_real_distribution<float> chance(0.0f, 1.0f);
  for (int x = 0; x < w_; ++x) {
    Column& c = cols_[static_cast<std::size_t>(x)];
    c.head += c.speed;
    const int headRow = static_cast<int>(c.head);
    if (headRow >= 0 && headRow < h_) cells_[static_cast<std::size_t>(headRow * w_ + x)] = randomGlyph();
    if (c.head - static_cast<float>(c.length) > static_cast<float>(h_) && chance(rng_) > 0.975f)
      resetColumn(c, true);
  }
}

void MatrixRain::advance(int ms) {
  stepCarry_ += ms;
  while (stepCarry_ >= kStepMs) {
    stepCarry_ -= kStepMs;
    step();
  }
  quoteElapsed_ += ms;
  while (quoteElapsed_ >= kQuoteCycleMs) {
    quoteElapsed_ -= kQuoteCycleMs;
    quoteIndex_ = (quoteIndex_ + 1) % kQuotes.size();
  }
}

std::string MatrixRain::currentQuote() const { return kQuotes[quoteIndex_]; }

Element MatrixRain::render() const {
  const Color head = Color::White;
  const Color bright = outputs::matrixGreen();
  const Color mid = outputs::trueColor() ? Color::RGB(0x00, 0xaa, 0x28) : Color::Green;
  const Color dimC = outputs::trueColor() ? Color::RGB(0x00, 0x5a, 0x15) : Color::GreenLight;

  Elements rows;
  for (int y = 0; y < h_; ++y) {
    Elements runs;
    std::string run;
    int runClass = -1;
    auto flush = [&] {
      if (run.empty()) return;
      Element e = text(run);
      if (runClass == 1)
        e = e | color(head) | bold;
      else if (runClass == 2)
        e = e | color(bright);
      else if (runClass == 3)
        e = e | color(mid);
      else if (runClass == 4)
        e = e | color(dimC);
      runs.push_back(e);
      run.clear();
    };
    for (int x = 0; x < w_; ++x) {
      const Column& c = cols_[static_cast<std::size_t>(x)];
      const float dist = c.head - static_cast<float>(y);
      int cls = 0;
      if (dist >= 0 && dist < 1) {
        cls = 1;
      } else if (dist >= 1 && dist < static_cast<float>(c.length)) {
        const float f = dist / static_cast<float>(c.length);
        cls = f < 0.35f ? 2 : f < 0.7f ? 3 : 4;
      }
      if (cls != runClass) {
        flush();
        runClass = cls;
      }
      run += cls == 0 ? std::string(" ") : cells_[static_cast<std::size_t>(y * w_ + x)];
    }
    flush();
    rows.push_back(hbox(std::move(runs)));
  }

  Element rain = vbox(std::move(rows));
  Elements layers = {rain};
  if (quoteElapsed_ < kQuoteVisibleMs)
    layers.push_back(vbox({filler(), text(" " + currentQuote() + " ") | bold | color(bright) | bgcolor(Color::Black) | center,
                           filler()}));
  layers.push_back(vbox({filler(), text("[ PRESS ESC OR ANY KEY TO EXIT ]") | color(dimC) | center}));
  return dbox(std::move(layers)) | bgcolor(Color::Black);
}

}  // namespace itsme::effects
