#include <catch2/catch_test_macros.hpp>

#include "../RenderHelper.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

using namespace itsme;
using namespace itsme::outputs;

TEST_CASE("theme maps tones and hex colors") {
  setTrueColor(true);
  CHECK(tone(core::Tone::Blue) == ftxui::Color::RGB(0x7a, 0xa2, 0xf7));
  CHECK(hexColor("#00ADD8") == ftxui::Color::RGB(0x00, 0xAD, 0xD8));
  CHECK(hexColor("garbage") == ftxui::Color::GrayLight);
  setTrueColor(false);
  CHECK(tone(core::Tone::Blue) == ftxui::Color::Blue);
  CHECK(hexColor("#00ADD8") == ftxui::Color::GrayLight);
  setTrueColor(true);
}

TEST_CASE("text output renders each line") {
  core::TextOutput t{core::Tone::Red, {"one", "two"}};
  auto s = renderPlain(renderText(t));
  CHECK(s.find("one") != std::string::npos);
  CHECK(s.find("two") != std::string::npos);
}

TEST_CASE("welcome shows banner, greeting and hint") {
  auto s = renderPlain(renderWelcome(RenderContext{100, 9}), 100, 20);
  CHECK(s.find("██████╗") != std::string::npos);
  CHECK(s.find("Good morning, visitor!") != std::string::npos);
  CHECK(s.find("DevOps Engineer & Software Engineer") != std::string::npos);
  CHECK(s.find("Type 'help'") != std::string::npos);
  auto evening = renderPlain(renderWelcome(RenderContext{100, 21}), 100, 20);
  CHECK(evening.find("Good evening, visitor!") != std::string::npos);
  auto narrow = renderPlain(renderWelcome(RenderContext{50, 12}), 50, 20);
  CHECK(narrow.find("██████╗") == std::string::npos);
  CHECK(narrow.find("DFANSO") != std::string::npos);
}

TEST_CASE("welcome typewriter reveals progressively") {
  CHECK(welcomeTypewriterLength() > 50);
  auto partial = renderPlain(renderWelcome(RenderContext{100, 12}, 4), 100, 20);
  CHECK(partial.find("Good") != std::string::npos);
  CHECK(partial.find("afternoon") == std::string::npos);
}

TEST_CASE("whoami, about, education, certifications, contact") {
  CHECK(renderPlain(renderWhoami()).find("Leo Felcianas") != std::string::npos);
  auto about = renderPlain(renderAbout());
  CHECK(about.find("Professional Summary") != std::string::npos);
  CHECK(about.find("Sri Lanka (Open to Remote)") != std::string::npos);
  auto edu = renderPlain(renderEducation());
  CHECK(edu.find("University of Plymouth") != std::string::npos);
  CHECK(edu.find("First-Class Honours") != std::string::npos);
  auto certs = renderPlain(renderCertifications());
  CHECK(certs.find("Credential ID 2025-27675") != std::string::npos);
  CHECK(certs.find("• Expires Sep 2028") != std::string::npos);
  auto contact = renderPlain(renderContact());
  CHECK(contact.find("[GH]") != std::string::npos);
  CHECK(contact.find("https://discord.gg/DcFFdcjfAf") != std::string::npos);
}

TEST_CASE("weather, ping, time") {
  CHECK(renderPlain(renderWeather()).find("Try looking outside your window!") != std::string::npos);
  auto ping = renderPlain(renderPing());
  CHECK(ping.find("PING dfanso.dev (192.168.1.1)") != std::string::npos);
  CHECK(ping.find("icmp_seq=4") != std::string::npos);
  CHECK(ping.find("rtt min/avg/max = 0.038/0.041/0.045 ms") != std::string::npos);
  CHECK(renderPlain(renderTime("12:34:56")).find("12:34:56") != std::string::npos);
}
