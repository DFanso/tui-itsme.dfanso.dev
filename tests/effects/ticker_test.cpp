#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>

#include "effects/Ticker.hpp"

TEST_CASE("ticker fires repeatedly and stops on destruction") {
  std::atomic<int> ticks{0};
  {
    itsme::effects::Ticker ticker(std::chrono::milliseconds(10), [&] { ticks.fetch_add(1); });
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
  }
  const int afterStop = ticks.load();
  CHECK(afterStop >= 3);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  CHECK(ticks.load() == afterStop);
}
