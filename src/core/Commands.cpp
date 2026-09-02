#include "core/Commands.hpp"

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

}  // namespace itsme::core
