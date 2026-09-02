# C++ TUI Portfolio — Design

**Date:** 2026-09-02
**Status:** Approved
**Source being ported:** https://github.com/DFanso/itsme.dfanso.dev (TanStack Start terminal-style portfolio)

## 1. Goal

Build a native terminal (TUI) version of the itsme.dfanso.dev portfolio in C++. It reproduces the
website's fake-shell experience — the same commands, copy, colors, easter eggs, live GitHub stats and
theatrical effects — as a single cross-platform executable. It must run as a local binary today and be
usable unchanged later as an SSH forced command (`ssh itsme.dfanso.dev`).

Non-goals: mobile layout, the CRT scanline overlay, the profile photo in `whoami`, the HTML/PDF résumé
generation (the `resume` command links to the existing PDF instead), SEO/meta content.

## 2. Decisions

| Topic | Decision |
|---|---|
| Language / standard | C++17 |
| TUI library | FTXUI (MIT), via CMake FetchContent |
| HTTP | libcurl (`find_package(CURL)`, FetchContent fallback when absent) |
| JSON | nlohmann/json (FetchContent) |
| Tests | Catch2 v3 (FetchContent) |
| Build | CMake ≥ 3.20 with presets for MSVC, GCC, Clang |
| Platforms | Windows 10+, Linux, macOS |
| Distribution | Local binary first; SSH mode later with no code change |
| GitHub data | Live: GraphQL when `GITHUB_TOKEN` is set, public REST fallback, graceful failure |
| Effects | Boot sequence, typewriter welcome, `matrix`, `hack`. No CRT overlay |
| Palette | Tokyo Night, true-color RGB; `--no-color` degrades to 16 ANSI colors |

## 3. Project layout

```
tui-itsme.dfanso.dev/
  CMakeLists.txt
  CMakePresets.json
  cmake/Dependencies.cmake        # FetchContent declarations
  src/
    main.cpp                      # CLI flags (--no-boot, --no-color, --version), builds App, runs loop
    app/
      App.hpp/.cpp                # owns TerminalState, FTXUI ScreenInteractive, event wiring
      TitleBar.hpp/.cpp           # "guest@dfanso.dev:~" + live HH:MM clock
      OutputPane.hpp/.cpp         # scrollable list of rendered Blocks
      InputLine.hpp/.cpp          # prompt + editable line + suggestions + cursor
      Prompt.hpp/.cpp             # normal prompt vs. "Would you like to see more projects? (y/n)"
    core/                         # pure logic; depends only on std + nlohmann/json
      Command.hpp                 # CommandDef, Execution, Kind, Action enums
      Commands.hpp/.cpp           # COMMANDS registry, findCommand, executeLine
      InputHelpers.hpp/.cpp       # getSuggestions, completeInput, navigateHistory, suggestClosest
      TerminalState.hpp/.cpp      # Block, TerminalState, reducer-style `submit(state, line)`
      Theme.hpp                   # Tokyo Night RGB constants + 16-color fallback map
    data/
      Portfolio.hpp/.cpp          # profile, companies[].roles[], education, certifications, skillGroups, projects
    outputs/                      # one `ftxui::Element render(...)` per command
      Welcome, Whoami, About, Projects, Skills, Experience, Education, Certifications,
      Contact, Help, Ls, Neofetch, Time, Weather, Ping, GitHubStats, TextOutput
    github/
      Model.hpp                   # GitHubStatsData, ProjectRepoStats
      Parse.hpp/.cpp              # pure: JSON string -> model (GraphQL + REST branches)
      Client.hpp/.cpp             # libcurl GET/POST with timeout; fetchStats, fetchProjectStats
      AsyncFetch.hpp              # std::future wrapper + FTXUI Post hook
    effects/
      Boot.hpp/.cpp               # "Booting portfolio OS..." staged lines
      Typewriter.hpp/.cpp         # per-character reveal state for the welcome banner
      Matrix.hpp/.cpp             # full-screen rain overlay
      Hack.hpp/.cpp               # scripted fake hacking overlay
      Ticker.hpp/.cpp             # background thread posting tick events at a fixed interval
  tests/
    CMakeLists.txt
    core/commands_test.cpp
    core/input_helpers_test.cpp
    core/terminal_state_test.cpp
    github/parse_test.cpp
    fixtures/github_graphql.json, github_rest_user.json, github_rest_repos.json, repo_stats.json
    app/smoke_test.cpp            # render one frame to an off-screen ftxui::Screen
  .github/workflows/ci.yml        # build + test matrix; release artifacts on tags
  README.md, LICENSE (MIT), .gitignore, .clang-format
```

Dependency rule: `core/`, `data/` and `github/Parse` never include FTXUI or curl. `outputs/` includes
FTXUI and `data/` but not curl. Only `github/Client` includes curl.

## 4. Core: command registry and executor

Ported 1:1 from `src/lib/commands.tsx`.

```cpp
enum class Kind { Output, Action };
enum class Action { None, Clear, Matrix, Hack, OpenResume };

struct LsEntry { std::string name, perms, note; };

struct CommandDef {
  std::string name;
  std::string description;
  std::optional<LsEntry> lsEntry;
  Kind kind;
  Action action = Action::None;
  bool hidden = false;   // resume, sudo, rm, vi, vim, nano
  bool async  = false;   // time, github
};

const std::vector<CommandDef>& commands();          // canonical order as the site
std::vector<std::string> commandNames();
const CommandDef* findCommand(std::string_view name);

struct TextOutput { Color color; std::vector<std::string> lines; };

struct Execution {
  enum class Kind { Component, Text, Action } kind;
  std::string componentName;            // Kind::Component
  std::optional<TextOutput> text;       // Text, or the echo line for actions
  Action action = Action::None;
  bool awaitProjectResponse = false;
};

struct ExecContext { bool awaitingProjectResponse; double rand; };
Execution executeLine(std::string_view raw, const ExecContext& ctx);
```

`executeLine` reproduces the site's branch order exactly: empty input → one of the 7 nudge messages
chosen by `rand`; `clear`; `resume` (action + "Opening resume..."); `sudo`; `rm` (the `-rf` + `/` or `*`
special case, else "rm: missing operand"); `vi`/`vim`/`nano`; the y/n follow-up when
`awaitingProjectResponse`; `matrix`; `hack`; registry lookup (setting `awaitProjectResponse` for
`projects`); otherwise "└─▶ Command not found: X" plus "Did you mean 'Y'?" when Levenshtein ≤ 2.

`help` lists non-hidden commands; `ls` lists commands with an `lsEntry`. Both derive from the registry.

## 5. Core: terminal state

```cpp
struct Block {
  std::string input;
  bool wasAwaitingProjectResponse;     // so scrollback shows the y/n prompt it answered
  Execution execution;
};

struct TerminalState {
  std::vector<Block> blocks;
  std::vector<std::string> history;
  int historyIndex;                    // == history.size() when not browsing
  bool awaitingProjectResponse = false;
};

// Pure: appends a Block (or clears), updates history and the y/n flag. Returns the Action
// the App must perform as a side effect (clear scroll, start overlay, open resume).
Action submit(TerminalState& state, std::string line, double rand);
```

History: every non-empty line is appended; Up/Down use `navigateHistory` semantics from the site.

## 6. UI (FTXUI)

Layout, top to bottom:

1. **TitleBar** — centered `guest@dfanso.dev:~`, right-aligned HH:MM clock refreshed each second by the
   Ticker. The three colored "window buttons" are drawn as `●` glyphs in red/yellow/green for looks only.
2. **OutputPane** — `vbox` of rendered Blocks inside a `vscroll_indicator | yframe`, auto-scrolled to
   bottom on each new block. Each Block = prompt echo line + output element.
3. **InputLine** — prompt + text with a block cursor. While typing, the unique remainder of a matching
   command is shown dimmed after the cursor (ghost completion); if several commands match, they are listed
   dimmed on the line below.

Prompt (normal): `❯ dfanso@terminal in ~/portfolio on main ` with the site's per-segment colors.
Prompt (awaiting y/n): `❯ Would you like to see more projects? (y/n) `.

Keys:

| Key | Action |
|---|---|
| Enter | submit |
| Up / Down | history |
| Tab | complete if unique match |
| Ctrl+L | clear |
| Ctrl+C, Ctrl+D | quit (screen restored) |
| Esc / any key | dismiss matrix/hack overlay |
| PgUp / PgDn, mouse wheel | scroll output |

Width: outputs use `paragraph`/`hflow` so they wrap below 80 columns instead of truncating. The banner
switches to the plain-text name below 60 columns.

## 7. Outputs

Each output is `ftxui::Element render(const Context&)` where `Context` carries terminal width and, for
async outputs, the fetch state. Copy and structure are ported from `src/components/outputs/*.tsx`:

- **Welcome** — DFANSO block-letter banner, time-of-day greeting (Good morning/afternoon/evening),
  tagline, separator, version line, hint. Typewriter applies to the text lines on first render only.
- **Whoami** — name + title (no image).
- **About** — summary paragraph.
- **Projects** — `├─▶ cat projects/<name>/ type: OPS` rows, description, tech tags joined with `│`,
  and `★ n  ⑂ n  👁 n` once per-repo stats arrive.
- **Skills** — one line per skill group.
- **Experience** — company → roles → responsibilities → tech tags, with the type badge colored.
- **Education, Certifications, Contact** — straight ports.
- **Help / Ls** — derived from the registry; `ls` uses the `perms  name  # note` column layout.
- **Neofetch** — logo art left, key/value list right (OS: Portfolio v2.4.2, etc.).
- **Time** — current local time and date, static once rendered.
- **Weather, Ping** — static copy.
- **GitHubStats** — see §8.
- **TextOutput** — colored plain lines for jokes/errors.
- **resume** — prints "Opening resume..." and `https://itsme.dfanso.dev/resume.pdf`; if
  `SSH_CONNECTION`/`SSH_TTY` are unset, launches the platform opener (`start` / `xdg-open` / `open`).

Icons from the site become short text tags; no Nerd Font requirement.

## 8. GitHub integration

Ported from `src/lib/github-fetch.ts`.

```cpp
struct GitHubStatsData { bool hasFullData; Stats stats; std::vector<LanguageStat> languageStats;
                         std::vector<TopRepo> topRepos; bool isPinned;
                         std::optional<ContributionCalendar> calendar; };

std::optional<GitHubStatsData> parseGraphQL(std::string_view json);
std::optional<GitHubStatsData> parseREST(std::string_view userJson, std::string_view reposJson);
std::optional<ProjectRepoStats> parseRepo(std::string_view json);

std::optional<GitHubStatsData> fetchStats(std::optional<std::string> token);        // GraphQL then REST
std::map<std::string, std::optional<ProjectRepoStats>> fetchProjectStats(
    const std::vector<std::string>& repos, std::optional<std::string> token);         // parallel
```

Behavior: same GraphQL query, same username (`dfansoo`), same `User-Agent`, top-8 language stats with
the same color table, pinned repos else top 6, REST `fork == false` filter. 10-second connect+total
timeout. Both sources failing → `nullopt` → the site's "Unable to fetch GitHub data" message.

Async model: `App::submit` starts the fetch on a `std::thread` returning a `std::future`; the Block
renders "Fetching GitHub data…" with a spinner driven by the Ticker; when the future is ready the Block's
data is set and `ScreenInteractive::Post` triggers a redraw. Quitting during a fetch detaches the thread
safely (shared state via `std::shared_ptr`).

Rendering: stat grid (commits/PRs/issues/contributions when `hasFullData`; repos/stars/forks/followers
always), language bar as a single `█` bar segmented by percentage plus a legend, contribution heatmap as
7 rows × up-to-52 columns of `■` in five green intensities with month labels (skipped below 80 cols),
top repos with language dot, stars and forks.

## 9. Effects

All animation is event-driven: `Ticker` posts a custom FTXUI event every N ms; components advance state
on tick and re-render. No busy loops. Any effect can be disabled with `--no-boot` (boot + typewriter).

- **Boot** — before the first frame accepts input, show `Booting portfolio OS...` then three staged lines
  (~300 ms apart), then the Welcome block. Total ≈ 900 ms, matching the site's timing.
- **Typewriter** — reveals Welcome's text lines (not the banner art) at ~15 ms/char; any key finishes
  it instantly.
- **Matrix** — full-screen overlay. Each column has a head position and speed; glyphs from a katakana +
  ASCII set; head bright white-green, trail fading through three greens. 50 ms ticks. Any key exits and
  restores the shell view.
- **Hack** — overlay that prints the HackOverlay copy line by line (~250 ms each) with a progress bar,
  ends on "ACCESS GRANTED 😈" and auto-dismisses after 1.5 s, or on any key.

## 10. Error handling

| Situation | Behavior |
|---|---|
| Unknown command | "Command not found" + closest suggestion |
| Network/API failure, rate limit | fallback message block; never throws to the loop |
| No `GITHUB_TOKEN` | REST branch, `hasFullData=false` sections hidden |
| Terminal < 60 cols | wrapped output, compact banner, heatmap hidden |
| Ctrl+C / SIGINT / SIGTERM | FTXUI restores the terminal mode and cursor |
| Non-TTY stdout (piped) | print usage hint and exit 1 |
| curl not available at build | FetchContent builds it (HTTPS via schannel on Windows, SecureTransport on macOS, OpenSSL on Linux) |

## 11. Testing

- **Unit (Catch2):** every `executeLine` branch (mirrors the site's `commands.test.tsx`), `help`/`ls`
  derivation, `getSuggestions` / `completeInput` / `navigateHistory` / `suggestClosest`, `submit` state
  transitions, GraphQL/REST/repo parsers against JSON fixtures, language-stat math.
- **Smoke:** build the App with a stubbed GitHub client, render one frame to `ftxui::Screen` at 100×30,
  assert the prompt and banner text appear.
- **Manual:** effects and resize behavior checked on Windows Terminal, GNOME Terminal, iTerm2.
- `ctest` runs everything; CI fails on any test failure or compiler warning (`-Wall -Wextra -Werror`,
  `/W4 /WX`).

## 12. Build, CI, release

- `cmake --preset default && cmake --build --preset default && ctest --preset default`.
- GitHub Actions: matrix `windows-latest` (MSVC), `ubuntu-latest` (GCC + Clang), `macos-latest`
  (AppleClang). On `v*` tags, upload `itsme-tui-<os>-<arch>` binaries to a release.
- README documents flags, `GITHUB_TOKEN`, and the SSH recipe: a dedicated user whose
  `authorized_keys`/`ForceCommand` runs the binary with a PTY (`PermitTTY yes`), no code changes required.

## 13. Open questions

None blocking. Font: block glyphs (`█ ■ ❯ ├ └ ▶`) need a Unicode-capable terminal; the README notes
Windows Terminal (not legacy conhost) is expected on Windows.
