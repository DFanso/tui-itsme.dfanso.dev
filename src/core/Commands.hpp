#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "core/Command.hpp"

namespace itsme::core {

const std::vector<CommandDef>& commands();
std::vector<std::string> commandNames();
const CommandDef* findCommand(std::string_view name);

struct ExecContext {
  bool awaitingProjectResponse = false;
  double rand = 0.0;  // [0,1), selects the empty-input nudge
};

Execution executeLine(std::string_view raw, const ExecContext& ctx);

}  // namespace itsme::core
