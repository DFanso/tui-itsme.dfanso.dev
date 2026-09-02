#pragma once
#include <optional>
#include <string>
#include <vector>

namespace itsme::core {

enum class Tone { Fg, Muted, Blue, Purple, Green, Red, Yellow, Cyan, Teal, Orange };
enum class CommandKind { Output, Action };
enum class Action { None, Clear, Matrix, Hack, OpenResume };
enum class ExecKind { Component, Text, Action };

struct LsEntry {
  std::string name;
  std::string perms;
  std::string note;
};

struct CommandDef {
  std::string name;
  std::string description;
  std::optional<LsEntry> lsEntry;
  CommandKind kind = CommandKind::Output;
  Action action = Action::None;
  bool hidden = false;
  bool async = false;
};

struct TextOutput {
  Tone tone = Tone::Fg;
  std::vector<std::string> lines;
};

struct Execution {
  ExecKind kind = ExecKind::Text;
  std::string componentName;
  std::optional<TextOutput> text;
  Action action = Action::None;
  bool awaitProjectResponse = false;
};

}  // namespace itsme::core
