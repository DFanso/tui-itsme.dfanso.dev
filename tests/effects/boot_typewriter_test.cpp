#include <catch2/catch_test_macros.hpp>

#include "effects/Boot.hpp"
#include "effects/Typewriter.hpp"

using namespace itsme::effects;

TEST_CASE("boot sequence reveals lines over time") {
  BootSequence boot;
  CHECK(boot.visibleLines().size() == 1);
  CHECK_FALSE(boot.done());
  boot.advance(299);
  CHECK(boot.visibleLines().size() == 1);
  boot.advance(1);
  CHECK(boot.visibleLines().size() == 2);
  boot.advance(600);
  CHECK(boot.visibleLines().size() == 4);
  CHECK(boot.visibleLines()[0].find("Booting portfolio OS") != std::string::npos);
  CHECK_FALSE(boot.done());
  boot.advance(200);
  CHECK(boot.done());
}

TEST_CASE("typewriter reveals characters at a fixed rate and can finish early") {
  Typewriter tw(100, 15);
  CHECK(tw.revealed() == 0);
  tw.advance(14);
  CHECK(tw.revealed() == 0);
  tw.advance(1);
  CHECK(tw.revealed() == 1);
  tw.advance(150);
  CHECK(tw.revealed() == 11);
  tw.advance(100000);
  CHECK(tw.revealed() == 100);
  CHECK(tw.done());
  Typewriter tw2(10);
  tw2.finish();
  CHECK(tw2.done());
  CHECK(tw2.revealed() == 10);
}
