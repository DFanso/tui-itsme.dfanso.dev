#include "effects/Hack.hpp"

#include <algorithm>
#include <string>

#include "outputs/Theme.hpp"

namespace itsme::effects {
using namespace ftxui;
using core::Tone;
using outputs::tone;

namespace {
struct Line {
  int at;
  const char* text;
  Tone tone;
  bool dimCenter;
};
struct Bar {
  int appearAt;
  int fillStart;
  const char* label;
  Tone tone;
};
constexpr int kFillMs = 1400;

// Timeline ported from HackOverlay.tsx's anime timeline (durations + gaps summed).
constexpr Line kIntro[] = {
    {0, "> Establishing encrypted tunnel to itsme.dfanso.dev...", Tone::Green, false},
    {500, "> Resolved IP: 76.76.21.21 | ASN: Vercel Inc.", Tone::Blue, false},
    {950, "> RTT: 0.4ms  |  Packet loss: 0%  |  TTL: 64", Tone::Muted, false},
    {1450, "> TLS 1.3 handshake complete. Session key established. ✓", Tone::Green, false},
};
constexpr Bar kBars[] = {
    {2500, 2580, "[ BYPASSING CLOUDFLARE WAF ]", Tone::Red},
    {4030, 4110, "[ EXPLOITING CVE-2024-LMAO ]", Tone::Yellow},
    {5560, 5640, "[ INJECTING REVERSE SHELL PAYLOAD ]", Tone::Purple},
    {7090, 7170, "[ ESCALATING TO ROOT PRIVILEGES ]", Tone::Blue},
};
constexpr int kSep2At = 8770;
constexpr int kGrantedAt = 8970;
constexpr Line kOutro[] = {
    {9870, "root@dfanso.dev:~# whoami", Tone::Green, false},
    {10220, "root", Tone::Fg, false},
    {10670, "root@dfanso.dev:~# cat /etc/secrets", Tone::Green, false},
    {11070, "cat: /etc/secrets: nice try 😄", Tone::Red, false},
    {11620, "> jk — this is just a portfolio. but the animations are real 🔥", Tone::Muted, false},
    {12320, "[ press ESC or any key to exit ]", Tone::Muted, true},
};
}  // namespace

Element HackSequence::render(int width) const {
  const int boxWidth = std::max(30, std::min(72, width - 4));
  const Color green = outputs::matrixGreen();
  const Color sepColor = outputs::trueColor() ? Color::RGB(0x1a, 0x3a, 0x1a) : Color::GreenLight;
  Elements rows;

  for (const auto& l : kIntro)
    if (elapsed_ >= l.at) rows.push_back(text(l.text) | color(tone(l.tone)));
  if (elapsed_ >= 2000) rows.push_back(text(std::string(60, '-')) | color(sepColor));

  for (const auto& b : kBars) {
    if (elapsed_ < b.appearAt) continue;
    float progress = 0.0f;
    if (elapsed_ >= b.fillStart) progress = std::min(1.0f, static_cast<float>(elapsed_ - b.fillStart) / kFillMs);
    const int pct = static_cast<int>(progress * 100.0f + 0.5f);
    rows.push_back(text(b.label) | color(tone(b.tone)));
    rows.push_back(hbox({gauge(progress) | color(tone(b.tone)) | flex,
                         text(" " + std::to_string(pct) + "%") | color(tone(Tone::Fg))}));
  }

  if (elapsed_ >= kSep2At) rows.push_back(text(std::string(60, '-')) | color(sepColor));
  if (elapsed_ >= kGrantedAt) {
    const bool flash = elapsed_ >= kGrantedAt + 300 && elapsed_ < kGrantedAt + 650 && ((elapsed_ / 70) % 2 == 0);
    rows.push_back(text("★★★  ACCESS GRANTED  ★★★") | bold | color(flash ? tone(Tone::Red) : green) | center);
  }
  for (const auto& l : kOutro) {
    if (elapsed_ < l.at) continue;
    Element e = text(l.text) | color(tone(l.tone));
    if (l.dimCenter) e = e | dim | center;
    rows.push_back(e);
  }

  Element box = vbox(std::move(rows)) | size(WIDTH, EQUAL, boxWidth);
  return vbox({filler(), hbox({filler(), box, filler()}), filler()}) | bgcolor(Color::Black);
}

}  // namespace itsme::effects
