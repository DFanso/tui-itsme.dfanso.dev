#include <catch2/catch_test_macros.hpp>

#include "core/TerminalState.hpp"

using namespace itsme::core;

TEST_CASE("initial state seeds welcome and whoami") {
  auto s = initialState();
  REQUIRE(s.blocks.size() == 2);
  CHECK(s.blocks[0].execution.componentName == "welcome");
  CHECK(s.blocks[1].execution.componentName == "whoami");
  CHECK(s.blocks[0].seeded);
  CHECK(s.nextId == 2);
  CHECK(s.history.empty());
  CHECK(s.historyIndex == 0);
}

TEST_CASE("submit appends a block, records history, returns the action") {
  auto s = initialState();
  CHECK(submit(s, "  About ", 0.0) == Action::None);
  REQUIRE(s.blocks.size() == 3);
  CHECK(s.blocks[2].id == 2);
  CHECK(s.blocks[2].input == "About");
  CHECK_FALSE(s.blocks[2].seeded);
  CHECK(s.history == std::vector<std::string>{"about"});
  CHECK(s.historyIndex == 1);
}

TEST_CASE("empty input adds a nudge block but no history") {
  auto s = initialState();
  submit(s, "", 0.0);
  CHECK(s.blocks.size() == 3);
  CHECK(s.history.empty());
}

TEST_CASE("clear wipes blocks and does not append") {
  auto s = initialState();
  submit(s, "about", 0.0);
  CHECK(submit(s, "clear", 0.0) == Action::Clear);
  CHECK(s.blocks.empty());
  CHECK(s.history == std::vector<std::string>{"about", "clear"});
  clearBlocks(s);
  CHECK(s.blocks.empty());
}

TEST_CASE("projects toggles awaiting flag and snapshot is kept on the answer block") {
  auto s = initialState();
  submit(s, "projects", 0.0);
  CHECK(s.awaitingProjectResponse);
  submit(s, "y", 0.0);
  CHECK_FALSE(s.awaitingProjectResponse);
  CHECK(s.blocks.back().wasAwaitingProjectResponse);
  CHECK_FALSE(s.blocks[s.blocks.size() - 2].wasAwaitingProjectResponse);
}

TEST_CASE("matrix returns its action and still appends the echo block") {
  auto s = initialState();
  CHECK(submit(s, "matrix", 0.0) == Action::Matrix);
  CHECK(s.blocks.back().execution.action == Action::Matrix);
}
