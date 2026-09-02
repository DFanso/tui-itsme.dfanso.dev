#include <catch2/catch_test_macros.hpp>
#include <ftxui/component/event.hpp>

#include "../RenderHelper.hpp"
#include "app/App.hpp"
#include "app/Clock.hpp"
#include "app/Prompt.hpp"

using namespace itsme::app;
using ftxui::Event;

TEST_CASE("clock formatting") {
  CHECK(clockHHMM(LocalTime{9, 5, 0}) == "09:05");
  CHECK(clockHHMMSS(LocalTime{23, 59, 7}) == "23:59:07");
  auto now = localNow();
  CHECK(now.hour >= 0);
  CHECK(now.hour < 24);
}

TEST_CASE("prompt variants") {
  CHECK(renderPlain(renderPrompt(false), 80, 1).find("dfanso@terminal in ~/portfolio on main") !=
        std::string::npos);
  CHECK(renderPlain(renderPrompt(true), 80, 1).find("Would you like to see more projects? (y/n)") !=
        std::string::npos);
}

TEST_CASE("app renders title bar, seeded blocks and the input line") {
  App app(Options{true, false});
  app.resize(100);
  auto s = renderPlain(app.component()->Render(), 100, 40);
  CHECK(s.find("guest@dfanso.dev:~") != std::string::npos);
  CHECK(s.find("visitor!") != std::string::npos);
  CHECK(s.find("Leo Felcianas") != std::string::npos);
  CHECK(s.find("~/portfolio") != std::string::npos);
}

TEST_CASE("typing, tab completion, enter and history") {
  App app(Options{true, false});
  auto root = app.component();
  root->OnEvent(Event::Character("a"));
  root->OnEvent(Event::Character("b"));
  root->OnEvent(Event::Tab);
  root->OnEvent(Event::Return);
  REQUIRE(app.state().blocks.size() == 3);
  CHECK(app.state().blocks.back().input == "about");
  CHECK(app.state().history == std::vector<std::string>{"about"});

  root->OnEvent(Event::ArrowUp);
  root->OnEvent(Event::Return);
  CHECK(app.state().blocks.size() == 4);
  CHECK(app.state().blocks.back().input == "about");

  app.submit("time");
  auto s = renderPlain(root->Render(), 100, 60);
  CHECK(s.find(":") != std::string::npos);  // HH:MM:SS from the time block

  root->OnEvent(Event::Special(std::string("\x0c")));  // Ctrl+L
  CHECK(app.state().blocks.empty());
}
