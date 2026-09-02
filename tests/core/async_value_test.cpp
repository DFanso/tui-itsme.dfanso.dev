#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>

#include "core/AsyncValue.hpp"

TEST_CASE("AsyncValue runs work off-thread and signals completion") {
  std::atomic<int> done{0};
  auto v = itsme::core::AsyncValue<int>::start([] { return 41 + 1; }, [&] { done.fetch_add(1); });
  for (int i = 0; i < 200 && !v->ready(); ++i) std::this_thread::sleep_for(std::chrono::milliseconds(5));
  REQUIRE(v->ready());
  CHECK(v->get() == 42);
  for (int i = 0; i < 200 && done.load() == 0; ++i) std::this_thread::sleep_for(std::chrono::milliseconds(5));
  CHECK(done.load() == 1);
}
