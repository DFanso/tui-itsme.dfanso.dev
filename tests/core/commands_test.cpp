#include <algorithm>
#include <catch2/catch_test_macros.hpp>

#include "core/Commands.hpp"
#include "core/Strings.hpp"

using namespace itsme::core;

TEST_CASE("registry has the site's 25 commands in canonical order") {
  auto names = commandNames();
  REQUIRE(names.size() == 25);
  CHECK(names[0] == "ls");
  CHECK(names[1] == "welcome");
  CHECK(names[9] == "contact");
  CHECK(names[10] == "clear");
  CHECK(names[18] == "github");
  CHECK(names[19] == "resume");
  CHECK(names[24] == "nano");
}

TEST_CASE("findCommand is case-insensitive and exact") {
  REQUIRE(findCommand("ABOUT") != nullptr);
  CHECK(findCommand("ABOUT")->name == "about");
  CHECK(findCommand("abou") == nullptr);
  CHECK(findCommand("about me") == nullptr);
}

TEST_CASE("hidden, async and ls metadata") {
  const auto& all = commands();
  auto hiddenCount = std::count_if(all.begin(), all.end(), [](const CommandDef& c) { return c.hidden; });
  auto lsCount = std::count_if(all.begin(), all.end(), [](const CommandDef& c) { return c.lsEntry.has_value(); });
  CHECK(hiddenCount == 6);
  CHECK(lsCount == 11);
  CHECK(findCommand("resume")->hidden);
  CHECK(findCommand("resume")->lsEntry->name == "resume.pdf");
  CHECK(findCommand("resume")->action == Action::OpenResume);
  CHECK(findCommand("about")->lsEntry->perms == "drwxr-xr-x");
  CHECK_FALSE(findCommand("clear")->lsEntry.has_value());
  CHECK(findCommand("clear")->kind == CommandKind::Action);
  CHECK(findCommand("github")->async);
  CHECK(findCommand("time")->async);
  CHECK(findCommand("sudo")->action == Action::None);
}

TEST_CASE("string helpers") {
  CHECK(trim("  hi  ") == "hi");
  CHECK(trim("\t\n") == "");
  CHECK(toLower("AbC") == "abc");
  CHECK(startsWith("rm -rf /", "rm"));
  CHECK_FALSE(startsWith("r", "rm"));
  CHECK(contains("rm -rf /", "-rf"));
  auto chars = utf8Chars("a❯b");
  REQUIRE(chars.size() == 3);
  CHECK(chars[1] == "❯");
}
