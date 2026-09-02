#include "core/Commands.hpp"

#include <array>
#include <cmath>

#include "core/InputHelpers.hpp"
#include "core/Strings.hpp"

namespace itsme::core {

namespace {
LsEntry dir(const char* name, const char* note) { return {name, "drwxr-xr-x", note}; }
LsEntry file(const char* name, const char* note) { return {name, "-rw-r--r--", note}; }

std::vector<CommandDef> buildRegistry() {
  std::vector<CommandDef> v;
  v.push_back({"ls", "List available sections and commands", std::nullopt, CommandKind::Output});
  v.push_back({"welcome", "Display welcome message and ASCII art", file("welcome.txt", "welcome message"),
               CommandKind::Output});
  v.push_back({"whoami", "Show detailed profile information", file("whoami.txt", "profile info"),
               CommandKind::Output});
  v.push_back({"about", "View my professional summary", dir("about/", "professional summary"),
               CommandKind::Output});
  v.push_back({"projects", "Browse my featured projects", dir("projects/", "featured work"),
               CommandKind::Output});
  v.push_back({"skills", "List technical skills and expertise", dir("skills/", "technical expertise"),
               CommandKind::Output});
  v.push_back({"experience", "View work history and roles", dir("experience/", "work history"),
               CommandKind::Output});
  v.push_back({"education", "View academic background", dir("education/", "academic background"),
               CommandKind::Output});
  v.push_back({"certifications", "View professional certificates", dir("certifications/", "licenses & certs"),
               CommandKind::Output});
  v.push_back({"contact", "Get my contact information", dir("contact/", "social links"),
               CommandKind::Output});
  v.push_back({"clear", "Clear terminal screen", std::nullopt, CommandKind::Action, Action::Clear});
  v.push_back({"help", "Show this help message", std::nullopt, CommandKind::Output});
  v.push_back({"neofetch", "Display system information", std::nullopt, CommandKind::Output});
  v.push_back({"time", "Show current time", std::nullopt, CommandKind::Output, Action::None, false, true});
  v.push_back({"weather", "Check the weather (sort of)", std::nullopt, CommandKind::Output});
  v.push_back({"matrix", "Full-screen matrix rain simulation", std::nullopt, CommandKind::Action,
               Action::Matrix});
  v.push_back({"hack", "Initiate a totally real hacking sequence 😈", std::nullopt, CommandKind::Action,
               Action::Hack});
  v.push_back({"ping", "Test connection to dfanso.dev", std::nullopt, CommandKind::Output});
  v.push_back({"github", "Show GitHub stats and contributions", dir("github/", "stats & contributions"),
               CommandKind::Output, Action::None, false, true});
  // Hidden / easter eggs: valid input, excluded from `help`.
  v.push_back({"resume", "Open the resume PDF", file("resume.pdf", "curriculum vitae"), CommandKind::Action,
               Action::OpenResume, true});
  v.push_back({"sudo", "Attempt to gain superuser privileges", std::nullopt, CommandKind::Action,
               Action::None, true});
  v.push_back({"rm", "Remove files (nice try)", std::nullopt, CommandKind::Action, Action::None, true});
  v.push_back({"vi", "Open the vi text editor", std::nullopt, CommandKind::Action, Action::None, true});
  v.push_back({"vim", "Open the vim text editor", std::nullopt, CommandKind::Action, Action::None, true});
  v.push_back({"nano", "Open the nano text editor", std::nullopt, CommandKind::Action, Action::None, true});
  return v;
}
}  // namespace

const std::vector<CommandDef>& commands() {
  static const std::vector<CommandDef> registry = buildRegistry();
  return registry;
}

std::vector<std::string> commandNames() {
  std::vector<std::string> names;
  for (const auto& c : commands()) names.push_back(c.name);
  return names;
}

const CommandDef* findCommand(std::string_view name) {
  const std::string lower = toLower(name);
  for (const auto& c : commands())
    if (c.name == lower) return &c;
  return nullptr;
}

namespace {
constexpr std::array<const char*, 7> kEmptyInputMessages = {
    "🤔 Hmm... trying to say something?",
    "💭 The silence is deafening...",
    "⌨️ The keyboard is waiting for your command!",
    "✨ Type 'help' if you're not sure what to do",
    "🌟 Press some keys, then press Enter!",
    "🎯 Almost there! Just need to type a command first",
    "🚀 Ready for your input, commander!",
};

Execution textExec(Tone tone, std::vector<std::string> lines) {
  Execution e;
  e.kind = ExecKind::Text;
  e.text = TextOutput{tone, std::move(lines)};
  return e;
}

Execution actionExec(Action action, std::optional<TextOutput> echo) {
  Execution e;
  e.kind = ExecKind::Action;
  e.action = action;
  e.text = std::move(echo);
  return e;
}
}  // namespace

Execution executeLine(std::string_view raw, const ExecContext& ctx) {
  const std::string command = toLower(trim(raw));

  if (command.empty()) {
    const double r = ctx.rand < 0.0 ? 0.0 : ctx.rand;
    auto idx = static_cast<std::size_t>(std::floor(r * kEmptyInputMessages.size()));
    if (idx >= kEmptyInputMessages.size()) idx = kEmptyInputMessages.size() - 1;
    return textExec(Tone::Purple, {kEmptyInputMessages[idx]});
  }

  const std::string cmd = command.substr(0, command.find(' '));

  if (cmd == "clear") return actionExec(Action::Clear, std::nullopt);
  if (cmd == "resume") return actionExec(Action::OpenResume, TextOutput{Tone::Fg, {"Opening resume..."}});

  if (command == "sudo")
    return textExec(Tone::Red, {"Permission denied: You are not in the sudoers file. This incident will be "
                                "reported to Santa Claus. 🎅"});

  if (startsWith(command, "rm")) {
    if (contains(command, "-rf") && (contains(command, "/") || contains(command, "*")))
      return textExec(Tone::Red, {"⚠️ CRITICAL ERROR: Nice try! But I can't let you delete my portfolio."});
    return textExec(Tone::Red, {"rm: missing operand"});
  }

  if (command == "vi" || command == "vim" || command == "nano")
    return textExec(Tone::Yellow,
                    {"Error: Text editor functionality not implemented yet. Try 'code .' instead? 😉"});

  if (ctx.awaitingProjectResponse) {
    if (command == "y" || command == "yes")
      return textExec(Tone::Green, {"🔗 Check out more of my projects:", "└─▶ https://github.com/DFanso"});
    if (command == "n" || command == "no")
      return textExec(Tone::Green, {"└─▶ Alright! Feel free to explore other commands using `help`."});
    Execution e = textExec(Tone::Red, {"└─▶ Please answer with y or n."});
    e.awaitProjectResponse = true;
    return e;
  }

  if (cmd == "matrix")
    return actionExec(Action::Matrix,
                      TextOutput{Tone::Green, {"Launching matrix simulation... (press ESC or any key to exit)"}});
  if (cmd == "hack")
    return actionExec(Action::Hack, TextOutput{Tone::Red, {"Initiating hack sequence... (stand by)"}});

  if (const CommandDef* found = findCommand(command)) {
    Execution e;
    e.kind = ExecKind::Component;
    e.componentName = found->name;
    e.awaitProjectResponse = (found->name == "projects");
    return e;
  }

  std::vector<std::string> lines = {"└─▶ Command not found: " + command};
  if (auto suggestion = suggestClosest(command, commandNames()))
    lines.push_back("Did you mean '" + *suggestion + "'?");
  return textExec(Tone::Red, std::move(lines));
}

}  // namespace itsme::core
