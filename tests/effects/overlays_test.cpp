#include <catch2/catch_test_macros.hpp>

#include "../RenderHelper.hpp"
#include "effects/Hack.hpp"
#include "effects/Matrix.hpp"

using namespace itsme::effects;

TEST_CASE("matrix rain fills the grid and cycles quotes") {
  MatrixRain rain(7);
  rain.resize(40, 12);
  CHECK(rain.width() == 40);
  CHECK(rain.height() == 12);
  for (int i = 0; i < 40; ++i) rain.advance(50);
  auto s = renderPlain(rain.render(), 40, 12);
  CHECK(s.find("PRESS ESC OR ANY KEY TO EXIT") != std::string::npos);
  CHECK(rain.currentQuote() == "Wake up, Neo...");
  rain.advance(4200);
  CHECK(rain.currentQuote() == "The Matrix has you.");
  // some glyph other than space/hint must be drawn
  auto glyphs = renderPlain(rain.render(), 40, 12);
  bool anyGlyph = false;
  for (const char* g : {"ｱ", "0", "A", "#", "ﾈ"})
    if (glyphs.find(g) != std::string::npos) anyGlyph = true;
  CHECK(anyGlyph);
}

TEST_CASE("hack sequence timeline") {
  HackSequence hack;
  auto s0 = renderPlain(hack.render(100), 100, 40);
  CHECK(s0.find("Establishing encrypted tunnel") != std::string::npos);
  CHECK(s0.find("Resolved IP") == std::string::npos);
  hack.advance(600);
  auto s1 = renderPlain(hack.render(100), 100, 40);
  CHECK(s1.find("Resolved IP: 76.76.21.21") != std::string::npos);
  CHECK(s1.find("ACCESS GRANTED") == std::string::npos);
  hack.advance(3000);
  auto s2 = renderPlain(hack.render(100), 100, 40);
  CHECK(s2.find("BYPASSING CLOUDFLARE WAF") != std::string::npos);
  hack.advance(6000);  // 9600 ms total
  auto s3 = renderPlain(hack.render(100), 100, 40);
  CHECK(s3.find("ACCESS GRANTED") != std::string::npos);
  CHECK_FALSE(hack.finished());
  hack.advance(5000);
  CHECK(hack.finished());
  auto s4 = renderPlain(hack.render(100), 100, 40);
  CHECK(s4.find("nice try") != std::string::npos);
  CHECK(s4.find("press ESC or any key to exit") != std::string::npos);
}
