#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "core/Command.hpp"

namespace itsme::core {

struct Block {
  int id = 0;
  std::string input;
  bool seeded = false;                      // rendered without a prompt echo
  bool wasAwaitingProjectResponse = false;  // echo shows the y/n prompt
  Execution execution;
};

struct TerminalState {
  std::vector<Block> blocks;
  std::vector<std::string> history;
  std::size_t historyIndex = 0;  // == history.size() when not browsing
  bool awaitingProjectResponse = false;
  int nextId = 0;
};

TerminalState initialState();
Action submit(TerminalState& state, std::string_view raw, double rand);
void clearBlocks(TerminalState& state);

}  // namespace itsme::core
