#include <catch2/catch_test_macros.hpp>

#include "core/Commands.hpp"

using namespace itsme::core;
static const ExecContext kCtx{};

TEST_CASE("empty input picks a nudge message by rand") {
  auto e = executeLine("   ", ExecContext{false, 0.0});
  REQUIRE(e.kind == ExecKind::Text);
  CHECK(e.text->tone == Tone::Purple);
  CHECK(e.text->lines == std::vector<std::string>{"🤔 Hmm... trying to say something?"});
  auto last = executeLine("", ExecContext{false, 0.999});
  CHECK(last.text->lines == std::vector<std::string>{"🚀 Ready for your input, commander!"});
}

TEST_CASE("clear and resume are actions") {
  auto c = executeLine("CLEAR", kCtx);
  CHECK(c.kind == ExecKind::Action);
  CHECK(c.action == Action::Clear);
  CHECK_FALSE(c.text.has_value());
  auto r = executeLine("resume now", kCtx);
  CHECK(r.action == Action::OpenResume);
  CHECK(r.text->lines == std::vector<std::string>{"Opening resume..."});
}

TEST_CASE("easter eggs") {
  auto sudo = executeLine("sudo", kCtx);
  CHECK(sudo.text->tone == Tone::Red);
  CHECK(sudo.text->lines[0].find("sudoers") != std::string::npos);
  CHECK(executeLine("sudo rm", kCtx).text->lines[0] == "└─▶ Command not found: sudo rm");
  CHECK(executeLine("rm -rf /", kCtx).text->lines[0].find("Nice try") != std::string::npos);
  CHECK(executeLine("rm -rf *", kCtx).text->lines[0].find("Nice try") != std::string::npos);
  CHECK(executeLine("rm file", kCtx).text->lines[0] == "rm: missing operand");
  CHECK(executeLine("rm -rf", kCtx).text->lines[0] == "rm: missing operand");
  for (const char* ed : {"vi", "vim", "nano"}) {
    auto e = executeLine(ed, kCtx);
    CHECK(e.text->tone == Tone::Yellow);
    CHECK(e.text->lines[0].find("code .") != std::string::npos);
  }
}

TEST_CASE("projects y/n follow-up") {
  auto p = executeLine("projects", kCtx);
  CHECK(p.kind == ExecKind::Component);
  CHECK(p.componentName == "projects");
  CHECK(p.awaitProjectResponse);

  ExecContext awaiting{true, 0.0};
  auto yes = executeLine("Y", awaiting);
  CHECK(yes.text->tone == Tone::Green);
  CHECK(yes.text->lines.size() == 2);
  CHECK_FALSE(yes.awaitProjectResponse);
  auto no = executeLine("no", awaiting);
  CHECK(no.text->lines[0] == "└─▶ Alright! Feel free to explore other commands using `help`.");
  auto other = executeLine("maybe", awaiting);
  CHECK(other.text->tone == Tone::Red);
  CHECK(other.text->lines[0] == "└─▶ Please answer with y or n.");
  CHECK(other.awaitProjectResponse);
  // Built-ins still win while awaiting
  CHECK(executeLine("clear", awaiting).action == Action::Clear);
  CHECK(executeLine("sudo", awaiting).text->tone == Tone::Red);
}

TEST_CASE("matrix and hack are actions with echo text") {
  auto m = executeLine("matrix", kCtx);
  CHECK(m.action == Action::Matrix);
  CHECK(m.text->tone == Tone::Green);
  auto h = executeLine("hack", kCtx);
  CHECK(h.action == Action::Hack);
  CHECK(h.text->tone == Tone::Red);
}

TEST_CASE("registry commands become components") {
  auto a = executeLine("  About ", kCtx);
  CHECK(a.kind == ExecKind::Component);
  CHECK(a.componentName == "about");
  CHECK_FALSE(a.awaitProjectResponse);
}

TEST_CASE("unknown commands suggest the closest name") {
  auto e = executeLine("hlep", kCtx);
  REQUIRE(e.text->lines.size() == 2);
  CHECK(e.text->lines[0] == "└─▶ Command not found: hlep");
  CHECK(e.text->lines[1] == "Did you mean 'help'?");
  auto far = executeLine("xxxxxxxx", kCtx);
  REQUIRE(far.text->lines.size() == 1);
}
