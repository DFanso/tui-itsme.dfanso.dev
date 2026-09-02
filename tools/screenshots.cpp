// Developer tool: renders representative frames of the TUI off-screen and writes them as
// SVG "screenshots" (used by the README). Build with -DITSME_BUILD_TOOLS=ON and run:
//   itsme_screenshots <output-dir>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/string.hpp>
#include <ftxui/screen/terminal.hpp>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "app/App.hpp"
#include "core/Strings.hpp"
#include "effects/Hack.hpp"
#include "effects/Matrix.hpp"
#include "github/Client.hpp"

namespace {

struct Style {
  std::string fg = "#c0caf5";
  std::string bg = "#1a1b26";
  bool bold = false;
  bool dim = false;
  bool inverted = false;
  bool operator==(const Style& o) const {
    return fg == o.fg && bg == o.bg && bold == o.bold && dim == o.dim && inverted == o.inverted;
  }
};

struct Span {
  int col;
  std::string text;
  Style style;
};

std::string hex(int r, int g, int b) {
  char buf[8];
  std::snprintf(buf, sizeof buf, "#%02x%02x%02x", r & 0xff, g & 0xff, b & 0xff);
  return buf;
}

// Minimal SGR interpreter for what FTXUI's Screen::ToString emits.
void applySgr(const std::vector<int>& p, Style& s) {
  for (std::size_t i = 0; i < p.size(); ++i) {
    const int code = p[i];
    if (code == 0) s = Style{};
    else if (code == 1) s.bold = true;
    else if (code == 2) s.dim = true;
    else if (code == 7) s.inverted = true;
    else if (code == 22) s.bold = s.dim = false;
    else if (code == 27) s.inverted = false;
    else if (code == 39) s.fg = "#c0caf5";
    else if (code == 49) s.bg = "#1a1b26";
    else if ((code == 38 || code == 48) && i + 4 < p.size() && p[i + 1] == 2) {
      const std::string c = hex(p[i + 2], p[i + 3], p[i + 4]);
      (code == 38 ? s.fg : s.bg) = c;
      i += 4;
    } else if ((code == 38 || code == 48) && i + 2 < p.size() && p[i + 1] == 5) {
      i += 2;  // 256-color: not used in true-color mode
    }
  }
}

std::string escapeXml(const std::string& s) {
  std::string out;
  for (char c : s) {
    switch (c) {
      case '&': out += "&amp;"; break;
      case '<': out += "&lt;"; break;
      case '>': out += "&gt;"; break;
      case '"': out += "&quot;"; break;
      default: out += c;
    }
  }
  return out;
}

void writeSvg(const std::string& path, const ftxui::Screen& screen, int cols, int rows) {
  const double cw = 8.4, lh = 18.0, pad = 14.0, fontSize = 14.0;
  const double width = cols * cw + 2 * pad, height = rows * lh + 2 * pad;

  std::vector<std::vector<Span>> lines(static_cast<std::size_t>(rows));
  const std::string ansi = screen.ToString();
  Style style;
  int row = 0, col = 0;
  auto lineAt = [&](int r) -> std::vector<Span>& { return lines[static_cast<std::size_t>(r)]; };
  for (std::size_t i = 0; i < ansi.size() && row < rows;) {
    const char c = ansi[i];
    if (c == '\x1b' && i + 1 < ansi.size() && ansi[i + 1] == '[') {
      std::size_t j = i + 2;
      std::vector<int> params;
      int cur = 0;
      bool any = false;
      while (j < ansi.size() && ansi[j] != 'm') {
        if (ansi[j] == ';') { params.push_back(cur); cur = 0; any = false; }
        else if (ansi[j] >= '0' && ansi[j] <= '9') { cur = cur * 10 + (ansi[j] - '0'); any = true; }
        ++j;
      }
      if (any || params.empty()) params.push_back(cur);
      applySgr(params, style);
      i = j + 1;
      continue;
    }
    if (c == '\r') { ++i; continue; }
    if (c == '\n') { ++row; col = 0; ++i; continue; }
    // one UTF-8 code point
    const unsigned char uc = static_cast<unsigned char>(c);
    std::size_t len = uc < 0x80 ? 1 : (uc >> 5) == 0x6 ? 2 : (uc >> 4) == 0xE ? 3 : (uc >> 3) == 0x1E ? 4 : 1;
    std::string glyph = ansi.substr(i, len);
    i += len;
    const int w = ftxui::string_width(glyph);
    auto& line = lineAt(row);
    if (!line.empty() && line.back().style == style && line.back().col + ftxui::string_width(line.back().text) == col)
      line.back().text += glyph;
    else
      line.push_back({col, glyph, style});
    col += w;
  }

  std::ofstream out(path, std::ios::binary);
  out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << width << "\" height=\"" << height
      << "\" viewBox=\"0 0 " << width << " " << height << "\" font-family=\"'JetBrains Mono','Cascadia Mono',"
      << "'Fira Code',Consolas,'DejaVu Sans Mono',monospace\" font-size=\"" << fontSize << "\">\n";
  out << "<rect width=\"100%\" height=\"100%\" rx=\"8\" fill=\"#1a1b26\"/>\n";
  for (int r = 0; r < rows; ++r) {
    const double y = pad + r * lh;
    for (const auto& sp : lineAt(r)) {
      std::string fg = sp.style.inverted ? sp.style.bg : sp.style.fg;
      std::string bg = sp.style.inverted ? sp.style.fg : sp.style.bg;
      const int w = ftxui::string_width(sp.text);
      if (bg != "#1a1b26")
        out << "<rect x=\"" << pad + sp.col * cw << "\" y=\"" << y << "\" width=\"" << w * cw << "\" height=\"" << lh
            << "\" fill=\"" << bg << "\"/>\n";
      bool blank = true;
      for (char ch : sp.text) if (ch != ' ') blank = false;
      if (blank) continue;
      out << "<text x=\"" << pad + sp.col * cw << "\" y=\"" << y + 13.5 << "\" xml:space=\"preserve\" fill=\"" << fg
          << "\"" << (sp.style.bold ? " font-weight=\"bold\"" : "") << (sp.style.dim ? " opacity=\"0.6\"" : "")
          << " textLength=\"" << w * cw << "\" lengthAdjust=\"spacingAndGlyphs\">" << escapeXml(sp.text)
          << "</text>\n";
    }
  }
  out << "</svg>\n";
  std::printf("wrote %s\n", path.c_str());
}

void shot(const std::string& dir, const char* name, ftxui::Element e, int cols, int rows) {
  auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(cols), ftxui::Dimension::Fixed(rows));
  ftxui::Render(screen, e);
  writeSvg(dir + "/" + name + ".svg", screen, cols, rows);
}

const char* kGraphQL = R"({"data":{"user":{"followers":{"totalCount":48},
 "repositories":{"totalCount":62,"nodes":[
  {"name":"k3s","stargazerCount":14,"forkCount":3,"primaryLanguage":{"name":"Go","color":"#00ADD8"}},
  {"name":"commit-msg","stargazerCount":9,"forkCount":2,"primaryLanguage":{"name":"Go","color":"#00ADD8"}},
  {"name":"itsme.dfanso.dev","stargazerCount":7,"forkCount":1,"primaryLanguage":{"name":"TypeScript","color":"#3178c6"}},
  {"name":"QuickQuest","stargazerCount":5,"forkCount":1,"primaryLanguage":{"name":"TypeScript","color":"#3178c6"}},
  {"name":"kyc","stargazerCount":3,"forkCount":0,"primaryLanguage":{"name":"Python","color":"#3572A5"}},
  {"name":"infra","stargazerCount":2,"forkCount":0,"primaryLanguage":{"name":"HCL","color":"#844FBA"}},
  {"name":"tui","stargazerCount":1,"forkCount":0,"primaryLanguage":{"name":"C++","color":"#f34b7d"}}]},
 "pinnedItems":{"nodes":[
  {"name":"k3s","description":"Kubernetes deployment with automated CI/CD, Helm and full observability","url":"https://github.com/DFanso/k3s","stargazerCount":14,"forkCount":3,"primaryLanguage":{"name":"Go","color":"#00ADD8"}},
  {"name":"commit-msg","description":"AI-powered conventional commit message generator","url":"https://github.com/DFanso/commit-msg","stargazerCount":9,"forkCount":2,"primaryLanguage":{"name":"Go","color":"#00ADD8"}}]},
 "contributionsCollection":{"totalCommitContributions":1284,"totalPullRequestContributions":96,"totalIssueContributions":21,
  "contributionCalendar":{"totalContributions":1512,"weeks":[WEEKS]}}}}})";

std::string calendarJson() {
  // 52 weeks of pseudo-random activity so the heatmap has texture.
  std::string weeks;
  unsigned seed = 7;
  for (int w = 0; w < 52; ++w) {
    weeks += "{\"contributionDays\":[";
    for (int d = 0; d < 7; ++d) {
      seed = seed * 1103515245u + 12345u;
      int count = static_cast<int>((seed >> 16) % 14);
      if (d == 0 || d == 6) count /= 3;
      weeks += "{\"contributionCount\":" + std::to_string(count) + ",\"date\":\"2026-01-01\",\"weekday\":" +
               std::to_string(d) + "}" + (d < 6 ? "," : "");
    }
    weeks += "]}" + std::string(w < 51 ? "," : "");
  }
  std::string json = kGraphQL;
  json.replace(json.find("[WEEKS]"), 7, "[" + weeks + "]");
  return json;
}

}  // namespace

int main(int argc, char** argv) {
  const std::string dir = argc > 1 ? argv[1] : "docs/screenshots";
  // Not attached to a terminal, so FTXUI would otherwise downgrade to a 16-colour palette.
  ftxui::Terminal::SetColorSupport(ftxui::Terminal::Color::TrueColor);
  using namespace itsme::app;
  const int cols = 100;

  {
    App app(Options{true, false}, nullptr);
    app.resize(cols);
    auto root = app.component();
    shot(dir, "startup", root->Render(), cols, 22);
    app.submit("help");
    shot(dir, "help", root->Render(), cols, 34);
    app.submit("clear");
    app.submit("projects");
    shot(dir, "projects", root->Render(), cols, 30);
    app.submit("n");
    app.submit("clear");
    app.submit("experience");
    shot(dir, "experience", root->Render(), cols, 30);
  }
  {
    auto fake = [](const std::string& url, const std::vector<std::string>&,
                   const std::optional<std::string>&) -> std::optional<itsme::github::HttpResponse> {
      if (url == "https://api.github.com/graphql") return itsme::github::HttpResponse{200, calendarJson()};
      return std::nullopt;
    };
    auto client = std::make_shared<const itsme::github::Client>(std::string("demo"), fake);
    App app(Options{true, false}, client);
    app.resize(cols);
    auto root = app.component();
    app.submit("clear");
    app.submit("github");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    shot(dir, "github", root->Render(), cols, 34);
  }
  {
    itsme::effects::HackSequence hack;
    hack.advance(12600);
    shot(dir, "hack", hack.render(cols), cols, 28);
    itsme::effects::MatrixRain rain(11);
    rain.resize(cols, 24);
    for (int i = 0; i < 36; ++i) rain.advance(50);
    shot(dir, "matrix", rain.render(), cols, 24);
  }
  return 0;
}
