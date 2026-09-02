#include "core/TerminalState.hpp"

#include "core/Commands.hpp"
#include "core/Strings.hpp"

namespace itsme::core {

namespace {
Block seed(int id, const char* name) {
  Block b;
  b.id = id;
  b.input = name;
  b.seeded = true;
  b.execution.kind = ExecKind::Component;
  b.execution.componentName = name;
  return b;
}
}  // namespace

TerminalState initialState() {
  TerminalState s;
  s.blocks.push_back(seed(0, "welcome"));
  s.blocks.push_back(seed(1, "whoami"));
  s.nextId = 2;
  return s;
}

Action submit(TerminalState& state, std::string_view raw, double rand) {
  const std::string trimmed = trim(raw);
  const bool wasAwaiting = state.awaitingProjectResponse;
  Execution exec = executeLine(raw, ExecContext{state.awaitingProjectResponse, rand});

  if (!trimmed.empty()) state.history.push_back(toLower(trimmed));
  state.historyIndex = state.history.size();
  state.awaitingProjectResponse = exec.awaitProjectResponse;

  if (exec.action == Action::Clear) {
    state.blocks.clear();
    return Action::Clear;
  }

  const Action action = exec.action;
  Block b;
  b.id = state.nextId++;
  b.input = trimmed;
  b.wasAwaitingProjectResponse = wasAwaiting;
  b.execution = std::move(exec);
  state.blocks.push_back(std::move(b));
  return action;
}

void clearBlocks(TerminalState& state) { state.blocks.clear(); }

}  // namespace itsme::core
