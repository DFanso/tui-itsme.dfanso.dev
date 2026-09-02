# C++ TUI Portfolio Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A native, cross-platform C++ terminal app that reproduces the itsme.dfanso.dev fake-shell portfolio: same commands, copy, colors, easter eggs, live GitHub stats and effects.

**Architecture:** Pure logic (`core/`, `data/`, `github/Parse`, `github/Client`) has no UI or network dependency and is unit-tested with Catch2. `outputs/` turns data into FTXUI `Element`s. `app/` wires an FTXUI `ScreenInteractive` loop around a reducer-style `TerminalState`. `effects/` are tick-driven state machines rendered as overlays. libcurl lives behind an injectable `HttpFn` so the client logic is testable offline.

**Tech Stack:** C++17, CMake ≥ 3.20 (FetchContent), FTXUI v5.0.0, nlohmann/json 3.11.3, libcurl (system or FetchContent 8.10.1), Catch2 v3.7.1. Default local toolchain: Ninja + MinGW g++ 13 (this machine); CI also builds MSVC and Clang.

**Spec:** `docs/superpowers/specs/2026-09-02-cpp-tui-portfolio-design.md`

## Global Constraints

- C++17, no compiler extensions (`CMAKE_CXX_EXTENSIONS OFF`).
- Namespaces: `itsme::core`, `itsme::data`, `itsme::outputs`, `itsme::github`, `itsme::effects`, `itsme::app`.
- Dependency rule: `core/`, `data/`, `github/Parse*`, `github/Client*` must not include FTXUI or curl. `outputs/`, `app/`, `effects/` may include FTXUI, never curl. Only `github/CurlHttp.cpp` includes `<curl/curl.h>`.
- Tokyo Night palette: bg `#1a1b26`, fg `#c0caf5`, muted `#a9b1d6`, blue `#7aa2f7`, purple `#bb9af7`, green `#9ece6a`, red `#f7768e`, yellow `#e0af68`, cyan `#7dcfff`, teal `#73daca`, orange `#ff9e64`. Matrix/hack green `#00ff41`.
- All user-visible copy is ported verbatim from the site unless a task says otherwise (only difference: "click" wording becomes key wording).
- Source files are UTF-8; MSVC gets `/utf-8`. Windows console is switched to UTF-8 in `main`.
- Warnings: `-Wall -Wextra -Wpedantic` (GCC/Clang), `/W4` (MSVC); `ITSME_WERROR=ON` in CI makes them errors.
- Build/test commands from repo root (Git Bash on this machine):
  `cmake --preset default && cmake --build --preset default && ctest --preset default`
- Commit after every task with the message given; trailer `Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>`.
- Tests live in one executable `itsme_tests`; each task appends its `.cpp` to `tests/CMakeLists.txt` via `target_sources`.

---

## File Map

| Path | Responsibility |
|---|---|
| `CMakeLists.txt`, `CMakePresets.json`, `cmake/*.cmake` | build, deps, warnings |
| `src/core/Version.hpp/.cpp` | version string |
| `src/core/Strings.hpp` | trim/lower/startsWith/utf8 helpers (header-only) |
| `src/core/Command.hpp` | `Tone`, `CommandKind`, `Action`, `CommandDef`, `TextOutput`, `Execution` |
| `src/core/Commands.hpp/.cpp` | registry + `executeLine` |
| `src/core/InputHelpers.hpp/.cpp` | suggestions, completion, history, Levenshtein |
| `src/core/LineEditor.hpp/.cpp` | cursor/text editing model for the input line |
| `src/core/TerminalState.hpp/.cpp` | `Block`, `TerminalState`, `submit`, `initialState` |
| `src/core/AsyncValue.hpp` | thread-backed future with completion callback |
| `src/data/Portfolio.hpp/.cpp` | all résumé/projects/skills/contact data |
| `src/github/Model.hpp` | GitHub data structs |
| `src/github/Parse.hpp/.cpp` | JSON → model |
| `src/github/Client.hpp/.cpp` | fetch orchestration over injectable `HttpFn` |
| `src/github/CurlHttp.hpp/.cpp` | libcurl `HttpFn` |
| `src/outputs/Theme.hpp/.cpp` | palette, `tone()`, hex→Color, true-color toggle |
| `src/outputs/Common.hpp/.cpp` | heading/branch/tag helpers |
| `src/outputs/Outputs.hpp` + one `.cpp` per command | renderers |
| `src/effects/Ticker.hpp/.cpp` | background tick thread with adjustable interval |
| `src/effects/Boot.hpp/.cpp`, `Typewriter.hpp/.cpp`, `Matrix.hpp/.cpp`, `Hack.hpp/.cpp` | effect state machines + renderers |
| `src/app/Prompt`, `TitleBar`, `Clock`, `Opener`, `BlockRenderer`, `App` | UI shell |
| `src/main.cpp` | flags, TTY check, wiring |
| `tests/**` | Catch2 tests + fixtures |
| `.github/workflows/ci.yml`, `README.md`, `LICENSE` | CI, docs |

---

### Task 1: Build skeleton, dependencies, first test

**Files:**
- Create: `CMakeLists.txt`, `CMakePresets.json`, `cmake/Warnings.cmake`, `cmake/Dependencies.cmake`
- Create: `src/core/Version.hpp`, `src/core/Version.cpp`, `src/main.cpp`
- Create: `tests/CMakeLists.txt`, `tests/sanity_test.cpp`
- Create: `LICENSE` (MIT, "Copyright (c) 2026 Leo Felcianas"), `README.md` (title + one-line description + build command)

**Interfaces:**
- Produces: CMake targets `itsme_core` (static lib, public include dir `src`, links nlohmann_json), `itsme` (exe), `itsme_tests` (Catch2). CMake function `itsme_set_warnings(target)`. `const char* itsme::version()`.

- [ ] **Step 1: Write the failing test**

`tests/sanity_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include <string>
#include "core/Version.hpp"

TEST_CASE("version string matches the CMake project version") {
  REQUIRE(std::string(itsme::version()) == "0.1.0");
}
```

`tests/CMakeLists.txt`:
```cmake
add_executable(itsme_tests sanity_test.cpp)
target_link_libraries(itsme_tests PRIVATE itsme_core Catch2::Catch2WithMain)
itsme_set_warnings(itsme_tests)
include(Catch)
catch_discover_tests(itsme_tests)
```

- [ ] **Step 2: Write the build files**

`cmake/Warnings.cmake`:
```cmake
function(itsme_set_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /utf-8 /permissive-
      $<$<BOOL:${ITSME_WERROR}>:/WX>)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic
      $<$<BOOL:${ITSME_WERROR}>:-Werror>)
  endif()
endfunction()
```

`cmake/Dependencies.cmake`:
```cmake
include(FetchContent)

set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG v5.0.0
  GIT_SHALLOW TRUE)

set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_Install OFF CACHE BOOL "" FORCE)
FetchContent_Declare(nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)

FetchContent_MakeAvailable(ftxui nlohmann_json)

if(ITSME_BUILD_TESTS)
  set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
  set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(Catch2)
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
endif()
```

`CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.20)
project(itsme_tui VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ITSME_BUILD_TESTS "Build unit tests" ON)
option(ITSME_WERROR "Treat warnings as errors" OFF)

include(cmake/Warnings.cmake)
include(cmake/Dependencies.cmake)

add_library(itsme_core STATIC
  src/core/Version.cpp)
target_include_directories(itsme_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(itsme_core PUBLIC nlohmann_json::nlohmann_json)
target_compile_definitions(itsme_core PUBLIC ITSME_VERSION="${PROJECT_VERSION}")
itsme_set_warnings(itsme_core)

add_executable(itsme src/main.cpp)
target_link_libraries(itsme PRIVATE itsme_core)
itsme_set_warnings(itsme)

if(ITSME_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
```

`CMakePresets.json`:
```json
{
  "version": 3,
  "cmakeMinimumRequired": { "major": 3, "minor": 20, "patch": 0 },
  "configurePresets": [
    { "name": "default", "displayName": "Ninja (compiler from PATH)", "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/default",
      "cacheVariables": { "CMAKE_BUILD_TYPE": "RelWithDebInfo" } },
    { "name": "release", "inherits": "default", "binaryDir": "${sourceDir}/build/release",
      "cacheVariables": { "CMAKE_BUILD_TYPE": "Release", "ITSME_BUILD_TESTS": "OFF" } },
    { "name": "ci", "inherits": "default", "binaryDir": "${sourceDir}/build/ci",
      "cacheVariables": { "ITSME_WERROR": "ON" } },
    { "name": "msvc", "displayName": "Visual Studio 2022 x64", "generator": "Visual Studio 17 2022",
      "architecture": "x64", "binaryDir": "${sourceDir}/build/msvc",
      "cacheVariables": { "ITSME_WERROR": "ON" } }
  ],
  "buildPresets": [
    { "name": "default", "configurePreset": "default" },
    { "name": "release", "configurePreset": "release" },
    { "name": "ci", "configurePreset": "ci" },
    { "name": "msvc", "configurePreset": "msvc", "configuration": "Release" }
  ],
  "testPresets": [
    { "name": "default", "configurePreset": "default", "output": { "outputOnFailure": true } },
    { "name": "ci", "configurePreset": "ci", "output": { "outputOnFailure": true } },
    { "name": "msvc", "configurePreset": "msvc", "configuration": "Release", "output": { "outputOnFailure": true } }
  ]
}
```

`src/core/Version.hpp`:
```cpp
#pragma once
namespace itsme {
const char* version();
}
```

`src/core/Version.cpp`:
```cpp
#include "core/Version.hpp"
namespace itsme {
const char* version() { return ITSME_VERSION; }
}
```

`src/main.cpp` (temporary body, replaced in Task 9):
```cpp
#include <cstdio>
#include "core/Version.hpp"

int main() {
  std::printf("itsme %s\n", itsme::version());
  return 0;
}
```

- [ ] **Step 3: Configure, build, run tests**

Run: `cmake --preset default && cmake --build --preset default && ctest --preset default`
Expected: FetchContent downloads FTXUI, json, Catch2; build succeeds; `1 tests passed`.
Run: `./build/default/itsme.exe` (or `./build/default/itsme`) → prints `itsme 0.1.0`.

- [ ] **Step 4: Commit**

```bash
git add -A
git commit -m "build: CMake skeleton with FTXUI, nlohmann/json, Catch2"
```

---

### Task 2: Command types, string helpers, registry

**Files:**
- Create: `src/core/Strings.hpp`, `src/core/Command.hpp`, `src/core/Commands.hpp`, `src/core/Commands.cpp`
- Modify: `CMakeLists.txt` (add `src/core/Commands.cpp` to `itsme_core`)
- Test: `tests/core/commands_test.cpp`, modify `tests/CMakeLists.txt`

**Interfaces:**
- Produces:
  - `itsme::core::Tone { Fg, Muted, Blue, Purple, Green, Red, Yellow, Cyan, Teal, Orange }`
  - `CommandKind { Output, Action }`, `Action { None, Clear, Matrix, Hack, OpenResume }`, `ExecKind { Component, Text, Action }`
  - `struct LsEntry { std::string name, perms, note; }`
  - `struct CommandDef { name, description, std::optional<LsEntry> lsEntry, kind, action, hidden, async }`
  - `struct TextOutput { Tone tone; std::vector<std::string> lines; }`
  - `struct Execution { ExecKind kind; std::string componentName; std::optional<TextOutput> text; Action action; bool awaitProjectResponse; }`
  - `const std::vector<CommandDef>& commands()`, `std::vector<std::string> commandNames()`, `const CommandDef* findCommand(std::string_view)`
  - Strings: `trim`, `toLower`, `startsWith`, `contains`, `utf8Chars(std::string_view) -> std::vector<std::string>`

- [ ] **Step 1: Write the failing tests**

`tests/core/commands_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
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
  auto hiddenCount = std::count_if(all.begin(), all.end(), [](auto& c) { return c.hidden; });
  auto lsCount = std::count_if(all.begin(), all.end(), [](auto& c) { return c.lsEntry.has_value(); });
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
```

Append to `tests/CMakeLists.txt` (after `add_executable`):
```cmake
target_sources(itsme_tests PRIVATE core/commands_test.cpp)
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `cmake --build --preset default`
Expected: compile error, `core/Commands.hpp: No such file`.

- [ ] **Step 3: Implement**

`src/core/Strings.hpp`:
```cpp
#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace itsme::core {

inline std::string trim(std::string_view s) {
  auto notSpace = [](unsigned char c) { return !std::isspace(c); };
  auto b = std::find_if(s.begin(), s.end(), notSpace);
  auto e = std::find_if(s.rbegin(), s.rend(), notSpace).base();
  return b < e ? std::string(b, e) : std::string();
}

inline std::string toLower(std::string_view s) {
  std::string out(s);
  std::transform(out.begin(), out.end(), out.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return out;
}

inline bool startsWith(std::string_view s, std::string_view prefix) {
  return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

inline bool contains(std::string_view s, std::string_view needle) {
  return s.find(needle) != std::string_view::npos;
}

// Splits a UTF-8 string into one std::string per code point.
inline std::vector<std::string> utf8Chars(std::string_view s) {
  std::vector<std::string> out;
  for (std::size_t i = 0; i < s.size();) {
    unsigned char c = static_cast<unsigned char>(s[i]);
    std::size_t len = c < 0x80 ? 1 : (c >> 5) == 0x6 ? 2 : (c >> 4) == 0xE ? 3 : (c >> 3) == 0x1E ? 4 : 1;
    if (i + len > s.size()) len = s.size() - i;
    out.emplace_back(s.substr(i, len));
    i += len;
  }
  return out;
}

}  // namespace itsme::core
```

`src/core/Command.hpp`:
```cpp
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
```

`src/core/Commands.hpp`:
```cpp
#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "core/Command.hpp"

namespace itsme::core {

const std::vector<CommandDef>& commands();
std::vector<std::string> commandNames();
const CommandDef* findCommand(std::string_view name);

struct ExecContext {
  bool awaitingProjectResponse = false;
  double rand = 0.0;  // [0,1), selects the empty-input nudge
};

Execution executeLine(std::string_view raw, const ExecContext& ctx);

}  // namespace itsme::core
```

`src/core/Commands.cpp` (registry part; `executeLine` is added in Task 4):
```cpp
#include "core/Commands.hpp"
#include "core/Strings.hpp"

namespace itsme::core {

namespace {
LsEntry dir(const char* name, const char* note) { return {name, "drwxr-xr-x", note}; }
LsEntry file(const char* name, const char* note) { return {name, "-rw-r--r--", note}; }

std::vector<CommandDef> buildRegistry() {
  std::vector<CommandDef> v;
  v.push_back({"ls", "List available sections and commands", std::nullopt, CommandKind::Output});
  v.push_back({"welcome", "Display welcome message and ASCII art", file("welcome.txt", "welcome message"), CommandKind::Output});
  v.push_back({"whoami", "Show detailed profile information", file("whoami.txt", "profile info"), CommandKind::Output});
  v.push_back({"about", "View my professional summary", dir("about/", "professional summary"), CommandKind::Output});
  v.push_back({"projects", "Browse my featured projects", dir("projects/", "featured work"), CommandKind::Output});
  v.push_back({"skills", "List technical skills and expertise", dir("skills/", "technical expertise"), CommandKind::Output});
  v.push_back({"experience", "View work history and roles", dir("experience/", "work history"), CommandKind::Output});
  v.push_back({"education", "View academic background", dir("education/", "academic background"), CommandKind::Output});
  v.push_back({"certifications", "View professional certificates", dir("certifications/", "licenses & certs"), CommandKind::Output});
  v.push_back({"contact", "Get my contact information", dir("contact/", "social links"), CommandKind::Output});
  v.push_back({"clear", "Clear terminal screen", std::nullopt, CommandKind::Action, Action::Clear});
  v.push_back({"help", "Show this help message", std::nullopt, CommandKind::Output});
  v.push_back({"neofetch", "Display system information", std::nullopt, CommandKind::Output});
  v.push_back({"time", "Show current time", std::nullopt, CommandKind::Output, Action::None, false, true});
  v.push_back({"weather", "Check the weather (sort of)", std::nullopt, CommandKind::Output});
  v.push_back({"matrix", "Full-screen matrix rain simulation", std::nullopt, CommandKind::Action, Action::Matrix});
  v.push_back({"hack", "Initiate a totally real hacking sequence 😈", std::nullopt, CommandKind::Action, Action::Hack});
  v.push_back({"ping", "Test connection to dfanso.dev", std::nullopt, CommandKind::Output});
  v.push_back({"github", "Show GitHub stats and contributions", dir("github/", "stats & contributions"), CommandKind::Output, Action::None, false, true});
  // Hidden / easter eggs
  v.push_back({"resume", "Open the resume PDF", file("resume.pdf", "curriculum vitae"), CommandKind::Action, Action::OpenResume, true});
  v.push_back({"sudo", "Attempt to gain superuser privileges", std::nullopt, CommandKind::Action, Action::None, true});
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
```

In `CMakeLists.txt`, change the `itsme_core` sources to:
```cmake
add_library(itsme_core STATIC
  src/core/Version.cpp
  src/core/Commands.cpp)
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cmake --build --preset default && ctest --preset default`
Expected: all tests pass (5 total).

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat(core): command registry and string helpers"
```

---

### Task 3: Input helpers (suggestions, completion, history, Levenshtein)

**Files:**
- Create: `src/core/InputHelpers.hpp`, `src/core/InputHelpers.cpp`
- Modify: `CMakeLists.txt` (add source), `tests/CMakeLists.txt`
- Test: `tests/core/input_helpers_test.cpp`

**Interfaces:**
- Produces:
  - `std::vector<std::string> getSuggestions(std::string_view input, const std::vector<std::string>& names)`
  - `std::optional<std::string> completeInput(std::string_view input, const std::vector<std::string>& names)`
  - `enum class HistoryDir { Up, Down }`, `struct HistoryNav { std::size_t index; std::string value; }`
  - `HistoryNav navigateHistory(const std::vector<std::string>& history, std::size_t index, HistoryDir dir)`
  - `int levenshtein(std::string_view a, std::string_view b)`
  - `std::optional<std::string> suggestClosest(std::string_view input, const std::vector<std::string>& names)`

- [ ] **Step 1: Write the failing tests**

`tests/core/input_helpers_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include "core/InputHelpers.hpp"

using namespace itsme::core;
static const std::vector<std::string> kNames = {"help", "hack", "about", "projects", "ping"};

TEST_CASE("getSuggestions returns prefix matches, excluding exact match") {
  CHECK(getSuggestions("", kNames).empty());
  CHECK(getSuggestions("h", kNames) == std::vector<std::string>{"help", "hack"});
  CHECK(getSuggestions("HE", kNames) == std::vector<std::string>{"help"});
  CHECK(getSuggestions("help", kNames).empty());
}

TEST_CASE("completeInput completes only a unique prefix") {
  CHECK(completeInput("ab", kNames) == std::optional<std::string>{"about"});
  CHECK_FALSE(completeInput("h", kNames).has_value());
  CHECK_FALSE(completeInput("zz", kNames).has_value());
  CHECK(completeInput("PING", kNames) == std::optional<std::string>{"ping"});
}

TEST_CASE("navigateHistory mirrors the site's arrow semantics") {
  std::vector<std::string> h = {"a", "b", "c"};
  auto up = navigateHistory(h, 3, HistoryDir::Up);
  CHECK(up.index == 2); CHECK(up.value == "c");
  up = navigateHistory(h, 0, HistoryDir::Up);
  CHECK(up.index == 0); CHECK(up.value == "a");
  auto down = navigateHistory(h, 1, HistoryDir::Down);
  CHECK(down.index == 2); CHECK(down.value == "c");
  down = navigateHistory(h, 2, HistoryDir::Down);
  CHECK(down.index == 3); CHECK(down.value == "");
  auto empty = navigateHistory({}, 0, HistoryDir::Up);
  CHECK(empty.index == 0); CHECK(empty.value == "");
}

TEST_CASE("levenshtein and suggestClosest") {
  CHECK(levenshtein("", "") == 0);
  CHECK(levenshtein("kitten", "sitting") == 3);
  CHECK(levenshtein("abc", "") == 3);
  CHECK(suggestClosest("hlep", kNames) == std::optional<std::string>{"help"});
  CHECK(suggestClosest("PROJECT", kNames) == std::optional<std::string>{"projects"});
  CHECK_FALSE(suggestClosest("xyzxyz", kNames).has_value());
  CHECK_FALSE(suggestClosest("a", {}).has_value());
}
```

`tests/CMakeLists.txt`: add `target_sources(itsme_tests PRIVATE core/input_helpers_test.cpp)`.

- [ ] **Step 2: Run tests to verify they fail**

Run: `cmake --build --preset default`
Expected: `core/InputHelpers.hpp: No such file`.

- [ ] **Step 3: Implement**

`src/core/InputHelpers.hpp`:
```cpp
#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace itsme::core {

std::vector<std::string> getSuggestions(std::string_view input, const std::vector<std::string>& names);
std::optional<std::string> completeInput(std::string_view input, const std::vector<std::string>& names);

enum class HistoryDir { Up, Down };
struct HistoryNav {
  std::size_t index;
  std::string value;
};
HistoryNav navigateHistory(const std::vector<std::string>& history, std::size_t index, HistoryDir dir);

int levenshtein(std::string_view a, std::string_view b);
std::optional<std::string> suggestClosest(std::string_view input, const std::vector<std::string>& names);

}  // namespace itsme::core
```

`src/core/InputHelpers.cpp`:
```cpp
#include "core/InputHelpers.hpp"
#include <algorithm>
#include <limits>
#include "core/Strings.hpp"

namespace itsme::core {

std::vector<std::string> getSuggestions(std::string_view input, const std::vector<std::string>& names) {
  std::vector<std::string> out;
  if (input.empty()) return out;
  const std::string lower = toLower(input);
  for (const auto& n : names)
    if (startsWith(n, lower) && n != lower) out.push_back(n);
  return out;
}

std::optional<std::string> completeInput(std::string_view input, const std::vector<std::string>& names) {
  const std::string lower = toLower(input);
  std::vector<std::string> matches;
  for (const auto& n : names)
    if (startsWith(n, lower)) matches.push_back(n);
  if (matches.size() == 1) return matches.front();
  return std::nullopt;
}

HistoryNav navigateHistory(const std::vector<std::string>& history, std::size_t index, HistoryDir dir) {
  if (dir == HistoryDir::Up) {
    const std::size_t newIndex = index > 0 ? index - 1 : 0;
    return {newIndex, newIndex < history.size() ? history[newIndex] : std::string()};
  }
  const std::size_t newIndex = (index + 1 < history.size()) ? index + 1 : history.size();
  return {newIndex, newIndex < history.size() ? history[newIndex] : std::string()};
}

int levenshtein(std::string_view a, std::string_view b) {
  std::vector<int> prev(b.size() + 1), cur(b.size() + 1);
  for (std::size_t j = 0; j <= b.size(); ++j) prev[j] = static_cast<int>(j);
  for (std::size_t i = 1; i <= a.size(); ++i) {
    cur[0] = static_cast<int>(i);
    for (std::size_t j = 1; j <= b.size(); ++j) {
      const int sub = prev[j - 1] + (a[i - 1] == b[j - 1] ? 0 : 1);
      cur[j] = std::min({sub, prev[j] + 1, cur[j - 1] + 1});
    }
    std::swap(prev, cur);
  }
  return prev[b.size()];
}

std::optional<std::string> suggestClosest(std::string_view input, const std::vector<std::string>& names) {
  const std::string lower = toLower(input);
  std::optional<std::string> best;
  int bestDistance = std::numeric_limits<int>::max();
  for (const auto& n : names) {
    const int d = levenshtein(lower, n);
    if (d < bestDistance) {
      bestDistance = d;
      best = n;
    }
  }
  if (best && bestDistance <= 2) return best;
  return std::nullopt;
}

}  // namespace itsme::core
```

Add `src/core/InputHelpers.cpp` to `itsme_core` in `CMakeLists.txt`.

- [ ] **Step 4: Run tests to verify they pass**

Run: `cmake --build --preset default && ctest --preset default`
Expected: all pass.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat(core): input helpers (suggestions, completion, history, did-you-mean)"
```

---

### Task 4: `executeLine` dispatcher

**Files:**
- Modify: `src/core/Commands.cpp` (append `executeLine` + nudge messages)
- Test: `tests/core/execute_line_test.cpp`, modify `tests/CMakeLists.txt`

**Interfaces:**
- Consumes: `findCommand`, `commandNames`, `suggestClosest`, `Strings.hpp`.
- Produces: `Execution executeLine(std::string_view raw, const ExecContext& ctx)` (declared in Task 2).

- [ ] **Step 1: Write the failing tests**

`tests/core/execute_line_test.cpp`:
```cpp
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
  for (auto ed : {"vi", "vim", "nano"}) {
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
```

`tests/CMakeLists.txt`: add `target_sources(itsme_tests PRIVATE core/execute_line_test.cpp)`.

- [ ] **Step 2: Run tests to verify they fail**

Run: `cmake --build --preset default`
Expected: link error, undefined reference to `executeLine`.

- [ ] **Step 3: Implement**

Append to `src/core/Commands.cpp` inside `namespace itsme::core` (add `#include "core/InputHelpers.hpp"`, `#include <array>`, `#include <cmath>` at top):
```cpp
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
    double r = ctx.rand < 0.0 ? 0.0 : ctx.rand;
    auto idx = static_cast<std::size_t>(std::floor(r * kEmptyInputMessages.size()));
    if (idx >= kEmptyInputMessages.size()) idx = kEmptyInputMessages.size() - 1;
    return textExec(Tone::Purple, {kEmptyInputMessages[idx]});
  }

  const std::string cmd = command.substr(0, command.find(' '));

  if (cmd == "clear") return actionExec(Action::Clear, std::nullopt);
  if (cmd == "resume") return actionExec(Action::OpenResume, TextOutput{Tone::Fg, {"Opening resume..."}});

  if (command == "sudo")
    return textExec(Tone::Red, {"Permission denied: You are not in the sudoers file. This incident will be reported to Santa Claus. 🎅"});

  if (startsWith(command, "rm")) {
    if (contains(command, "-rf") && (contains(command, "/") || contains(command, "*")))
      return textExec(Tone::Red, {"⚠️ CRITICAL ERROR: Nice try! But I can't let you delete my portfolio."});
    return textExec(Tone::Red, {"rm: missing operand"});
  }

  if (command == "vi" || command == "vim" || command == "nano")
    return textExec(Tone::Yellow, {"Error: Text editor functionality not implemented yet. Try 'code .' instead? 😉"});

  if (ctx.awaitingProjectResponse) {
    if (command == "y" || command == "yes")
      return textExec(Tone::Green, {"🔗 Check out more of my projects:", "└─▶ https://github.com/dfansoo"});
    if (command == "n" || command == "no")
      return textExec(Tone::Green, {"└─▶ Alright! Feel free to explore other commands using `help`."});
    Execution e = textExec(Tone::Red, {"└─▶ Please answer with y or n."});
    e.awaitProjectResponse = true;
    return e;
  }

  if (cmd == "matrix")
    return actionExec(Action::Matrix, TextOutput{Tone::Green, {"Launching matrix simulation... (press ESC or any key to exit)"}});
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
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cmake --build --preset default && ctest --preset default`
Expected: all pass.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat(core): executeLine dispatcher ported from commands.tsx"
```

---

### Task 5: Terminal state reducer and line editor

**Files:**
- Create: `src/core/TerminalState.hpp`, `src/core/TerminalState.cpp`, `src/core/LineEditor.hpp`, `src/core/LineEditor.cpp`
- Modify: `CMakeLists.txt`, `tests/CMakeLists.txt`
- Test: `tests/core/terminal_state_test.cpp`, `tests/core/line_editor_test.cpp`

**Interfaces:**
- Produces:
  - `struct Block { int id; std::string input; bool seeded; bool wasAwaitingProjectResponse; Execution execution; }`
  - `struct TerminalState { std::vector<Block> blocks; std::vector<std::string> history; std::size_t historyIndex; bool awaitingProjectResponse; int nextId; }`
  - `TerminalState initialState()` — seeded with `welcome` (id 0) and `whoami` (id 1) blocks
  - `Action submit(TerminalState&, std::string_view raw, double rand)` — returns the side-effect action for the App
  - `void clearBlocks(TerminalState&)` — Ctrl+L
  - `class LineEditor { text(), cursor(), insert(std::string_view), backspace(), del(), left(), right(), home(), end(), set(std::string), clear() }` — cursor is a code-point index.

- [ ] **Step 1: Write the failing tests**

`tests/core/terminal_state_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include "core/TerminalState.hpp"

using namespace itsme::core;

TEST_CASE("initial state seeds welcome and whoami") {
  auto s = initialState();
  REQUIRE(s.blocks.size() == 2);
  CHECK(s.blocks[0].execution.componentName == "welcome");
  CHECK(s.blocks[1].execution.componentName == "whoami");
  CHECK(s.blocks[0].seeded);
  CHECK(s.nextId == 2);
  CHECK(s.history.empty());
  CHECK(s.historyIndex == 0);
}

TEST_CASE("submit appends a block, records history, returns the action") {
  auto s = initialState();
  CHECK(submit(s, "  About ", 0.0) == Action::None);
  REQUIRE(s.blocks.size() == 3);
  CHECK(s.blocks[2].id == 2);
  CHECK(s.blocks[2].input == "About");
  CHECK_FALSE(s.blocks[2].seeded);
  CHECK(s.history == std::vector<std::string>{"about"});
  CHECK(s.historyIndex == 1);
}

TEST_CASE("empty input adds a nudge block but no history") {
  auto s = initialState();
  submit(s, "", 0.0);
  CHECK(s.blocks.size() == 3);
  CHECK(s.history.empty());
}

TEST_CASE("clear wipes blocks and does not append") {
  auto s = initialState();
  submit(s, "about", 0.0);
  CHECK(submit(s, "clear", 0.0) == Action::Clear);
  CHECK(s.blocks.empty());
  CHECK(s.history == std::vector<std::string>{"about", "clear"});
  clearBlocks(s);
  CHECK(s.blocks.empty());
}

TEST_CASE("projects toggles awaiting flag and snapshot is kept on the answer block") {
  auto s = initialState();
  submit(s, "projects", 0.0);
  CHECK(s.awaitingProjectResponse);
  submit(s, "y", 0.0);
  CHECK_FALSE(s.awaitingProjectResponse);
  CHECK(s.blocks.back().wasAwaitingProjectResponse);
  CHECK_FALSE(s.blocks[s.blocks.size() - 2].wasAwaitingProjectResponse);
}

TEST_CASE("matrix returns its action and still appends the echo block") {
  auto s = initialState();
  CHECK(submit(s, "matrix", 0.0) == Action::Matrix);
  CHECK(s.blocks.back().execution.action == Action::Matrix);
}
```

`tests/core/line_editor_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include "core/LineEditor.hpp"

using itsme::core::LineEditor;

TEST_CASE("insert and cursor movement over code points") {
  LineEditor ed;
  ed.insert("ab");
  CHECK(ed.text() == "ab"); CHECK(ed.cursor() == 2);
  ed.left(); ed.insert("❯");
  CHECK(ed.text() == "a❯b"); CHECK(ed.cursor() == 2);
  ed.home(); CHECK(ed.cursor() == 0);
  ed.end(); CHECK(ed.cursor() == 3);
  ed.right(); CHECK(ed.cursor() == 3);
}

TEST_CASE("backspace and delete") {
  LineEditor ed;
  ed.set("héllo");
  CHECK(ed.cursor() == 5);
  ed.backspace(); CHECK(ed.text() == "héll");
  ed.home(); ed.backspace(); CHECK(ed.text() == "héll");
  ed.right(); ed.del(); CHECK(ed.text() == "hll"); CHECK(ed.cursor() == 1);
  ed.clear(); CHECK(ed.text().empty()); CHECK(ed.cursor() == 0);
}

TEST_CASE("split around cursor") {
  LineEditor ed;
  ed.set("abc"); ed.left();
  CHECK(ed.before() == "ab");
  CHECK(ed.at() == "c");
  CHECK(ed.after() == "");
  ed.end();
  CHECK(ed.at() == "");
}
```

`tests/CMakeLists.txt`: add both files via `target_sources`.

- [ ] **Step 2: Run tests to verify they fail**

Run: `cmake --build --preset default` → missing headers.

- [ ] **Step 3: Implement**

`src/core/TerminalState.hpp`:
```cpp
#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
#include "core/Command.hpp"

namespace itsme::core {

struct Block {
  int id = 0;
  std::string input;
  bool seeded = false;                     // rendered without a prompt echo
  bool wasAwaitingProjectResponse = false; // echo shows the y/n prompt
  Execution execution;
};

struct TerminalState {
  std::vector<Block> blocks;
  std::vector<std::string> history;
  std::size_t historyIndex = 0;  // == history.size() when not browsing
  bool awaitingProjectResponse = false;
  int nextId = 0;
};

TerminalState initialState();
Action submit(TerminalState& state, std::string_view raw, double rand);
void clearBlocks(TerminalState& state);

}  // namespace itsme::core
```

`src/core/TerminalState.cpp`:
```cpp
#include "core/TerminalState.hpp"
#include "core/Commands.hpp"
#include "core/Strings.hpp"

namespace itsme::core {

namespace {
Block seed(int id, const char* name) {
  Block b;
  b.id = id;
  b.input = name;
  b.seeded = true;
  b.execution.kind = ExecKind::Component;
  b.execution.componentName = name;
  return b;
}
}  // namespace

TerminalState initialState() {
  TerminalState s;
  s.blocks.push_back(seed(0, "welcome"));
  s.blocks.push_back(seed(1, "whoami"));
  s.nextId = 2;
  return s;
}

Action submit(TerminalState& state, std::string_view raw, double rand) {
  const std::string trimmed = trim(raw);
  const bool wasAwaiting = state.awaitingProjectResponse;
  Execution exec = executeLine(raw, ExecContext{state.awaitingProjectResponse, rand});

  if (!trimmed.empty()) state.history.push_back(toLower(trimmed));
  state.historyIndex = state.history.size();
  state.awaitingProjectResponse = exec.awaitProjectResponse;

  if (exec.action == Action::Clear) {
    state.blocks.clear();
    return Action::Clear;
  }

  const Action action = exec.action;
  Block b;
  b.id = state.nextId++;
  b.input = trimmed;
  b.wasAwaitingProjectResponse = wasAwaiting;
  b.execution = std::move(exec);
  state.blocks.push_back(std::move(b));
  return action;
}

void clearBlocks(TerminalState& state) { state.blocks.clear(); }

}  // namespace itsme::core
```

`src/core/LineEditor.hpp`:
```cpp
#pragma once
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace itsme::core {

// Text + cursor model for the input line. Cursor is an index in code points.
class LineEditor {
 public:
  const std::string& text() const { return text_; }
  std::size_t cursor() const { return cursor_; }
  std::size_t length() const { return chars_.size(); }

  void insert(std::string_view s);
  void backspace();
  void del();
  void left();
  void right();
  void home();
  void end();
  void set(std::string s);  // replaces text, cursor at end
  void clear();

  std::string before() const;  // text before cursor
  std::string at() const;      // code point under cursor ("" at end)
  std::string after() const;   // text after the cursor code point

 private:
  void rebuild();
  std::string text_;
  std::vector<std::string> chars_;
  std::size_t cursor_ = 0;
};

}  // namespace itsme::core
```

`src/core/LineEditor.cpp`:
```cpp
#include "core/LineEditor.hpp"
#include "core/Strings.hpp"

namespace itsme::core {

void LineEditor::rebuild() {
  text_.clear();
  for (const auto& c : chars_) text_ += c;
  if (cursor_ > chars_.size()) cursor_ = chars_.size();
}

void LineEditor::insert(std::string_view s) {
  auto incoming = utf8Chars(s);
  chars_.insert(chars_.begin() + static_cast<std::ptrdiff_t>(cursor_), incoming.begin(), incoming.end());
  cursor_ += incoming.size();
  rebuild();
}

void LineEditor::backspace() {
  if (cursor_ == 0) return;
  chars_.erase(chars_.begin() + static_cast<std::ptrdiff_t>(cursor_ - 1));
  --cursor_;
  rebuild();
}

void LineEditor::del() {
  if (cursor_ >= chars_.size()) return;
  chars_.erase(chars_.begin() + static_cast<std::ptrdiff_t>(cursor_));
  rebuild();
}

void LineEditor::left() { if (cursor_ > 0) --cursor_; }
void LineEditor::right() { if (cursor_ < chars_.size()) ++cursor_; }
void LineEditor::home() { cursor_ = 0; }
void LineEditor::end() { cursor_ = chars_.size(); }

void LineEditor::set(std::string s) {
  chars_ = utf8Chars(s);
  cursor_ = chars_.size();
  rebuild();
}

void LineEditor::clear() {
  chars_.clear();
  cursor_ = 0;
  rebuild();
}

std::string LineEditor::before() const {
  std::string out;
  for (std::size_t i = 0; i < cursor_ && i < chars_.size(); ++i) out += chars_[i];
  return out;
}

std::string LineEditor::at() const { return cursor_ < chars_.size() ? chars_[cursor_] : std::string(); }

std::string LineEditor::after() const {
  std::string out;
  for (std::size_t i = cursor_ + 1; i < chars_.size(); ++i) out += chars_[i];
  return out;
}

}  // namespace itsme::core
```

Add `src/core/TerminalState.cpp` and `src/core/LineEditor.cpp` to `itsme_core`.

- [ ] **Step 4: Run tests to verify they pass**

Run: `cmake --build --preset default && ctest --preset default` → all pass.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat(core): terminal state reducer and UTF-8 line editor"
```

---

### Task 6: Portfolio data

**Files:**
- Create: `src/data/Portfolio.hpp`, `src/data/Portfolio.cpp`
- Modify: `CMakeLists.txt`, `tests/CMakeLists.txt`
- Test: `tests/data/portfolio_test.cpp`

**Interfaces:**
- Produces (namespace `itsme::data`):
  - `struct Profile { name, title, tagline, location, email, linkedin, linkedinHandle, github, githubHandle, portfolio, portfolioHandle, resumeUrl, summary }`
  - `struct Role { std::string type; core::Tone typeTone; std::string title, period; std::vector<std::string> responsibilities, tech; }`
  - `struct Company { std::string company, totalPeriod; std::vector<Role> roles; }`
  - `struct EducationEntry { degree, institution, grade, period, location }`
  - `struct Certification { name, issuer, date, expiry, id }` (empty string = absent)
  - `struct SkillCategory { std::string name; std::vector<std::string> skills; }`
  - `struct Project { std::string type, name, url; std::optional<std::string> github; std::string description; std::vector<std::string> tech; }`
  - `struct ContactLink { std::string id, name; std::optional<std::string> url; }`
  - Accessors: `profile()`, `companies()`, `education()`, `certifications()`, `skillCategories()`, `projects()`, `contactLinks()` — all return const refs to statics.

- [ ] **Step 1: Write the failing test**

`tests/data/portfolio_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include "data/Portfolio.hpp"

using namespace itsme::data;

TEST_CASE("profile basics") {
  CHECK(profile().name == "Leo Felcianas");
  CHECK(profile().githubHandle == "github.com/dfansoo");
  CHECK(profile().resumeUrl == "https://itsme.dfanso.dev/resume.pdf");
}

TEST_CASE("companies and roles are complete") {
  REQUIRE(companies().size() == 7);
  CHECK(companies()[0].company == "CD Extreme OPC");
  CHECK(companies()[1].roles.size() == 3);
  CHECK(companies()[1].roles[0].title == "DevOps Engineer");
  CHECK(companies()[1].roles[0].typeTone == itsme::core::Tone::Yellow);
  CHECK(companies()[6].company == "FOSS Community - NSBM");
  CHECK(companies()[0].roles[0].tech.size() == 13);
}

TEST_CASE("education, certifications, skills, projects, contact") {
  REQUIRE(education().size() == 2);
  CHECK(education()[0].grade == "First-Class Honours");
  REQUIRE(certifications().size() == 2);
  CHECK(certifications()[1].expiry.empty());
  REQUIRE(skillCategories().size() == 10);
  CHECK(skillCategories()[0].name == "cloud");
  CHECK(skillCategories()[9].skills.back() == "CQRS / Mediator");
  REQUIRE(projects().size() == 7);
  CHECK(projects()[2].name == "techxeed");
  CHECK_FALSE(projects()[2].github.has_value());
  CHECK(projects()[0].github == std::optional<std::string>{"DFanso/DevOps-Project-001"});
  REQUIRE(contactLinks().size() == 6);
  CHECK(contactLinks()[1].id == "LO");
  CHECK_FALSE(contactLinks()[1].url.has_value());
}
```

`tests/CMakeLists.txt`: add `target_sources(itsme_tests PRIVATE data/portfolio_test.cpp)`.

- [ ] **Step 2: Run to verify failure**

Run: `cmake --build --preset default` → missing `data/Portfolio.hpp`.

- [ ] **Step 3: Implement**

`src/data/Portfolio.hpp`:
```cpp
#pragma once
#include <optional>
#include <string>
#include <vector>
#include "core/Command.hpp"

namespace itsme::data {

struct Profile {
  std::string name, title, tagline, location, email, linkedin, linkedinHandle, github, githubHandle,
      portfolio, portfolioHandle, resumeUrl, summary;
};

struct Role {
  std::string type;
  core::Tone typeTone;
  std::string title;
  std::string period;
  std::vector<std::string> responsibilities;
  std::vector<std::string> tech;
};

struct Company {
  std::string company;
  std::string totalPeriod;
  std::vector<Role> roles;
};

struct EducationEntry {
  std::string degree, institution, grade, period, location;
};

struct Certification {
  std::string name, issuer, date, expiry, id;
};

struct SkillCategory {
  std::string name;
  std::vector<std::string> skills;
};

struct Project {
  std::string type, name, url;
  std::optional<std::string> github;
  std::string description;
  std::vector<std::string> tech;
};

struct ContactLink {
  std::string id, name;
  std::optional<std::string> url;
};

const Profile& profile();
const std::vector<Company>& companies();
const std::vector<EducationEntry>& education();
const std::vector<Certification>& certifications();
const std::vector<SkillCategory>& skillCategories();
const std::vector<Project>& projects();
const std::vector<ContactLink>& contactLinks();

}  // namespace itsme::data
```

`src/data/Portfolio.cpp`:
```cpp
#include "data/Portfolio.hpp"

namespace itsme::data {
using core::Tone;

const Profile& profile() {
  static const Profile p{
      "Leo Felcianas",
      "DevOps Engineer | Software Engineer | Freelancer",
      "DevOps Engineer & Software Engineer",
      "Colombo, Western Province, Sri Lanka",
      "leogavin123@outlook.com",
      "https://www.linkedin.com/in/leogavin/",
      "linkedin.com/in/leogavin",
      "https://github.com/dfansoo",
      "github.com/dfansoo",
      "https://itsme.dfanso.dev/",
      "itsme.dfanso.dev",
      "https://itsme.dfanso.dev/resume.pdf",
      "Senior Software Engineer at CD Extreme OPC, holding a First-Class Honours degree from the University of "
      "Plymouth. Experienced across DevOps pipelines, backend development, cloud infrastructure, and AI-driven "
      "automation. Co-Founder & CTO of CodeXeed and KlexD, building scalable cloud-native applications and "
      "intelligent systems for global clients.",
  };
  return p;
}

const std::vector<Company>& companies() {
  static const std::vector<Company> c = {
      {"CD Extreme OPC", "March 2026 - Present",
       {{"SWE", Tone::Blue, "Senior Software Engineer", "March 2026 - Present",
         {"Developing and maintaining high-performance applications using .NET Core and Clean Architecture principles",
          "Integrating Azure DevOps pipelines for CI/CD and leveraging Azure Web Services for cloud deployments",
          "Applying architectural patterns including Mediator, CQRS, and Domain-Driven Design (DDD)",
          "Working with Entity Framework Core and PostgreSQL for robust, scalable data access layers",
          "Delivering innovative software solutions and game-related applications for a global audience",
          "Built a KYC (Know Your Customer) verification platform for 747Live, covering document and facial-similarity verification workflows",
          "Trained and fine-tuned custom Python ML models for automated identity verification and fraud detection",
          "Designed load-balanced, auto-scaling infrastructure across Cloudflare and Azure to handle high-throughput verification traffic",
          "Set up Azure DevOps (ADO) pipelines for CI/CD of the KYC platform and automated ML model deployment"},
         {".NET Core", "C#", "Azure DevOps", "Azure", "EF Core", "PostgreSQL", "Clean Arch", "Mediator", "Python",
          "PyTorch", "Cloudflare", "Docker", "Redis"}}}},
      {"Empite", "July 2024 - March 2026 · 1 yr 8 mos",
       {{"DEV", Tone::Yellow, "DevOps Engineer", "August 2025 - March 2026",
         {"Implemented auto-scaling solutions, achieving a 20% cost reduction while maintaining high availability",
          "Executed disaster recovery plans and led migrations to immutable infrastructure using Terraform",
          "Conducted load testing using Artillery, improving infrastructure scalability by 70%",
          "Collaborated with developers to optimize codebases, ensuring systems handled increased traffic",
          "Enhanced CI/CD pipelines and containerized services, accelerating deployments"},
         {"AWS", "Azure", "Linux", "Terraform", "Packer", "Go", "K8s", "Docker", "Artillery", "Rapid7"}},
        {"DEV", Tone::Yellow, "Associate DevOps Engineer", "November 2024 - July 2025",
         {"Migrated legacy project to modern Docker/ECS, reducing costs by 3x and enhancing maintainability",
          "Built Azure IaC with Terraform, reducing provisioning time by 60% and standardizing deployments",
          "Recovered an abandoned project, designing the infrastructure charter and deploying to production",
          "Refactored legacy Terraform scripts to integrate with modern services for easier scaling",
          "Designed and executed artillery-based load testing and auto-scaling improvements"},
         {"AWS", "Nest.js", "Go", "Linux", "TypeScript", "Azure DevOps", "Terraform", "Artillery"}},
        {"INT", Tone::Green, "Software Engineer Intern", "July 2024 - October 2024",
         {"Developed NestJS microservices backends and integrated Next.js frontends",
          "Optimized code performance by refactoring core services, reducing latency",
          "Contributed to microservices-based architecture, enhancing modularity",
          "Assisted in CI/CD pipeline improvements and containerization of services"},
         {"AWS", "React", "Nest.js", "Node.js", "Linux", "Git"}}}},
      {"CodeXeed", "June 2025 - Present · 11 mos",
       {{"CTO", Tone::Red, "Co-Founder & CTO", "June 2025 - Present",
         {"Defining the tech stack and architectural patterns for client projects, from discovery to deployment",
          "Driving development of enterprise-grade web applications using Next.js, NestJS, and PostgreSQL",
          "Implementing high-performance SEO-optimized structures and robust security measures",
          "Leading legacy system revamps, migrating Angular/PHP monoliths to Next.js microservices"},
         {"Next.js", "Nest.js", "PostgreSQL", "Docker", "AWS", "Terraform"}}}},
      {"KlexD", "October 2024 - Present · 1 yr 7 mos",
       {{"CTO", Tone::Red, "Co-Founder & CTO", "October 2024 - Present",
         {"Architecting intelligent systems and smart assistants using Python, OpenAI, and LangChain",
          "Steering development of mobile and web platforms using React Native and Next.js",
          "Building robust REST/GraphQL API pipelines and ETL processes to connect business tools",
          "Establishing high-quality code culture through QA testing, Docker, and CI/CD pipelines"},
         {"Python", "OpenAI", "LangChain", "React Native", "Next.js", "Docker"}}}},
      {"Fiverr", "January 2020 - May 2025 · 5 yrs 5 mos",
       {{"FRL", Tone::Purple, "Software Engineer", "May 2023 - May 2025",
         {"Delivering custom web and mobile solutions for diverse client requirements",
          "Implementing secure, scalable full-stack architectures with Go and Node.js",
          "Managing end-to-end project lifecycle from requirements to deployment"},
         {"AWS", "Next.js", "Nest.js", "Go", "Linux", "Prisma", "Tailwind", "Firebase"}},
        {"VID", Tone::Orange, "Video Editor", "January 2020 - May 2021",
         {"Produced and edited professional video content for international clients",
          "Managed end-to-end video production workflow from raw footage to final delivery"},
         {"Premiere Pro", "After Effects"}}}},
      {"Melstasoft", "June 2022 - August 2022 · 3 mos",
       {{"INT", Tone::Green, "Software Engineer Intern", "June 2022 - August 2022",
         {"Developed and maintained enterprise applications using ASP.NET and the .NET framework",
          "Worked with MSSQL databases, writing queries and managing data models",
          "Collaborated with senior engineers to deliver production-ready features"},
         {".NET", "ASP.NET", "MSSQL", "C#", "Git"}}}},
      {"FOSS Community - NSBM", "June 2020 - March 2023 · 2 yrs 10 mos",
       {{"VOL", Tone::Teal, "Digital Marketing Team Lead", "February 2022 - March 2023",
         {"Led the digital marketing team, managing campaigns and community outreach",
          "Coordinated with council members to align marketing with community events"},
         {"Social Media", "Design"}},
        {"VOL", Tone::Teal, "Council Member", "June 2021 - March 2023",
         {"Served as an active council member, driving open-source initiatives and community growth",
          "Organized and participated in FOSS events, workshops, and hackathons"},
         {"Open Source", "Linux"}},
        {"VOL", Tone::Teal, "Volunteer", "June 2020 - June 2021",
         {"Volunteered in community activities, supporting FOSS events and initiatives"},
         {"Open Source"}}}},
  };
  return c;
}

const std::vector<EducationEntry>& education() {
  static const std::vector<EducationEntry> e = {
      {"BSc in Computer Software Engineering", "University of Plymouth", "First-Class Honours",
       "June 2021 - December 2024", "Plymouth, United Kingdom"},
      {"Foundation Program for Bachelor's Degree", "NSBM Green University", "", "March 2020 - April 2021",
       "Sri Lanka"},
  };
  return e;
}

const std::vector<Certification>& certifications() {
  static const std::vector<Certification> c = {
      {"Multicloud Network Associate", "Aviatrix", "Issued Sep 2025", "Expires Sep 2028", "Credential ID 2025-27675"},
      {"AWS Cloud Practitioner Essentials", "Amazon Web Services (AWS)", "Issued May 2025", "", ""},
  };
  return c;
}

const std::vector<SkillCategory>& skillCategories() {
  static const std::vector<SkillCategory> s = {
      {"cloud", {"AWS", "GCP", "Azure", "Cloudflare"}},
      {"containers", {"Docker", "Kubernetes", "K3s", "ECS", "ELK"}},
      {"infra", {"Linux", "Helm", "Nginx", "Terraform", "Packer", "Ansible", "Jenkins", "GitHub Actions", "ArgoCD", "GitOps", "Artillery"}},
      {"lang", {"Go", "Rust", "Python", "TypeScript", "JavaScript", "Node.js", "C#"}},
      {"frameworks", {"Next.js", "FastAPI", "11ty", "Nest.js", "Fiber", "HTMX", "Tailwind", "GraphQL", "React Native", ".NET Core", "EF Core"}},
      {"ai", {"OpenAI", "LangChain", "AI Automation"}},
      {"db", {"PostgreSQL", "MySQL", "MongoDB", "Redis", "Firebase", "Prisma", "GORM"}},
      {"serverless", {"Vercel", "App Runner", "App Service", "Lambda", "Workers", "Convex"}},
      {"monitor", {"Prometheus", "Grafana", "Loki", "Tempo", "Mimir", "ELK Stack", "Rapid7"}},
      {"arch", {"Microservices", "System Design", "Network Security", "Technical Product Management", "REST / GraphQL APIs", "Clean Architecture", "CQRS / Mediator"}},
  };
  return s;
}

const std::vector<Project>& projects() {
  static const std::vector<Project> p = {
      {"OPS", "eks-gitops-platform",
       "https://github.com/DFanso?tab=repositories&q=DevOps-Project-001&type=&language=&sort=",
       std::string("DFanso/DevOps-Project-001"),
       "Production-grade 3-repo GitOps platform on AWS EKS with Terraform IaC, GitHub Actions CI, and ArgoCD for continuous deployment with DataDog observability.",
       {"AWS EKS", "Terraform", "Kubernetes", "Helm", "ArgoCD", "DataDog", "GitHub Actions", "Go"}},
      {"OPS", "k3s-cicd", "https://github.com/DFanso/k3s", std::string("DFanso/k3s"),
       "A complete Kubernetes deployment setup with automated CI/CD using GitHub Actions, Helm, and full observability stack.",
       {"K3s", "GitHub Actions", "Helm", "Prometheus", "Grafana", "Nginx", "FastAPI"}},
      {"WEB", "techxeed", "https://www.techxeed.com/", std::nullopt,
       "A comprehensive digital solutions platform offering web development, mobile apps, AI solutions, and digital marketing services.",
       {"Next.js", "Nest.js", "MongoDB", "TailwindCSS", "Firebase", "AWS", "Stripe", "Terraform"}},
      {"APP", "quickquest", "https://github.com/DFanso/QuickQuest", std::string("DFanso/QuickQuest"),
       "A location-based platform connecting customers with laborers, featuring real-time chat via SSE and geospatial queries.",
       {"Next.js", "Nest.js", "MongoDB", "Python", "AWS", "PayPal"}},
      {"APP", "rss-reader", "https://github.com/DFanso/rss", std::string("DFanso/rss"),
       "A simple, modern RSS reader and generator built with Go and HTMX, featuring persistent storage and a clean UI.",
       {"Go", "HTMX", "TailwindCSS"}},
      {"CLI", "commit-msg", "https://github.com/DFanso/commit-msg", std::string("DFanso/commit-msg"),
       "AI-powered CLI tool that generates conventional commit messages using various LLMs including Gemini, Grok, and OpenAI.",
       {"Go", "LLMs"}},
      {"OPS", "aws-ecs-infra", "https://github.com/DFanso/aws-ecs-infrastructure",
       std::string("DFanso/aws-ecs-infrastructure"),
       "Infrastructure as Code configurations for deploying scalable applications on AWS ECS using Terraform.",
       {"Terraform", "AWS"}},
  };
  return p;
}

const std::vector<ContactLink>& contactLinks() {
  static const std::vector<ContactLink> c = {
      {"EM", "leogavin123@outlook.com", std::string("mailto:leogavin123@outlook.com")},
      {"LO", "Colombo, Sri Lanka", std::nullopt},
      {"GH", "github.com/dfansoo", std::string("https://github.com/dfansoo")},
      {"IN", "linkedin.com/in/leogavin", std::string("https://www.linkedin.com/in/leogavin/")},
      {"DC", "discord", std::string("https://discord.gg/DcFFdcjfAf")},
      {"IG", "instagram.com/dfansoo", std::string("https://instagram.com/dfansoo")},
  };
  return c;
}

}  // namespace itsme::data
```

Add `src/data/Portfolio.cpp` to `itsme_core`.

- [ ] **Step 4: Run tests** → all pass.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat(data): port portfolio content (profile, experience, skills, projects, contact)"
```

---

### Task 7: Theme, output helpers, simple outputs

**Files:**
- Create: `src/outputs/Theme.hpp`, `src/outputs/Theme.cpp`, `src/outputs/Common.hpp`, `src/outputs/Common.cpp`, `src/outputs/Outputs.hpp`, `src/outputs/TextOutput.cpp`, `src/outputs/Welcome.cpp`, `src/outputs/Whoami.cpp`, `src/outputs/About.cpp`, `src/outputs/Education.cpp`, `src/outputs/Certifications.cpp`, `src/outputs/Contact.cpp`, `src/outputs/Misc.cpp` (weather, ping, time)
- Create: `tests/RenderHelper.hpp`, `tests/outputs/simple_outputs_test.cpp`
- Modify: `CMakeLists.txt` (new lib `itsme_ui`), `tests/CMakeLists.txt` (link `itsme_ui`)

**Interfaces:**
- Produces (namespace `itsme::outputs`):
  - `void setTrueColor(bool)`, `bool trueColor()`, `ftxui::Color tone(core::Tone)`, `ftxui::Color hexColor(std::string_view "#rrggbb")`, `ftxui::Color matrixGreen()`, `ftxui::Color bgColor()`
  - Common: `heading(str)`, `branch(bool last)`, `t(str, Tone)`, `para(str, Tone)`, `richPara(std::vector<std::pair<std::string, core::Tone>>)`, `tagRow(std::vector<std::string>)`, `indent(Element, int)`, `blank()`
  - `struct RenderContext { int width = 80; int hour = 12; }`
  - `Element renderText(const core::TextOutput&)`, `renderWelcome(const RenderContext&, int revealed = -1)`, `int welcomeTypewriterLength()`, `renderWhoami()`, `renderAbout()`, `renderEducation()`, `renderCertifications()`, `renderContact()`, `renderWeather()`, `renderPing()`, `renderTime(const std::string&)`
  - Test helper `std::string renderPlain(ftxui::Element, int w = 100, int h = 40)` strips ANSI codes.
  - CMake target `itsme_ui` (links `itsme_core`, `ftxui::component ftxui::dom ftxui::screen`).

- [ ] **Step 1: Write the failing tests**

`tests/RenderHelper.hpp`:
```cpp
#pragma once
#include <string>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>

inline std::string stripAnsi(const std::string& in) {
  std::string out;
  for (std::size_t i = 0; i < in.size();) {
    if (in[i] == '\x1b') {
      while (i < in.size() && in[i] != 'm') ++i;
      ++i;
    } else {
      out += in[i++];
    }
  }
  return out;
}

inline std::string renderPlain(ftxui::Element e, int w = 100, int h = 40) {
  auto screen = ftxui::Screen::Create(ftxui::Dimension::Fixed(w), ftxui::Dimension::Fixed(h));
  ftxui::Render(screen, e);
  return stripAnsi(screen.ToString());
}
```

`tests/outputs/simple_outputs_test.cpp`:
```cpp
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
```

`tests/CMakeLists.txt`: add `target_sources(itsme_tests PRIVATE outputs/simple_outputs_test.cpp)` and change link line to `target_link_libraries(itsme_tests PRIVATE itsme_core itsme_ui Catch2::Catch2WithMain)`.

- [ ] **Step 2: Run to verify failure**

Run: `cmake --build --preset default` → missing `outputs/Outputs.hpp` / unknown target `itsme_ui`.

- [ ] **Step 3: Implement**

`CMakeLists.txt` — add after `itsme_core`:
```cmake
add_library(itsme_ui STATIC
  src/outputs/Theme.cpp
  src/outputs/Common.cpp
  src/outputs/TextOutput.cpp
  src/outputs/Welcome.cpp
  src/outputs/Whoami.cpp
  src/outputs/About.cpp
  src/outputs/Education.cpp
  src/outputs/Certifications.cpp
  src/outputs/Contact.cpp
  src/outputs/Misc.cpp)
target_link_libraries(itsme_ui PUBLIC itsme_core ftxui::component ftxui::dom ftxui::screen)
itsme_set_warnings(itsme_ui)
```
and change `itsme` to link `itsme_ui` (`target_link_libraries(itsme PRIVATE itsme_ui)`).

`src/outputs/Theme.hpp`:
```cpp
#pragma once
#include <string_view>
#include <ftxui/screen/color.hpp>
#include "core/Command.hpp"

namespace itsme::outputs {

void setTrueColor(bool enabled);
bool trueColor();

ftxui::Color tone(core::Tone t);
// "#rrggbb" → Color. Falls back to `fallback` on bad input or when true color is off.
ftxui::Color hexColor(std::string_view hex, ftxui::Color fallback = ftxui::Color::GrayLight);
ftxui::Color matrixGreen();  // #00ff41
ftxui::Color bgColor();      // #1a1b26

}  // namespace itsme::outputs
```

`src/outputs/Theme.cpp`:
```cpp
#include "outputs/Theme.hpp"
#include <cstdlib>
#include <string>

namespace itsme::outputs {
using ftxui::Color;

namespace {
bool g_trueColor = true;

Color pick(Color rgb, Color fallback16) { return g_trueColor ? rgb : fallback16; }
}  // namespace

void setTrueColor(bool enabled) { g_trueColor = enabled; }
bool trueColor() { return g_trueColor; }

Color tone(core::Tone t) {
  using core::Tone;
  switch (t) {
    case Tone::Fg:     return pick(Color::RGB(0xc0, 0xca, 0xf5), Color::White);
    case Tone::Muted:  return pick(Color::RGB(0xa9, 0xb1, 0xd6), Color::GrayLight);
    case Tone::Blue:   return pick(Color::RGB(0x7a, 0xa2, 0xf7), Color::Blue);
    case Tone::Purple: return pick(Color::RGB(0xbb, 0x9a, 0xf7), Color::Magenta);
    case Tone::Green:  return pick(Color::RGB(0x9e, 0xce, 0x6a), Color::Green);
    case Tone::Red:    return pick(Color::RGB(0xf7, 0x76, 0x8e), Color::Red);
    case Tone::Yellow: return pick(Color::RGB(0xe0, 0xaf, 0x68), Color::Yellow);
    case Tone::Cyan:   return pick(Color::RGB(0x7d, 0xcf, 0xff), Color::Cyan);
    case Tone::Teal:   return pick(Color::RGB(0x73, 0xda, 0xca), Color::CyanLight);
    case Tone::Orange: return pick(Color::RGB(0xff, 0x9e, 0x64), Color::YellowLight);
  }
  return Color::Default;
}

Color hexColor(std::string_view hex, Color fallback) {
  if (!g_trueColor || hex.size() != 7 || hex[0] != '#') return fallback;
  for (std::size_t i = 1; i < 7; ++i)
    if (!std::isxdigit(static_cast<unsigned char>(hex[i]))) return fallback;
  auto byte = [&](std::size_t pos) {
    return static_cast<std::uint8_t>(std::strtol(std::string(hex.substr(pos, 2)).c_str(), nullptr, 16));
  };
  return Color::RGB(byte(1), byte(3), byte(5));
}

Color matrixGreen() { return pick(Color::RGB(0x00, 0xff, 0x41), Color::GreenLight); }
Color bgColor() { return pick(Color::RGB(0x1a, 0x1b, 0x26), Color::Black); }

}  // namespace itsme::outputs
```
(add `#include <cctype>` and `#include <cstdint>`.)

`src/outputs/Common.hpp`:
```cpp
#pragma once
#include <string>
#include <utility>
#include <vector>
#include <ftxui/dom/elements.hpp>
#include "core/Command.hpp"

namespace itsme::outputs {

ftxui::Element heading(const std::string& s);                     // purple bold
ftxui::Element branch(bool last);                                 // "├─▶ " or "└─▶ " muted
ftxui::Element t(const std::string& s, core::Tone tone);          // colored text
ftxui::Element para(const std::string& s, core::Tone tone);       // word-wrapped
ftxui::Element richPara(const std::vector<std::pair<std::string, core::Tone>>& segments);
ftxui::Element tagRow(const std::vector<std::string>& items);     // "│ item" flow, green
ftxui::Element indent(ftxui::Element e, int cols);
ftxui::Element blank();

}  // namespace itsme::outputs
```

`src/outputs/Common.cpp`:
```cpp
#include "outputs/Common.hpp"
#include <sstream>
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element heading(const std::string& s) { return text(s) | color(tone(Tone::Purple)) | bold; }

Element branch(bool last) { return text(last ? "└─▶ " : "├─▶ ") | color(tone(Tone::Muted)); }

Element t(const std::string& s, Tone tn) { return text(s) | color(tone(tn)); }

Element para(const std::string& s, Tone tn) { return paragraph(s) | color(tone(tn)); }

Element richPara(const std::vector<std::pair<std::string, Tone>>& segments) {
  Elements words;
  for (const auto& [str, tn] : segments) {
    std::istringstream in(str);
    std::string word;
    while (in >> word) words.push_back(text(word + " ") | color(tone(tn)));
  }
  return hflow(std::move(words));
}

Element tagRow(const std::vector<std::string>& items) {
  Elements cells;
  for (const auto& item : items)
    cells.push_back(hbox({text("│ ") | color(tone(Tone::Muted)), text(item + "  ") | color(tone(Tone::Green))}));
  return hflow(std::move(cells));
}

Element indent(Element e, int cols) { return hbox({text(std::string(static_cast<std::size_t>(cols), ' ')), std::move(e)}); }

Element blank() { return text(""); }

}  // namespace itsme::outputs
```

`src/outputs/Outputs.hpp`:
```cpp
#pragma once
#include <map>
#include <optional>
#include <string>
#include <ftxui/dom/elements.hpp>
#include "core/Command.hpp"

namespace itsme::outputs {

struct RenderContext {
  int width = 80;
  int hour = 12;  // local hour 0-23, drives the welcome greeting
};

ftxui::Element renderText(const core::TextOutput& out);

// revealed = number of code points of the text lines shown (-1 = everything).
ftxui::Element renderWelcome(const RenderContext& ctx, int revealed = -1);
int welcomeTypewriterLength();

ftxui::Element renderWhoami();
ftxui::Element renderAbout();
ftxui::Element renderEducation();
ftxui::Element renderCertifications();
ftxui::Element renderContact();
ftxui::Element renderWeather();
ftxui::Element renderPing();
ftxui::Element renderTime(const std::string& timeString);

}  // namespace itsme::outputs
```

`src/outputs/TextOutput.cpp`:
```cpp
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"

namespace itsme::outputs {
using namespace ftxui;

Element renderText(const core::TextOutput& out) {
  Elements lines;
  for (const auto& line : out.lines) lines.push_back(para(line, out.tone));
  return vbox(std::move(lines));
}

}  // namespace itsme::outputs
```

`src/outputs/Welcome.cpp`:
```cpp
#include <array>
#include "core/Strings.hpp"
#include "core/Version.hpp"
#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

namespace {
constexpr std::array<const char*, 6> kBanner = {
    " ██████╗ ███████╗ █████╗ ███╗   ██╗███████╗ ██████╗ ",
    " ██╔══██╗██╔════╝██╔══██╗████╗  ██║██╔════╝██╔═══██╗",
    " ██║  ██║█████╗  ███████║██╔██╗ ██║███████╗██║   ██║",
    " ██║  ██║██╔══╝  ██╔══██║██║╚██╗██║╚════██║██║   ██║",
    " ██████╔╝██║     ██║  ██║██║ ╚████║███████║╚██████╔╝",
    " ╚═════╝ ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═══╝╚══════╝ ╚═════╝ ",
};

std::string greeting(int hour) {
  if (hour < 12) return "Good morning, visitor!";
  if (hour < 18) return "Good afternoon, visitor!";
  return "Good evening, visitor!";
}

struct Line {
  std::string text;
  Tone tone;
};

std::vector<Line> textLines(int hour) {
  return {
      {greeting(hour), Tone::Green},
      {data::profile().tagline, Tone::Muted},
      {"────────────────────────────────────", Tone::Muted},
      {"Welcome to my terminal portfolio. (TUI v" + std::string(itsme::version()) + ")", Tone::Fg},
      {"Type 'help' to see available commands, or try 'github' for stats.", Tone::Muted},
  };
}
}  // namespace

int welcomeTypewriterLength() {
  int n = 0;
  for (const auto& l : textLines(12)) n += static_cast<int>(core::utf8Chars(l.text).size());
  return n;
}

Element renderWelcome(const RenderContext& ctx, int revealed) {
  Elements rows;
  if (ctx.width >= 60) {
    for (const char* row : kBanner) rows.push_back(text(row) | color(tone(Tone::Blue)) | bold);
  } else {
    rows.push_back(text("DFANSO") | color(tone(Tone::Blue)) | bold);
  }
  rows.push_back(blank());

  int budget = revealed;
  for (const auto& l : textLines(ctx.hour)) {
    if (revealed < 0) {
      rows.push_back(t(l.text, l.tone));
      continue;
    }
    if (budget <= 0) break;
    auto chars = core::utf8Chars(l.text);
    std::string shown;
    int take = static_cast<int>(chars.size()) < budget ? static_cast<int>(chars.size()) : budget;
    for (int i = 0; i < take; ++i) shown += chars[static_cast<std::size_t>(i)];
    budget -= take;
    rows.push_back(t(shown, l.tone));
  }
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
```

`src/outputs/Whoami.cpp`:
```cpp
#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderWhoami() {
  const auto& p = data::profile();
  return vbox({
      text(p.name) | color(tone(Tone::Blue)) | bold,
      t(p.tagline, Tone::Muted),
  });
}

}  // namespace itsme::outputs
```

`src/outputs/About.cpp`:
```cpp
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderAbout() {
  Element bar = text("│ ") | color(tone(Tone::Muted));
  auto row = [&](Element e) { return hbox({text("  "), bar, std::move(e) | flex}); };

  Element focus = hflow({
      text("DevOps Pipelines  ") | color(tone(Tone::Blue)),
      text("Cloud Infrastructure  ") | color(tone(Tone::Green)),
      text("Backend  ") | color(tone(Tone::Purple)),
      text("AI Automation") | color(tone(Tone::Yellow)),
  });

  return vbox({
      heading("Professional Summary"),
      row(richPara({{"Senior Software Engineer", Tone::Blue}, {"at", Tone::Fg}, {"CD Extreme OPC,", Tone::Blue},
                    {"holding a", Tone::Fg}, {"First-Class Honours", Tone::Yellow}, {"degree from the", Tone::Fg},
                    {"University of Plymouth.", Tone::Green},
                    {"Experienced across DevOps pipelines, backend development, cloud infrastructure, and AI-driven automation.", Tone::Fg}})),
      row(blank()),
      row(richPara({{"Co-Founder & CTO", Tone::Purple}, {"of", Tone::Fg}, {"CodeXeed", Tone::Blue}, {"and", Tone::Fg},
                    {"KlexD,", Tone::Blue},
                    {"building scalable cloud-native applications and intelligent systems for global clients.", Tone::Fg}})),
      row(blank()),
      row(t("FOCUS", Tone::Muted)),
      row(focus),
      row(blank()),
      row(t("LOCATION", Tone::Muted)),
      row(hbox({text("📍 ") | color(tone(Tone::Red)), t("Sri Lanka (Open to Remote)", Tone::Fg)})),
  });
}

}  // namespace itsme::outputs
```

`src/outputs/Education.cpp`:
```cpp
#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderEducation() {
  Elements rows = {heading("Education")};
  for (const auto& e : data::education()) {
    rows.push_back(hbox({branch(true), t(e.degree, Tone::Yellow)}));
    Elements meta = {t(e.institution, Tone::Green), t(" | ", Tone::Muted), t(e.location, Tone::Fg)};
    if (!e.grade.empty()) {
      meta.push_back(t(" | ", Tone::Muted));
      meta.push_back(t(e.grade, Tone::Red));
    }
    rows.push_back(indent(hbox(std::move(meta)), 4));
    rows.push_back(indent(t(e.period, Tone::Muted), 4));
    rows.push_back(blank());
  }
  rows.pop_back();
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
```

`src/outputs/Certifications.cpp`:
```cpp
#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderCertifications() {
  Elements rows = {heading("Certifications")};
  for (const auto& c : data::certifications()) {
    rows.push_back(hbox({branch(true), t(c.name, Tone::Yellow)}));
    Elements meta = {t(c.issuer, Tone::Green)};
    if (!c.id.empty()) {
      meta.push_back(t(" | ", Tone::Muted));
      meta.push_back(t(c.id, Tone::Muted));
    }
    rows.push_back(indent(hbox(std::move(meta)), 4));
    std::string dates = c.date + (c.expiry.empty() ? "" : " • " + c.expiry);
    rows.push_back(indent(t(dates, Tone::Muted), 4));
    rows.push_back(blank());
  }
  rows.pop_back();
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
```

`src/outputs/Contact.cpp`:
```cpp
#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderContact() {
  Elements rows = {heading("Contact Information")};
  for (const auto& link : data::contactLinks()) {
    Elements cells = {branch(true), t("[" + link.id + "] ", Tone::Yellow)};
    if (link.url) {
      cells.push_back(t(link.name, Tone::Blue));
      if (*link.url != "mailto:" + link.name) cells.push_back(t("  " + *link.url, Tone::Muted));
    } else {
      cells.push_back(t(link.name, Tone::Fg));
    }
    rows.push_back(hbox(std::move(cells)));
  }
  rows.push_back(blank());
  rows.push_back(hbox({t("Note: ", Tone::Green), t("Select a URL in your terminal to copy it", Tone::Muted)}));
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
```

`src/outputs/Misc.cpp`:
```cpp
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderWeather() {
  return vbox({
      t("Weather information is not available in the terminal.", Tone::Blue),
      t("Try looking outside your window! 🌤️", Tone::Muted),
  });
}

Element renderPing() {
  auto line = [](const char* seq, const char* ms) {
    return hbox({t("64 bytes from dfanso.dev", Tone::Blue),
                 t(std::string(" : icmp_seq=") + seq + " ttl=64 time= ", Tone::Muted), t(ms, Tone::Green)});
  };
  return vbox({
      heading("PING dfanso.dev (192.168.1.1)"),
      line("1", "0.045 ms"),
      line("2", "0.038 ms"),
      line("3", "0.042 ms"),
      line("4", "0.039 ms"),
      blank(),
      hbox({t("--- ", Tone::Muted), t("dfanso.dev ping statistics ", Tone::Fg), t("---", Tone::Muted)}),
      t("4 packets transmitted, 4 received, 0% packet loss, time 3ms", Tone::Fg),
      t("rtt min/avg/max = 0.038/0.041/0.045 ms", Tone::Fg),
  });
}

Element renderTime(const std::string& timeString) { return t(timeString, Tone::Green); }

}  // namespace itsme::outputs
```

- [ ] **Step 4: Run tests** → `cmake --build --preset default && ctest --preset default`, all pass.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat(outputs): theme, helpers and static outputs (welcome, whoami, about, education, certs, contact, misc)"
```

---

### Task 8: Registry-derived and structured outputs (help, ls, skills, experience, projects, neofetch)

**Files:**
- Create: `src/github/Model.hpp` (header-only data model, needed by projects output)
- Create: `src/outputs/Help.cpp`, `src/outputs/Ls.cpp`, `src/outputs/Skills.cpp`, `src/outputs/Experience.cpp`, `src/outputs/Projects.cpp`, `src/outputs/Neofetch.cpp`
- Modify: `src/outputs/Outputs.hpp`, `CMakeLists.txt`, `tests/CMakeLists.txt`
- Test: `tests/outputs/structured_outputs_test.cpp`

**Interfaces:**
- Produces (namespace `itsme::github`, in `Model.hpp`):
  - `inline constexpr const char* kUsername = "dfansoo";`
  - `struct Stats { long commits, prs, issues, repos, stars, forks, followers, contributions; }` (all default 0)
  - `struct LanguageStat { std::string name; int percentage; std::string color; }`
  - `struct TopRepo { std::string name, url; std::optional<std::string> description, language, languageColor; long stars, forks; }`
  - `struct ContributionDay { int count; std::string date; int weekday; }`, `struct ContributionWeek { std::vector<ContributionDay> days; }`, `struct ContributionCalendar { long total; std::vector<ContributionWeek> weeks; }`
  - `struct GitHubStatsData { bool hasFullData; Stats stats; std::vector<LanguageStat> languageStats; std::vector<TopRepo> topRepos; bool isPinned; std::optional<ContributionCalendar> calendar; }`
  - `struct ProjectRepoStats { long stars, forks, watchers; }`, `using ProjectStatsMap = std::map<std::string, std::optional<ProjectRepoStats>>;`
- Produces (namespace `itsme::outputs`): `renderHelp()`, `renderLs()`, `renderSkills()`, `renderExperience()`, `renderNeofetch()`, `renderProjects(const github::ProjectStatsMap* stats)` (nullptr = no stats row).

- [ ] **Step 1: Write the failing tests**

`tests/outputs/structured_outputs_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include "../RenderHelper.hpp"
#include "outputs/Outputs.hpp"

using namespace itsme::outputs;

TEST_CASE("help lists visible commands only") {
  auto s = renderPlain(renderHelp(), 100, 60);
  CHECK(s.find("Terminal Portfolio Help") != std::string::npos);
  CHECK(s.find("AVAILABLE COMMANDS:") != std::string::npos);
  CHECK(s.find("neofetch") != std::string::npos);
  CHECK(s.find("Show GitHub stats and contributions") != std::string::npos);
  CHECK(s.find("sudo") == std::string::npos);
  CHECK(s.find("Ctrl+L") != std::string::npos);
}

TEST_CASE("ls lists directories then files with perms") {
  auto s = renderPlain(renderLs(), 100, 40);
  CHECK(s.find("Directory listing of ~/portfolio") != std::string::npos);
  CHECK(s.find("drwxr-xr-x") != std::string::npos);
  CHECK(s.find("resume.pdf") != std::string::npos);
  CHECK(s.find("about/") < s.find("welcome.txt"));
  CHECK(s.find("curriculum vitae") != std::string::npos);
}

TEST_CASE("skills and experience render trees") {
  auto sk = renderPlain(renderSkills(), 120, 80);
  CHECK(sk.find("Technical Skills") != std::string::npos);
  CHECK(sk.find("~/cloud/") != std::string::npos);
  CHECK(sk.find("CQRS / Mediator") != std::string::npos);
  auto ex = renderPlain(renderExperience(), 120, 200);
  CHECK(ex.find("Experience History") != std::string::npos);
  CHECK(ex.find("CD Extreme OPC") != std::string::npos);
  CHECK(ex.find("[DEV]") != std::string::npos);
  CHECK(ex.find("Associate DevOps Engineer") != std::string::npos);
  CHECK(ex.find("Premiere Pro") != std::string::npos);
}

TEST_CASE("projects with and without stats") {
  auto none = renderPlain(renderProjects(nullptr), 120, 80);
  CHECK(none.find("Featured Projects") != std::string::npos);
  CHECK(none.find("projects/eks-gitops-platform/") != std::string::npos);
  CHECK(none.find("type: WEB") != std::string::npos);
  CHECK(none.find("★") == std::string::npos);
  itsme::github::ProjectStatsMap stats;
  stats["DFanso/k3s"] = itsme::github::ProjectRepoStats{12, 3, 4};
  stats["DFanso/rss"] = std::nullopt;
  auto with = renderPlain(renderProjects(&stats), 120, 80);
  CHECK(with.find("★ 12") != std::string::npos);
  CHECK(with.find("⑂ 3") != std::string::npos);
  CHECK(with.find("👁 4") != std::string::npos);
}

TEST_CASE("neofetch") {
  auto s = renderPlain(renderNeofetch(), 100, 20);
  CHECK(s.find("OS:") != std::string::npos);
  CHECK(s.find("dfanso.dev") != std::string::npos);
  CHECK(s.find("Portfolio-CLI") != std::string::npos);
}
```

`tests/CMakeLists.txt`: add `target_sources(itsme_tests PRIVATE outputs/structured_outputs_test.cpp)`.

- [ ] **Step 2: Run to verify failure** → undefined `renderHelp` etc.

- [ ] **Step 3: Implement**

`src/github/Model.hpp`:
```cpp
#pragma once
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace itsme::github {

inline constexpr const char* kUsername = "dfansoo";

struct Stats {
  long commits = 0, prs = 0, issues = 0, repos = 0, stars = 0, forks = 0, followers = 0, contributions = 0;
};

struct LanguageStat {
  std::string name;
  int percentage = 0;
  std::string color;  // "#rrggbb"
};

struct TopRepo {
  std::string name;
  std::string url;
  std::optional<std::string> description;
  std::optional<std::string> language;
  std::optional<std::string> languageColor;
  long stars = 0;
  long forks = 0;
};

struct ContributionDay {
  int count = 0;
  std::string date;
  int weekday = 0;
};
struct ContributionWeek {
  std::vector<ContributionDay> days;
};
struct ContributionCalendar {
  long total = 0;
  std::vector<ContributionWeek> weeks;
};

struct GitHubStatsData {
  bool hasFullData = false;
  Stats stats;
  std::vector<LanguageStat> languageStats;
  std::vector<TopRepo> topRepos;
  bool isPinned = false;
  std::optional<ContributionCalendar> calendar;
};

struct ProjectRepoStats {
  long stars = 0, forks = 0, watchers = 0;
};
using ProjectStatsMap = std::map<std::string, std::optional<ProjectRepoStats>>;

}  // namespace itsme::github
```

Append to `src/outputs/Outputs.hpp` (add `#include "github/Model.hpp"`):
```cpp
ftxui::Element renderHelp();
ftxui::Element renderLs();
ftxui::Element renderSkills();
ftxui::Element renderExperience();
ftxui::Element renderNeofetch();
ftxui::Element renderProjects(const github::ProjectStatsMap* stats);
```

`src/outputs/Help.cpp`:
```cpp
#include "core/Commands.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderHelp() {
  Elements rows = {
      heading("Terminal Portfolio Help"),
      blank(),
      t("USAGE:", Tone::Blue),
      indent(t("command [arguments]", Tone::Fg), 4),
      indent(t("Tip: press Tab to autocomplete a command.", Tone::Muted), 4),
      blank(),
      t("AVAILABLE COMMANDS:", Tone::Blue),
  };
  for (const auto& c : core::commands()) {
    if (c.hidden) continue;
    rows.push_back(indent(hbox({t(c.name, Tone::Yellow) | size(WIDTH, EQUAL, 16), t(c.description, Tone::Fg)}), 4));
  }
  rows.push_back(blank());
  rows.push_back(t("Use arrow keys ↑↓ to navigate command history", Tone::Muted));
  rows.push_back(t("Press Ctrl+L or type 'clear' to clear screen", Tone::Muted));
  rows.push_back(t("Press Ctrl+C or Ctrl+D to exit", Tone::Muted));
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
```

`src/outputs/Ls.cpp`:
```cpp
#include "core/Commands.hpp"
#include "core/Strings.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderLs() {
  Elements rows = {heading("Directory listing of ~/portfolio")};
  auto emit = [&](bool dirs) {
    for (const auto& c : core::commands()) {
      if (!c.lsEntry) continue;
      const bool isDir = core::startsWith(c.lsEntry->perms, "d");
      if (isDir != dirs) continue;
      rows.push_back(hbox({
          t(c.lsEntry->perms, Tone::Muted),
          text("  "),
          t(c.lsEntry->name, isDir ? Tone::Blue : Tone::Yellow) | size(WIDTH, EQUAL, 18),
          t(c.lsEntry->note, Tone::Muted),
      }));
    }
  };
  emit(true);
  emit(false);
  rows.push_back(blank());
  rows.push_back(t("Type the command to open an entry (e.g., 'about', 'projects')", Tone::Muted));
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
```

`src/outputs/Skills.cpp`:
```cpp
#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderSkills() {
  const auto& cats = data::skillCategories();
  Elements rows = {heading("Technical Skills")};
  for (std::size_t i = 0; i < cats.size(); ++i) {
    rows.push_back(hbox({branch(i + 1 == cats.size()), t("ls ", Tone::Yellow), t("~/" + cats[i].name + "/", Tone::Blue)}));
    rows.push_back(indent(tagRow(cats[i].skills), 4));
    if (i + 1 < cats.size()) rows.push_back(blank());
  }
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
```

`src/outputs/Experience.cpp`:
```cpp
#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderExperience() {
  const auto& companies = data::companies();
  Elements rows = {heading("Experience History")};
  for (std::size_t ci = 0; ci < companies.size(); ++ci) {
    const auto& company = companies[ci];
    rows.push_back(hbox({branch(ci + 1 == companies.size()),
                         text(company.company) | color(tone(Tone::Blue)) | bold,
                         t(" · ", Tone::Muted), t(company.totalPeriod, Tone::Muted)}));
    for (std::size_t ri = 0; ri < company.roles.size(); ++ri) {
      const auto& role = company.roles[ri];
      rows.push_back(indent(hbox({t(ri + 1 == company.roles.size() ? "└─ " : "├─ ", Tone::Muted),
                                  t("[" + role.type + "] ", role.typeTone),
                                  text(role.title) | color(tone(Tone::Green)) | bold}), 4));
      rows.push_back(indent(t(role.period, Tone::Muted), 8));
      for (const auto& resp : role.responsibilities)
        rows.push_back(indent(hbox({t("│ ", Tone::Muted), para(resp, Tone::Fg) | flex}), 8));
      rows.push_back(indent(tagRow(role.tech), 8));
      rows.push_back(blank());
    }
  }
  rows.pop_back();
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
```

`src/outputs/Projects.cpp`:
```cpp
#include "data/Portfolio.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderProjects(const github::ProjectStatsMap* stats) {
  const auto& projects = data::projects();
  Elements rows = {heading("Featured Projects")};
  for (std::size_t i = 0; i < projects.size(); ++i) {
    const auto& p = projects[i];
    Elements head = {branch(i + 1 == projects.size()), t("cat ", Tone::Yellow),
                     t("projects/" + p.name + "/ ", Tone::Blue), t("type: ", Tone::Muted), t(p.type, Tone::Red)};
    if (stats && p.github) {
      auto it = stats->find(*p.github);
      if (it != stats->end() && it->second) {
        const auto& s = *it->second;
        head.push_back(t("   ★ " + std::to_string(s.stars), Tone::Yellow));
        head.push_back(t("  ⑂ " + std::to_string(s.forks), Tone::Cyan));
        head.push_back(t("  👁 " + std::to_string(s.watchers), Tone::Purple));
      }
    }
    rows.push_back(hbox(std::move(head)));
    rows.push_back(indent(para(p.description, Tone::Fg) | flex, 4));
    rows.push_back(indent(tagRow(p.tech), 4));
    rows.push_back(indent(t(p.url, Tone::Muted), 4));
    if (i + 1 < projects.size()) rows.push_back(blank());
  }
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
```

`src/outputs/Neofetch.cpp`:
```cpp
#include "core/Version.hpp"
#include "outputs/Common.hpp"
#include "outputs/Outputs.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

Element renderNeofetch() {
  Elements art;
  for (const char* row : {"                  ▄▄▄▄▄▄▄▄▄▄▄", "                ▄▀█▀█▀█▀█▀█▀█▀▄", "               █▀█▀█▀█▀█▀█▀█▀█▀█",
                          "              ▄█▀█▀█▀█▀█▀█▀█▀█▀█▄", "             ▀▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▀"})
    art.push_back(t(row, Tone::Blue));

  auto kv = [](const char* k, std::string v) { return hbox({t(k, Tone::Blue), text(" "), t(std::move(v), Tone::Fg)}); };
  Element info = vbox({
      kv("OS:", std::string("Portfolio TUI v") + itsme::version()),
      kv("Host:", "dfanso.dev"),
      kv("Kernel:", "DevOps 5.0.1"),
      kv("Uptime:", "24/7"),
      kv("Shell:", "Portfolio-CLI"),
      kv("IDE:", "VS Code / Neovim"),
  });
  return hbox({vbox(std::move(art)), text("    "), info});
}

}  // namespace itsme::outputs
```

Add the six new `.cpp` files to `itsme_ui` in `CMakeLists.txt`.

- [ ] **Step 4: Run tests** → all pass.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat(outputs): help, ls, skills, experience, projects, neofetch"
```

---

### Task 9: Interactive shell (App, prompt, title bar, input line, main)

**Files:**
- Create: `src/app/Clock.hpp`, `src/app/Clock.cpp`, `src/app/Prompt.hpp`, `src/app/Prompt.cpp`, `src/app/TitleBar.hpp`, `src/app/TitleBar.cpp`, `src/app/BlockRenderer.hpp`, `src/app/BlockRenderer.cpp`, `src/app/App.hpp`, `src/app/App.cpp`
- Modify: `src/main.cpp` (replace), `CMakeLists.txt`, `tests/CMakeLists.txt`
- Test: `tests/app/smoke_test.cpp`

**Interfaces:**
- Consumes: `core::TerminalState/submit/clearBlocks/initialState`, `core::LineEditor`, `core::navigateHistory/completeInput/getSuggestions/commandNames`, all `outputs::render*`.
- Produces (namespace `itsme::app`):
  - `struct LocalTime { int hour, minute, second; }`, `LocalTime localNow()`, `std::string clockHHMM(const LocalTime&)`, `std::string clockHHMMSS(const LocalTime&)`
  - `ftxui::Element renderPrompt(bool awaitingProjectResponse)`
  - `ftxui::Element renderTitleBar(const std::string& clock)`
  - `struct BlockRuntime { std::string timeString; int typewriterRevealed = -1; std::shared_ptr<const github::ProjectStatsMap> projects; }`
  - `ftxui::Element renderBlock(const core::Block&, const BlockRuntime&, const outputs::RenderContext&)`
  - `struct Options { bool noBoot = false; bool noColor = false; }`
  - `class App { App(Options); int run(); ftxui::Component component(); void submit(const std::string&); const core::TerminalState& state() const; void resize(int width); }` with protected hooks `virtual bool onEvent(const ftxui::Event&)`, `virtual ftxui::Element render()`, `virtual void onBlockAdded(const core::Block&)`, `virtual void performAction(core::Action, int blockId)`, `void requestRedraw()`, `void requestExit()`, `outputs::RenderContext context() const`, and protected members `state_`, `editor_`, `runtime_`, `scroll_`, `width_`, `screen_`, `rng_`.

- [ ] **Step 1: Write the failing test**

`tests/app/smoke_test.cpp`:
```cpp
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
  CHECK(now.hour >= 0); CHECK(now.hour < 24);
}

TEST_CASE("prompt variants") {
  CHECK(renderPlain(renderPrompt(false), 80, 1).find("dfanso@terminal in ~/portfolio on main") != std::string::npos);
  CHECK(renderPlain(renderPrompt(true), 80, 1).find("Would you like to see more projects? (y/n)") != std::string::npos);
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
```

`tests/CMakeLists.txt`: add `target_sources(itsme_tests PRIVATE app/smoke_test.cpp)`.

- [ ] **Step 2: Run to verify failure** → missing `app/App.hpp`.

- [ ] **Step 3: Implement**

`src/app/Clock.hpp`:
```cpp
#pragma once
#include <string>

namespace itsme::app {
struct LocalTime {
  int hour = 0, minute = 0, second = 0;
};
LocalTime localNow();
std::string clockHHMM(const LocalTime& t);
std::string clockHHMMSS(const LocalTime& t);
}  // namespace itsme::app
```

`src/app/Clock.cpp`:
```cpp
#include "app/Clock.hpp"
#include <cstdio>
#include <ctime>

namespace itsme::app {

LocalTime localNow() {
  std::time_t now = std::time(nullptr);
  std::tm tm{};
#ifdef _MSC_VER
  localtime_s(&tm, &now);
#else
  // Only ever called from the UI thread.
  tm = *std::localtime(&now);
#endif
  return {tm.tm_hour, tm.tm_min, tm.tm_sec};
}

std::string clockHHMM(const LocalTime& t) {
  char buf[8];
  std::snprintf(buf, sizeof buf, "%02d:%02d", t.hour, t.minute);
  return buf;
}

std::string clockHHMMSS(const LocalTime& t) {
  char buf[12];
  std::snprintf(buf, sizeof buf, "%02d:%02d:%02d", t.hour, t.minute, t.second);
  return buf;
}

}  // namespace itsme::app
```

`src/app/Prompt.hpp`:
```cpp
#pragma once
#include <ftxui/dom/elements.hpp>
namespace itsme::app {
ftxui::Element renderPrompt(bool awaitingProjectResponse);
}
```

`src/app/Prompt.cpp`:
```cpp
#include "app/Prompt.hpp"
#include "outputs/Common.hpp"

namespace itsme::app {
using namespace ftxui;
using core::Tone;
using outputs::t;

Element renderPrompt(bool awaitingProjectResponse) {
  if (awaitingProjectResponse)
    return hbox({t("❯ ", Tone::Green), t("Would you like to see more projects? ", Tone::Fg), t("(y/n) ", Tone::Muted)});
  return hbox({t("❯ ", Tone::Green), t("dfanso", Tone::Blue), t("@", Tone::Muted), t("terminal", Tone::Purple),
               t(" in ", Tone::Muted), t("~/portfolio", Tone::Yellow), t(" on ", Tone::Muted), t("main ", Tone::Red)});
}

}  // namespace itsme::app
```

`src/app/TitleBar.hpp`:
```cpp
#pragma once
#include <string>
#include <ftxui/dom/elements.hpp>
namespace itsme::app {
ftxui::Element renderTitleBar(const std::string& clock);
}
```

`src/app/TitleBar.cpp`:
```cpp
#include "app/TitleBar.hpp"
#include "outputs/Common.hpp"
#include "outputs/Theme.hpp"

namespace itsme::app {
using namespace ftxui;
using core::Tone;
using outputs::t;

Element renderTitleBar(const std::string& clock) {
  Element buttons = hbox({t("● ", Tone::Red), t("● ", Tone::Yellow), t("●", Tone::Green)});
  return vbox({
      hbox({text(" "), buttons, filler(), t("guest@dfanso.dev:~", Tone::Fg), filler(), t(clock, Tone::Muted), text(" ")}),
      separator() | color(outputs::tone(Tone::Muted)),
  });
}

}  // namespace itsme::app
```

`src/app/BlockRenderer.hpp`:
```cpp
#pragma once
#include <memory>
#include <string>
#include <ftxui/dom/elements.hpp>
#include "core/TerminalState.hpp"
#include "github/Model.hpp"
#include "outputs/Outputs.hpp"

namespace itsme::app {

// Per-block mutable state owned by the App (async results, animation progress).
struct BlockRuntime {
  std::string timeString;                                   // `time`
  int typewriterRevealed = -1;                              // `welcome`, -1 = fully shown
  std::shared_ptr<const github::ProjectStatsMap> projects;  // `projects`, null until fetched
};

ftxui::Element renderBlock(const core::Block& block, const BlockRuntime& rt, const outputs::RenderContext& ctx);

}  // namespace itsme::app
```

`src/app/BlockRenderer.cpp`:
```cpp
#include "app/BlockRenderer.hpp"
#include "app/Prompt.hpp"
#include "outputs/Common.hpp"

namespace itsme::app {
using namespace ftxui;
using core::Tone;
using namespace outputs;

namespace {
Element renderComponent(const std::string& name, const BlockRuntime& rt, const RenderContext& ctx) {
  if (name == "welcome") return renderWelcome(ctx, rt.typewriterRevealed);
  if (name == "whoami") return renderWhoami();
  if (name == "about") return renderAbout();
  if (name == "projects") return renderProjects(rt.projects.get());
  if (name == "skills") return renderSkills();
  if (name == "experience") return renderExperience();
  if (name == "education") return renderEducation();
  if (name == "certifications") return renderCertifications();
  if (name == "contact") return renderContact();
  if (name == "help") return renderHelp();
  if (name == "ls") return renderLs();
  if (name == "neofetch") return renderNeofetch();
  if (name == "time") return renderTime(rt.timeString);
  if (name == "weather") return renderWeather();
  if (name == "ping") return renderPing();
  if (name == "github") return t("GitHub stats are not available in this build.", Tone::Muted);
  return t("(no renderer for '" + name + "')", Tone::Red);
}
}  // namespace

Element renderBlock(const core::Block& block, const BlockRuntime& rt, const RenderContext& ctx) {
  Elements rows;
  if (!block.seeded)
    rows.push_back(hbox({renderPrompt(block.wasAwaitingProjectResponse), t(block.input, Tone::Fg)}));

  const auto& ex = block.execution;
  switch (ex.kind) {
    case core::ExecKind::Component:
      rows.push_back(renderComponent(ex.componentName, rt, ctx));
      break;
    case core::ExecKind::Text:
    case core::ExecKind::Action:
      if (ex.text) rows.push_back(renderText(*ex.text));
      break;
  }
  rows.push_back(blank());
  return vbox(std::move(rows));
}

}  // namespace itsme::app
```

`src/app/App.hpp`:
```cpp
#pragma once
#include <random>
#include <string>
#include <unordered_map>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include "app/BlockRenderer.hpp"
#include "core/LineEditor.hpp"
#include "core/TerminalState.hpp"
#include "outputs/Outputs.hpp"

namespace itsme::app {

struct Options {
  bool noBoot = false;
  bool noColor = false;
};

class App {
 public:
  explicit App(Options opts);
  virtual ~App() = default;

  int run();                              // blocks until exit
  ftxui::Component component();           // root component (tests render it directly)
  void submit(const std::string& line);
  const core::TerminalState& state() const { return state_; }
  void resize(int width) { width_ = width; }

 protected:
  virtual bool onEvent(const ftxui::Event& e);
  virtual ftxui::Element render();
  virtual void onBlockAdded(const core::Block& block);
  virtual void performAction(core::Action action, int blockId);
  ftxui::Element renderInputLine();
  outputs::RenderContext context() const;
  void requestRedraw();
  void requestExit();

  Options opts_;
  core::TerminalState state_;
  core::LineEditor editor_;
  std::unordered_map<int, BlockRuntime> runtime_;
  float scroll_ = 1.0f;  // 1.0 = bottom
  int width_ = 80;
  ftxui::ScreenInteractive* screen_ = nullptr;
  std::mt19937 rng_{std::random_device{}()};
};

}  // namespace itsme::app
```

`src/app/App.cpp`:
```cpp
#include "app/App.hpp"
#include "app/Clock.hpp"
#include "app/Prompt.hpp"
#include "app/TitleBar.hpp"
#include "core/Commands.hpp"
#include "core/InputHelpers.hpp"
#include "outputs/Common.hpp"
#include "outputs/Theme.hpp"

namespace itsme::app {
using namespace ftxui;
using core::Tone;
using outputs::t;

namespace {
const Event kCtrlL = Event::Special(std::string("\x0c"));
const Event kCtrlD = Event::Special(std::string("\x04"));
}  // namespace

App::App(Options opts) : opts_(opts), state_(core::initialState()) {
  outputs::setTrueColor(!opts_.noColor);
}

Component App::component() {
  return CatchEvent(Renderer([this] { return render(); }), [this](const Event& e) { return onEvent(e); });
}

int App::run() {
  auto screen = ScreenInteractive::Fullscreen();
  screen_ = &screen;
  screen.Loop(component());
  screen_ = nullptr;
  return 0;
}

void App::requestRedraw() {
  if (screen_) screen_->PostEvent(Event::Custom);
}

void App::requestExit() {
  if (screen_) screen_->Exit();
}

outputs::RenderContext App::context() const {
  outputs::RenderContext ctx;
  ctx.width = width_;
  ctx.hour = localNow().hour;
  return ctx;
}

void App::submit(const std::string& line) {
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  const core::Action action = core::submit(state_, line, dist(rng_));
  scroll_ = 1.0f;
  if (action == core::Action::Clear) {
    runtime_.clear();
    return;
  }
  const core::Block& added = state_.blocks.back();
  onBlockAdded(added);
  if (action != core::Action::None) performAction(action, added.id);
}

void App::onBlockAdded(const core::Block& block) {
  if (block.execution.kind == core::ExecKind::Component && block.execution.componentName == "time")
    runtime_[block.id].timeString = clockHHMMSS(localNow());
}

void App::performAction(core::Action /*action*/, int /*blockId*/) {
  // Matrix/Hack overlays and resume opening are wired in later tasks.
}

bool App::onEvent(const Event& e) {
  if (e == Event::Return) {
    const std::string line = editor_.text();
    editor_.clear();
    submit(line);
    return true;
  }
  if (e == kCtrlL) { core::clearBlocks(state_); runtime_.clear(); return true; }
  if (e == kCtrlD) { requestExit(); return true; }
  if (e == Event::ArrowUp || e == Event::ArrowDown) {
    auto nav = core::navigateHistory(state_.history, state_.historyIndex,
                                     e == Event::ArrowUp ? core::HistoryDir::Up : core::HistoryDir::Down);
    state_.historyIndex = nav.index;
    editor_.set(nav.value);
    return true;
  }
  if (e == Event::Tab) {
    if (auto c = core::completeInput(editor_.text(), core::commandNames())) editor_.set(*c);
    return true;
  }
  if (e == Event::Backspace) { editor_.backspace(); return true; }
  if (e == Event::Delete) { editor_.del(); return true; }
  if (e == Event::ArrowLeft) { editor_.left(); return true; }
  if (e == Event::ArrowRight) { editor_.right(); return true; }
  if (e == Event::Home) { editor_.home(); return true; }
  if (e == Event::End) { editor_.end(); return true; }
  if (e == Event::PageUp) { scroll_ = scroll_ > 0.2f ? scroll_ - 0.2f : 0.0f; return true; }
  if (e == Event::PageDown) { scroll_ = scroll_ < 0.8f ? scroll_ + 0.2f : 1.0f; return true; }
  if (e.is_mouse()) {
    if (e.mouse().button == Mouse::WheelUp) { scroll_ = scroll_ > 0.05f ? scroll_ - 0.05f : 0.0f; return true; }
    if (e.mouse().button == Mouse::WheelDown) { scroll_ = scroll_ < 0.95f ? scroll_ + 0.05f : 1.0f; return true; }
    return false;
  }
  if (e == Event::Custom) return true;
  if (e.is_character()) {
    editor_.insert(e.character());
    scroll_ = 1.0f;
    return true;
  }
  return false;
}

Element App::renderInputLine() {
  const auto names = core::commandNames();
  const auto suggestions = core::getSuggestions(editor_.text(), names);
  std::string ghost;
  if (suggestions.size() == 1 && editor_.at().empty())
    ghost = suggestions.front().substr(editor_.text().size());

  const std::string at = editor_.at();
  Element line = hbox({
      renderPrompt(state_.awaitingProjectResponse),
      t(editor_.before(), Tone::Fg),
      text(at.empty() ? " " : at) | inverted,
      t(editor_.after(), Tone::Fg),
      t(ghost, Tone::Muted) | dim,
  });
  if (suggestions.size() > 1) {
    std::string joined;
    for (const auto& s : suggestions) joined += s + "  ";
    return vbox({line, outputs::indent(t(joined, Tone::Muted) | dim, 2)});
  }
  return line;
}

Element App::render() {
  if (screen_) width_ = screen_->dimx();
  const auto ctx = context();
  Elements blocks;
  for (const auto& b : state_.blocks) blocks.push_back(renderBlock(b, runtime_[b.id], ctx));
  Element output = vbox(std::move(blocks)) | focusPositionRelative(0.0f, scroll_) | yframe | flex;
  return vbox({renderTitleBar(clockHHMM(localNow())), output, renderInputLine()}) | bgcolor(outputs::bgColor());
}

}  // namespace itsme::app
```

`src/main.cpp`:
```cpp
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "app/App.hpp"
#include "core/Version.hpp"

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define ISATTY(fd) _isatty(fd)
#define FILENO(f) _fileno(f)
#else
#include <unistd.h>
#define ISATTY(fd) isatty(fd)
#define FILENO(f) fileno(f)
#endif

namespace {
void usage() {
  std::printf("itsme %s - terminal portfolio of Leo Felcianas\n\n"
              "usage: itsme [--no-boot] [--no-color] [--version] [--help]\n"
              "  --no-boot   skip the boot and typewriter animations\n"
              "  --no-color  use the 16-color palette (also honours NO_COLOR)\n",
              itsme::version());
}
}  // namespace

int main(int argc, char** argv) {
  itsme::app::Options opts;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--no-boot") == 0) opts.noBoot = true;
    else if (std::strcmp(argv[i], "--no-color") == 0) opts.noColor = true;
    else if (std::strcmp(argv[i], "--version") == 0) { std::printf("%s\n", itsme::version()); return 0; }
    else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) { usage(); return 0; }
    else { std::fprintf(stderr, "itsme: unknown option '%s'\n", argv[i]); usage(); return 2; }
  }
  if (std::getenv("NO_COLOR") != nullptr) opts.noColor = true;

  if (!ISATTY(FILENO(stdout)) || !ISATTY(FILENO(stdin))) {
    std::fprintf(stderr, "itsme: needs an interactive terminal (stdin/stdout is not a TTY).\n");
    return 1;
  }
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif
  itsme::app::App app(opts);
  return app.run();
}
```

`CMakeLists.txt`: add to `itsme_ui` sources `src/app/Clock.cpp src/app/Prompt.cpp src/app/TitleBar.cpp src/app/BlockRenderer.cpp src/app/App.cpp`.

- [ ] **Step 4: Run tests, then run the binary**

Run: `cmake --build --preset default && ctest --preset default` → all pass.
Run: `./build/default/itsme` in Windows Terminal. Expected: title bar with clock, banner, prompt; typing `ab`+Tab completes to `about`; Enter renders the summary; Ctrl+L clears; Ctrl+D exits and the terminal is restored.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat(app): interactive FTXUI shell with prompt, history, completion and scrolling"
```

---

### Task 10: GitHub JSON parsing

**Files:**
- Create: `src/github/Parse.hpp`, `src/github/Parse.cpp`
- Create fixtures: `tests/fixtures/github_graphql.json`, `tests/fixtures/github_graphql_nouser.json`, `tests/fixtures/github_rest_user.json`, `tests/fixtures/github_rest_repos.json`, `tests/fixtures/repo_stats.json`
- Create: `tests/Fixtures.hpp`, `tests/github/parse_test.cpp`
- Modify: `CMakeLists.txt` (add Parse.cpp to `itsme_core`), `tests/CMakeLists.txt` (fixtures dir define)

**Interfaces:**
- Consumes: `github/Model.hpp`.
- Produces (namespace `itsme::github`):
  - `std::optional<GitHubStatsData> parseGraphQL(std::string_view body)`
  - `std::optional<GitHubStatsData> parseREST(std::string_view userJson, std::string_view reposJson)`
  - `std::optional<ProjectRepoStats> parseRepo(std::string_view body)`
  - `std::string languageColor(std::string_view language)` (site's table, `#8b8b8b` default)
  - `const char* graphqlQuery()`
  - Test helper: `std::string readFixture(const char* name)`.

- [ ] **Step 1: Write fixtures and failing tests**

`tests/fixtures/github_graphql.json`:
```json
{"data":{"user":{
  "name":"Leo","login":"dfansoo","followers":{"totalCount":42},"following":{"totalCount":7},
  "repositories":{"totalCount":30,"nodes":[
    {"name":"k3s","stargazerCount":10,"forkCount":2,"primaryLanguage":{"name":"Go","color":"#00ADD8"}},
    {"name":"rss","stargazerCount":5,"forkCount":1,"primaryLanguage":{"name":"Go","color":"#00ADD8"}},
    {"name":"site","stargazerCount":3,"forkCount":0,"primaryLanguage":{"name":"TypeScript","color":"#3178c6"}},
    {"name":"notes","stargazerCount":0,"forkCount":0,"primaryLanguage":null}
  ]},
  "pinnedItems":{"nodes":[
    {"name":"k3s","description":"K3s CI/CD","url":"https://github.com/DFanso/k3s","stargazerCount":10,"forkCount":2,"primaryLanguage":{"name":"Go","color":"#00ADD8"}}
  ]},
  "contributionsCollection":{
    "totalCommitContributions":812,"totalPullRequestContributions":40,"totalIssueContributions":9,"totalRepositoryContributions":12,
    "contributionCalendar":{"totalContributions":900,"weeks":[
      {"contributionDays":[{"contributionCount":0,"date":"2026-08-23","weekday":0},{"contributionCount":3,"date":"2026-08-24","weekday":1}]},
      {"contributionDays":[{"contributionCount":12,"date":"2026-08-30","weekday":0}]}
    ]}
  }
}}}
```

`tests/fixtures/github_graphql_nouser.json`:
```json
{"data":{"user":null},"errors":[{"message":"Could not resolve to a User"}]}
```

`tests/fixtures/github_rest_user.json`:
```json
{"login":"dfansoo","public_repos":25,"followers":40}
```

`tests/fixtures/github_rest_repos.json`:
```json
[
 {"name":"k3s","html_url":"https://github.com/DFanso/k3s","description":"K3s CI/CD","language":"Go","stargazers_count":10,"forks_count":2,"fork":false},
 {"name":"forked-thing","html_url":"https://github.com/DFanso/forked-thing","description":null,"language":"Python","stargazers_count":7,"forks_count":0,"fork":true},
 {"name":"rss","html_url":"https://github.com/DFanso/rss","description":"RSS reader","language":"Go","stargazers_count":5,"forks_count":1,"fork":false},
 {"name":"site","html_url":"https://github.com/DFanso/site","description":"Portfolio","language":"TypeScript","stargazers_count":3,"forks_count":0,"fork":false},
 {"name":"nolang","html_url":"https://github.com/DFanso/nolang","description":null,"language":null,"stargazers_count":0,"forks_count":0,"fork":false}
]
```

`tests/fixtures/repo_stats.json`:
```json
{"full_name":"DFanso/k3s","stargazers_count":10,"forks_count":2,"subscribers_count":4}
```

`tests/Fixtures.hpp`:
```cpp
#pragma once
#include <fstream>
#include <sstream>
#include <string>

inline std::string readFixture(const char* name) {
  std::ifstream in(std::string(ITSME_FIXTURES_DIR) + "/" + name, std::ios::binary);
  std::stringstream ss;
  ss << in.rdbuf();
  return ss.str();
}
```

`tests/github/parse_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include "../Fixtures.hpp"
#include "github/Parse.hpp"

using namespace itsme::github;

TEST_CASE("parseGraphQL maps the full-data branch") {
  auto d = parseGraphQL(readFixture("github_graphql.json"));
  REQUIRE(d.has_value());
  CHECK(d->hasFullData);
  CHECK(d->stats.commits == 812);
  CHECK(d->stats.prs == 40);
  CHECK(d->stats.issues == 9);
  CHECK(d->stats.repos == 30);
  CHECK(d->stats.stars == 18);
  CHECK(d->stats.forks == 3);
  CHECK(d->stats.followers == 42);
  CHECK(d->stats.contributions == 900);
  REQUIRE(d->languageStats.size() == 2);
  CHECK(d->languageStats[0].name == "Go");
  CHECK(d->languageStats[0].percentage == 67);
  CHECK(d->languageStats[0].color == "#00ADD8");
  CHECK(d->languageStats[1].name == "TypeScript");
  CHECK(d->languageStats[1].percentage == 33);
  CHECK(d->isPinned);
  REQUIRE(d->topRepos.size() == 1);
  CHECK(d->topRepos[0].url == "https://github.com/DFanso/k3s");
  CHECK(d->topRepos[0].description == std::optional<std::string>{"K3s CI/CD"});
  REQUIRE(d->calendar.has_value());
  CHECK(d->calendar->total == 900);
  REQUIRE(d->calendar->weeks.size() == 2);
  CHECK(d->calendar->weeks[0].days[1].count == 3);
  CHECK(d->calendar->weeks[1].days[0].date == "2026-08-30");
}

TEST_CASE("parseGraphQL returns nullopt for null user or garbage") {
  CHECK_FALSE(parseGraphQL(readFixture("github_graphql_nouser.json")).has_value());
  CHECK_FALSE(parseGraphQL("not json").has_value());
  CHECK_FALSE(parseGraphQL("{}").has_value());
}

TEST_CASE("parseREST maps the fallback branch") {
  auto d = parseREST(readFixture("github_rest_user.json"), readFixture("github_rest_repos.json"));
  REQUIRE(d.has_value());
  CHECK_FALSE(d->hasFullData);
  CHECK(d->stats.repos == 25);
  CHECK(d->stats.followers == 40);
  CHECK(d->stats.stars == 25);
  CHECK(d->stats.forks == 3);
  CHECK(d->stats.commits == 0);
  CHECK_FALSE(d->calendar.has_value());
  CHECK_FALSE(d->isPinned);
  REQUIRE(d->topRepos.size() == 4);  // forks excluded
  CHECK(d->topRepos[0].name == "k3s");
  CHECK(d->topRepos[0].languageColor == std::optional<std::string>{"#00ADD8"});
  CHECK_FALSE(d->topRepos[3].language.has_value());
  REQUIRE(d->languageStats.size() == 3);  // Go, Python, TypeScript (forks counted, as on the site)
  CHECK(d->languageStats[0].name == "Go");
  CHECK(d->languageStats[0].percentage == 50);
  CHECK_FALSE(parseREST("{}", "[]").has_value() == false);  // empty repos still parse
  CHECK_FALSE(parseREST("nope", "[]").has_value());
}

TEST_CASE("parseRepo and languageColor") {
  auto s = parseRepo(readFixture("repo_stats.json"));
  REQUIRE(s.has_value());
  CHECK(s->stars == 10); CHECK(s->forks == 2); CHECK(s->watchers == 4);
  CHECK_FALSE(parseRepo("[]").has_value());
  CHECK(languageColor("Go") == "#00ADD8");
  CHECK(languageColor("Brainfuck") == "#8b8b8b");
  CHECK(std::string(graphqlQuery()).find("contributionCalendar") != std::string::npos);
}
```

`tests/CMakeLists.txt`: add
```cmake
target_sources(itsme_tests PRIVATE github/parse_test.cpp)
target_compile_definitions(itsme_tests PRIVATE ITSME_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")
```

- [ ] **Step 2: Run to verify failure** → missing `github/Parse.hpp`.

- [ ] **Step 3: Implement**

`src/github/Parse.hpp`:
```cpp
#pragma once
#include <optional>
#include <string>
#include <string_view>
#include "github/Model.hpp"

namespace itsme::github {

const char* graphqlQuery();
std::string languageColor(std::string_view language);

std::optional<GitHubStatsData> parseGraphQL(std::string_view body);
std::optional<GitHubStatsData> parseREST(std::string_view userJson, std::string_view reposJson);
std::optional<ProjectRepoStats> parseRepo(std::string_view body);

}  // namespace itsme::github
```

`src/github/Parse.cpp`:
```cpp
#include "github/Parse.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <nlohmann/json.hpp>

namespace itsme::github {
using json = nlohmann::json;

const char* graphqlQuery() {
  return R"(query($username: String!) {
  user(login: $username) {
    name login avatarUrl bio
    followers { totalCount }
    following { totalCount }
    repositories(first: 100, ownerAffiliations: OWNER, orderBy: {field: STARGAZERS, direction: DESC}) {
      totalCount
      nodes { name stargazerCount forkCount primaryLanguage { name color } }
    }
    pinnedItems(first: 6, types: REPOSITORY) {
      nodes { ... on Repository { name description url stargazerCount forkCount primaryLanguage { name color } } }
    }
    contributionsCollection {
      totalCommitContributions totalPullRequestContributions totalIssueContributions totalRepositoryContributions
      contributionCalendar { totalContributions weeks { contributionDays { contributionCount date weekday } } }
    }
  }
})";
}

std::string languageColor(std::string_view language) {
  static const std::map<std::string, std::string, std::less<>> colors = {
      {"TypeScript", "#3178c6"}, {"JavaScript", "#f1e05a"}, {"Python", "#3572A5"}, {"Go", "#00ADD8"},
      {"Rust", "#dea584"},       {"Java", "#b07219"},       {"C#", "#178600"},     {"C++", "#f34b7d"},
      {"C", "#555555"},          {"PHP", "#4F5D95"},        {"Ruby", "#701516"},   {"Swift", "#F05138"},
      {"Kotlin", "#A97BFF"},     {"Dart", "#00B4AB"},       {"Shell", "#89e051"},  {"HTML", "#e34c26"},
      {"CSS", "#563d7c"},        {"Vue", "#41b883"},        {"Svelte", "#ff3e00"}, {"Astro", "#ff5a03"},
      {"HCL", "#844FBA"},        {"Dockerfile", "#384d54"},
  };
  auto it = colors.find(language);
  return it == colors.end() ? "#8b8b8b" : it->second;
}

namespace {

std::optional<std::string> optString(const json& j, const char* key) {
  if (!j.contains(key) || j[key].is_null()) return std::nullopt;
  return j[key].get<std::string>();
}

long optLong(const json& j, const char* key) {
  if (!j.contains(key) || !j[key].is_number()) return 0;
  return j[key].get<long>();
}

struct LangInput {
  std::optional<std::string> name;
  std::optional<std::string> color;
};

// Port of calculateLanguageStats: count, keep first-seen order, stable sort desc, top 8.
std::vector<LanguageStat> languageStatsFrom(const std::vector<LangInput>& repos) {
  struct Acc { std::string name; int count; std::string color; };
  std::vector<Acc> acc;
  for (const auto& r : repos) {
    if (!r.name) continue;
    auto it = std::find_if(acc.begin(), acc.end(), [&](const Acc& a) { return a.name == *r.name; });
    if (it == acc.end()) acc.push_back({*r.name, 1, r.color.value_or("#8b8b8b")});
    else ++it->count;
  }
  int total = 0;
  for (const auto& a : acc) total += a.count;
  std::stable_sort(acc.begin(), acc.end(), [](const Acc& a, const Acc& b) { return a.count > b.count; });
  if (acc.size() > 8) acc.resize(8);
  std::vector<LanguageStat> out;
  for (const auto& a : acc)
    out.push_back({a.name, total ? static_cast<int>(std::lround(100.0 * a.count / total)) : 0, a.color});
  return out;
}

}  // namespace

std::optional<GitHubStatsData> parseGraphQL(std::string_view body) {
  try {
    const json root = json::parse(body);
    if (!root.contains("data") || !root["data"].contains("user") || root["data"]["user"].is_null()) return std::nullopt;
    const json& user = root["data"]["user"];

    GitHubStatsData d;
    d.hasFullData = true;

    const json& repoNodes = user.at("repositories").at("nodes");
    std::vector<LangInput> langs;
    for (const auto& r : repoNodes) {
      LangInput li;
      if (r.contains("primaryLanguage") && !r["primaryLanguage"].is_null()) {
        li.name = optString(r["primaryLanguage"], "name");
        li.color = optString(r["primaryLanguage"], "color");
      }
      langs.push_back(li);
      d.stats.stars += optLong(r, "stargazerCount");
      d.stats.forks += optLong(r, "forkCount");
    }
    d.languageStats = languageStatsFrom(langs);

    const json& contrib = user.at("contributionsCollection");
    const json& cal = contrib.at("contributionCalendar");
    d.stats.commits = optLong(contrib, "totalCommitContributions");
    d.stats.prs = optLong(contrib, "totalPullRequestContributions");
    d.stats.issues = optLong(contrib, "totalIssueContributions");
    d.stats.repos = optLong(user.at("repositories"), "totalCount");
    d.stats.followers = optLong(user.at("followers"), "totalCount");
    d.stats.contributions = optLong(cal, "totalContributions");

    const json& pinned = user.at("pinnedItems").at("nodes");
    d.isPinned = !pinned.empty();
    const json& source = d.isPinned ? pinned : repoNodes;
    std::size_t limit = d.isPinned ? source.size() : std::min<std::size_t>(6, source.size());
    for (std::size_t i = 0; i < limit; ++i) {
      const json& r = source[i];
      TopRepo tr;
      tr.name = r.value("name", "");
      tr.url = r.value("url", "");
      tr.description = optString(r, "description");
      if (r.contains("primaryLanguage") && !r["primaryLanguage"].is_null()) {
        tr.language = optString(r["primaryLanguage"], "name");
        tr.languageColor = optString(r["primaryLanguage"], "color");
      }
      tr.stars = optLong(r, "stargazerCount");
      tr.forks = optLong(r, "forkCount");
      d.topRepos.push_back(std::move(tr));
    }

    ContributionCalendar c;
    c.total = optLong(cal, "totalContributions");
    for (const auto& w : cal.at("weeks")) {
      ContributionWeek week;
      for (const auto& day : w.at("contributionDays"))
        week.days.push_back({static_cast<int>(optLong(day, "contributionCount")), day.value("date", ""),
                             static_cast<int>(optLong(day, "weekday"))});
      c.weeks.push_back(std::move(week));
    }
    d.calendar = std::move(c);
    return d;
  } catch (const json::exception&) {
    return std::nullopt;
  }
}

std::optional<GitHubStatsData> parseREST(std::string_view userJson, std::string_view reposJson) {
  try {
    const json user = json::parse(userJson);
    const json repos = json::parse(reposJson);
    if (!user.is_object() || !repos.is_array()) return std::nullopt;

    GitHubStatsData d;
    d.hasFullData = false;
    std::vector<LangInput> langs;
    for (const auto& r : repos) {
      LangInput li;
      li.name = optString(r, "language");
      if (li.name) li.color = languageColor(*li.name);
      langs.push_back(li);
      d.stats.stars += optLong(r, "stargazers_count");
      d.stats.forks += optLong(r, "forks_count");
    }
    d.languageStats = languageStatsFrom(langs);
    d.stats.repos = optLong(user, "public_repos");
    d.stats.followers = optLong(user, "followers");

    for (const auto& r : repos) {
      if (r.value("fork", false)) continue;
      if (d.topRepos.size() >= 6) break;
      TopRepo tr;
      tr.name = r.value("name", "");
      tr.url = r.value("html_url", "");
      tr.description = optString(r, "description");
      tr.language = optString(r, "language");
      if (tr.language) tr.languageColor = languageColor(*tr.language);
      tr.stars = optLong(r, "stargazers_count");
      tr.forks = optLong(r, "forks_count");
      d.topRepos.push_back(std::move(tr));
    }
    return d;
  } catch (const json::exception&) {
    return std::nullopt;
  }
}

std::optional<ProjectRepoStats> parseRepo(std::string_view body) {
  try {
    const json j = json::parse(body);
    if (!j.is_object()) return std::nullopt;
    return ProjectRepoStats{optLong(j, "stargazers_count"), optLong(j, "forks_count"), optLong(j, "subscribers_count")};
  } catch (const json::exception&) {
    return std::nullopt;
  }
}

}  // namespace itsme::github
```

Add `src/github/Parse.cpp` to `itsme_core`.

- [ ] **Step 4: Run tests** → all pass.

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "feat(github): GraphQL/REST parsers with fixtures"
```

---

### Task 11: GitHub client (libcurl), async fetch, GitHub stats output, App wiring

**Files:**
- Create: `src/github/Client.hpp`, `src/github/Client.cpp`, `src/github/CurlHttp.hpp`, `src/github/CurlHttp.cpp`, `src/core/AsyncValue.hpp`, `src/outputs/GitHubStats.hpp`, `src/outputs/GitHubStats.cpp`
- Modify: `cmake/Dependencies.cmake` (curl), `CMakeLists.txt` (Client.cpp → `itsme_core`; new lib `itsme_net`; `itsme` links it), `src/app/BlockRenderer.hpp/.cpp`, `src/app/App.hpp/.cpp`, `src/main.cpp`
- Test: `tests/github/client_test.cpp`, `tests/outputs/github_stats_test.cpp`, `tests/core/async_value_test.cpp`

**Interfaces:**
- Produces (namespace `itsme::github`):
  - `struct HttpResponse { long status = 0; std::string body; }`
  - `using HttpFn = std::function<std::optional<HttpResponse>(const std::string& url, const std::vector<std::string>& headers, const std::optional<std::string>& postBody)>;`
  - `class Client { Client(std::optional<std::string> token, HttpFn http); std::optional<GitHubStatsData> fetchStats() const; ProjectStatsMap fetchProjectStats(const std::vector<std::string>& repos) const; bool hasToken() const; }`
  - `std::optional<std::string> tokenFromEnv()` (reads `GITHUB_TOKEN`, empty → nullopt)
  - `HttpFn curlHttp(long timeoutSeconds = 10)` (in `CurlHttp.hpp`, lib `itsme_net`)
- Produces (namespace `itsme::core`): `template <class T> class AsyncValue { static std::shared_ptr<AsyncValue> start(std::function<T()> work, std::function<void()> onDone); bool ready() const; const T& get() const; }`
- Produces (namespace `itsme::outputs`): `struct GitHubView { enum class Status { Loading, Ready, Failed }; Status status; std::optional<github::GitHubStatsData> data; int spinnerFrame = 0; }`, `ftxui::Element renderGitHubStats(const GitHubView&, const RenderContext&)`
- Modifies `app::BlockRuntime`: adds `std::shared_ptr<core::AsyncValue<std::optional<github::GitHubStatsData>>> githubFetch; std::shared_ptr<core::AsyncValue<github::ProjectStatsMap>> projectsFetch; int spinnerFrame = 0;` and removes the `projects` field (the fetch replaces it).
- Modifies `app::App`: constructor becomes `App(Options, std::shared_ptr<const github::Client> client)` (client may be null = offline).

- [ ] **Step 1: Write the failing tests**

`tests/core/async_value_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include <atomic>
#include <chrono>
#include <thread>
#include "core/AsyncValue.hpp"

TEST_CASE("AsyncValue runs work off-thread and signals completion") {
  std::atomic<int> done{0};
  auto v = itsme::core::AsyncValue<int>::start([] { return 41 + 1; }, [&] { done.fetch_add(1); });
  for (int i = 0; i < 200 && !v->ready(); ++i) std::this_thread::sleep_for(std::chrono::milliseconds(5));
  REQUIRE(v->ready());
  CHECK(v->get() == 42);
  for (int i = 0; i < 200 && done.load() == 0; ++i) std::this_thread::sleep_for(std::chrono::milliseconds(5));
  CHECK(done.load() == 1);
}
```

`tests/github/client_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include <vector>
#include "../Fixtures.hpp"
#include "github/Client.hpp"

using namespace itsme::github;

namespace {
struct FakeHttp {
  std::vector<std::string> urls;
  std::vector<std::vector<std::string>> headers;
  long graphqlStatus = 200;
  bool restOk = true;

  HttpFn fn() {
    return [this](const std::string& url, const std::vector<std::string>& hdrs,
                  const std::optional<std::string>& body) -> std::optional<HttpResponse> {
      urls.push_back(url);
      headers.push_back(hdrs);
      if (url == "https://api.github.com/graphql") {
        REQUIRE(body.has_value());
        return HttpResponse{graphqlStatus, graphqlStatus == 200 ? readFixture("github_graphql.json") : "{}"};
      }
      if (!restOk) return std::nullopt;
      if (url == "https://api.github.com/users/dfansoo") return HttpResponse{200, readFixture("github_rest_user.json")};
      if (url.rfind("https://api.github.com/users/dfansoo/repos", 0) == 0) return HttpResponse{200, readFixture("github_rest_repos.json")};
      if (url == "https://api.github.com/repos/DFanso/k3s") return HttpResponse{200, readFixture("repo_stats.json")};
      return HttpResponse{404, "{}"};
    };
  }
};
}  // namespace

TEST_CASE("with a token, GraphQL is used and the bearer header is sent") {
  FakeHttp http;
  Client c(std::string("tok"), http.fn());
  auto d = c.fetchStats();
  REQUIRE(d.has_value());
  CHECK(d->hasFullData);
  REQUIRE(http.urls.size() == 1);
  bool hasAuth = false;
  for (auto& h : http.headers[0]) if (h == "Authorization: Bearer tok") hasAuth = true;
  CHECK(hasAuth);
}

TEST_CASE("GraphQL failure falls back to REST; no token goes straight to REST") {
  FakeHttp http;
  http.graphqlStatus = 401;
  Client c(std::string("bad"), http.fn());
  auto d = c.fetchStats();
  REQUIRE(d.has_value());
  CHECK_FALSE(d->hasFullData);
  CHECK(http.urls.size() == 3);

  FakeHttp http2;
  Client c2(std::nullopt, http2.fn());
  REQUIRE(c2.fetchStats().has_value());
  CHECK(http2.urls.size() == 2);
  CHECK_FALSE(c2.hasToken());
}

TEST_CASE("both sources failing yields nullopt") {
  FakeHttp http;
  http.graphqlStatus = 500;
  http.restOk = false;
  Client c(std::string("tok"), http.fn());
  CHECK_FALSE(c.fetchStats().has_value());
}

TEST_CASE("fetchProjectStats maps per repo, null on failure") {
  FakeHttp http;
  Client c(std::nullopt, http.fn());
  auto m = c.fetchProjectStats({"DFanso/k3s", "DFanso/missing"});
  REQUIRE(m.size() == 2);
  REQUIRE(m["DFanso/k3s"].has_value());
  CHECK(m["DFanso/k3s"]->watchers == 4);
  CHECK_FALSE(m["DFanso/missing"].has_value());
}
```

`tests/outputs/github_stats_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include "../Fixtures.hpp"
#include "../RenderHelper.hpp"
#include "github/Parse.hpp"
#include "outputs/GitHubStats.hpp"

using namespace itsme::outputs;

TEST_CASE("github stats states") {
  GitHubView loading;
  CHECK(renderPlain(renderGitHubStats(loading, RenderContext{100, 12})).find("Fetching GitHub stats...") != std::string::npos);

  GitHubView failed;
  failed.status = GitHubView::Status::Failed;
  CHECK(renderPlain(renderGitHubStats(failed, RenderContext{100, 12})).find("Unable to fetch GitHub data") != std::string::npos);

  GitHubView ready;
  ready.status = GitHubView::Status::Ready;
  ready.data = itsme::github::parseGraphQL(readFixture("github_graphql.json"));
  auto s = renderPlain(renderGitHubStats(ready, RenderContext{100, 12}), 100, 40);
  CHECK(s.find("GitHub Statistics") != std::string::npos);
  CHECK(s.find("@dfansoo") != std::string::npos);
  CHECK(s.find("812") != std::string::npos);
  CHECK(s.find("Commits") != std::string::npos);
  CHECK(s.find("Go 67%") != std::string::npos);
  CHECK(s.find("contributions.heatmap") != std::string::npos);
  CHECK(s.find("900 contributions this year") != std::string::npos);
  CHECK(s.find("k3s") != std::string::npos);

  auto narrow = renderPlain(renderGitHubStats(ready, RenderContext{60, 12}), 60, 40);
  CHECK(narrow.find("contributions.heatmap") == std::string::npos);
}
```

`tests/CMakeLists.txt`: add the three test files via `target_sources`.

- [ ] **Step 2: Run to verify failure** → missing headers.

- [ ] **Step 3: Implement the core pieces**

`src/core/AsyncValue.hpp`:
```cpp
#pragma once
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>

namespace itsme::core {

// Runs `work` on a detached thread. The result is readable once ready() is true.
// `onDone` runs on the worker thread after the value is stored (use it to poke the UI loop).
template <class T>
class AsyncValue {
 public:
  static std::shared_ptr<AsyncValue> start(std::function<T()> work, std::function<void()> onDone) {
    auto self = std::make_shared<AsyncValue>();
    std::thread([self, work = std::move(work), onDone = std::move(onDone)] {
      T result = work();
      {
        std::lock_guard<std::mutex> lock(self->mutex_);
        self->value_ = std::move(result);
      }
      self->ready_.store(true, std::memory_order_release);
      if (onDone) onDone();
    }).detach();
    return self;
  }

  bool ready() const { return ready_.load(std::memory_order_acquire); }
  const T& get() const { return *value_; }  // only valid when ready()

 private:
  std::atomic<bool> ready_{false};
  std::mutex mutex_;
  std::optional<T> value_;
};

}  // namespace itsme::core
```

`src/github/Client.hpp`:
```cpp
#pragma once
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include "github/Model.hpp"

namespace itsme::github {

struct HttpResponse {
  long status = 0;
  std::string body;
};

using HttpFn = std::function<std::optional<HttpResponse>(
    const std::string& url, const std::vector<std::string>& headers, const std::optional<std::string>& postBody)>;

class Client {
 public:
  Client(std::optional<std::string> token, HttpFn http);
  std::optional<GitHubStatsData> fetchStats() const;
  ProjectStatsMap fetchProjectStats(const std::vector<std::string>& repos) const;
  bool hasToken() const { return token_.has_value(); }

 private:
  std::optional<std::string> token_;
  HttpFn http_;
};

std::optional<std::string> tokenFromEnv();

}  // namespace itsme::github
```

`src/github/Client.cpp`:
```cpp
#include "github/Client.hpp"
#include <cstdlib>
#include <future>
#include <nlohmann/json.hpp>
#include "github/Parse.hpp"

namespace itsme::github {

namespace {
const std::vector<std::string> kRestHeaders = {"Accept: application/vnd.github.v3+json", "User-Agent: Portfolio-Site"};
}

Client::Client(std::optional<std::string> token, HttpFn http) : token_(std::move(token)), http_(std::move(http)) {}

std::optional<GitHubStatsData> Client::fetchStats() const {
  if (token_) {
    nlohmann::json body = {{"query", graphqlQuery()}, {"variables", {{"username", kUsername}}}};
    auto resp = http_("https://api.github.com/graphql",
                      {"Authorization: Bearer " + *token_, "Content-Type: application/json", "User-Agent: Portfolio-Site"},
                      body.dump());
    if (resp && resp->status == 200)
      if (auto data = parseGraphQL(resp->body)) return data;
  }
  auto user = http_(std::string("https://api.github.com/users/") + kUsername, kRestHeaders, std::nullopt);
  auto repos = http_(std::string("https://api.github.com/users/") + kUsername + "/repos?sort=stars&direction=desc&per_page=100",
                     kRestHeaders, std::nullopt);
  if (user && repos && user->status == 200 && repos->status == 200) return parseREST(user->body, repos->body);
  return std::nullopt;
}

ProjectStatsMap Client::fetchProjectStats(const std::vector<std::string>& repos) const {
  std::vector<std::string> headers = kRestHeaders;
  if (token_) headers.push_back("Authorization: Bearer " + *token_);

  std::vector<std::pair<std::string, std::future<std::optional<ProjectRepoStats>>>> futures;
  for (const auto& repo : repos) {
    futures.emplace_back(repo, std::async(std::launch::async, [this, repo, headers]() -> std::optional<ProjectRepoStats> {
      auto resp = http_("https://api.github.com/repos/" + repo, headers, std::nullopt);
      if (!resp || resp->status != 200) return std::nullopt;
      return parseRepo(resp->body);
    }));
  }
  ProjectStatsMap out;
  for (auto& [repo, fut] : futures) out[repo] = fut.get();
  return out;
}

std::optional<std::string> tokenFromEnv() {
  const char* v = std::getenv("GITHUB_TOKEN");
  if (v == nullptr || *v == '\0') return std::nullopt;
  return std::string(v);
}

}  // namespace itsme::github
```

`src/github/CurlHttp.hpp`:
```cpp
#pragma once
#include "github/Client.hpp"
namespace itsme::github {
HttpFn curlHttp(long timeoutSeconds = 10);
}
```

`src/github/CurlHttp.cpp`:
```cpp
#include "github/CurlHttp.hpp"
#include <curl/curl.h>
#include <mutex>

namespace itsme::github {

namespace {
std::size_t writeBody(char* ptr, std::size_t size, std::size_t nmemb, void* userdata) {
  auto* out = static_cast<std::string*>(userdata);
  out->append(ptr, size * nmemb);
  return size * nmemb;
}

void ensureGlobalInit() {
  static std::once_flag once;
  std::call_once(once, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });
}
}  // namespace

HttpFn curlHttp(long timeoutSeconds) {
  ensureGlobalInit();
  return [timeoutSeconds](const std::string& url, const std::vector<std::string>& headers,
                          const std::optional<std::string>& postBody) -> std::optional<HttpResponse> {
    CURL* curl = curl_easy_init();
    if (!curl) return std::nullopt;

    struct curl_slist* list = nullptr;
    for (const auto& h : headers) list = curl_slist_append(list, h.c_str());

    std::string body;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeBody);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSeconds);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeoutSeconds);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    if (postBody) {
      curl_easy_setopt(curl, CURLOPT_POST, 1L);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postBody->c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(postBody->size()));
    }

    const CURLcode rc = curl_easy_perform(curl);
    long status = 0;
    if (rc == CURLE_OK) curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_slist_free_all(list);
    curl_easy_cleanup(curl);
    if (rc != CURLE_OK) return std::nullopt;
    return HttpResponse{status, std::move(body)};
  };
}

}  // namespace itsme::github
```

Append to `cmake/Dependencies.cmake`:
```cmake
find_package(CURL QUIET)
if(NOT CURL_FOUND)
  message(STATUS "libcurl not found on the system; building it with FetchContent")
  set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
  set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
  set(BUILD_LIBCURL_DOCS OFF CACHE BOOL "" FORCE)
  set(BUILD_MISC_DOCS OFF CACHE BOOL "" FORCE)
  set(ENABLE_CURL_MANUAL OFF CACHE BOOL "" FORCE)
  set(HTTP_ONLY ON CACHE BOOL "" FORCE)
  set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
  set(CURL_USE_LIBSSH2 OFF CACHE BOOL "" FORCE)
  set(CURL_ZLIB OFF CACHE BOOL "" FORCE)
  set(CURL_BROTLI OFF CACHE BOOL "" FORCE)
  set(CURL_ZSTD OFF CACHE BOOL "" FORCE)
  set(USE_NGHTTP2 OFF CACHE BOOL "" FORCE)
  set(USE_LIBIDN2 OFF CACHE BOOL "" FORCE)
  set(CURL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
  if(WIN32)
    set(CURL_USE_SCHANNEL ON CACHE BOOL "" FORCE)
  elseif(APPLE)
    set(CURL_USE_SECTRANSP ON CACHE BOOL "" FORCE)
  else()
    set(CURL_USE_OPENSSL ON CACHE BOOL "" FORCE)
  endif()
  FetchContent_Declare(curl
    URL https://github.com/curl/curl/releases/download/curl-8_10_1/curl-8.10.1.tar.xz)
  FetchContent_MakeAvailable(curl)
endif()
```

`CMakeLists.txt`: add `src/github/Client.cpp` to `itsme_core`; add
```cmake
add_library(itsme_net STATIC src/github/CurlHttp.cpp)
target_link_libraries(itsme_net PUBLIC itsme_core CURL::libcurl)
itsme_set_warnings(itsme_net)
```
and `target_link_libraries(itsme PRIVATE itsme_ui itsme_net)`. (`itsme_tests` must NOT link `itsme_net`.)

- [ ] **Step 4: Implement the GitHub stats output**

`src/outputs/GitHubStats.hpp`:
```cpp
#pragma once
#include <optional>
#include <ftxui/dom/elements.hpp>
#include "github/Model.hpp"
#include "outputs/Outputs.hpp"

namespace itsme::outputs {

struct GitHubView {
  enum class Status { Loading, Ready, Failed };
  Status status = Status::Loading;
  std::optional<github::GitHubStatsData> data;
  int spinnerFrame = 0;
};

ftxui::Element renderGitHubStats(const GitHubView& view, const RenderContext& ctx);
extern const char* const kSpinnerFrames[10];

}  // namespace itsme::outputs
```

`src/outputs/GitHubStats.cpp`:
```cpp
#include "outputs/GitHubStats.hpp"
#include <algorithm>
#include "outputs/Common.hpp"
#include "outputs/Theme.hpp"

namespace itsme::outputs {
using namespace ftxui;
using core::Tone;

const char* const kSpinnerFrames[10] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};

namespace {
Element stat(long value, const char* label) {
  return vbox({text(std::to_string(value)) | color(tone(Tone::Fg)) | bold, t(label, Tone::Muted)}) | size(WIDTH, EQUAL, 16);
}

Color heatColor(int count) {
  if (count <= 0) return trueColor() ? Color::RGB(0x2a, 0x2c, 0x3a) : Color::GrayDark;
  if (count <= 2) return hexColor("#0e4429", Color::Green);
  if (count <= 5) return hexColor("#006d32", Color::Green);
  if (count <= 9) return hexColor("#26a641", Color::GreenLight);
  return hexColor("#39d353", Color::GreenLight);
}

Element languageBar(const std::vector<github::LanguageStat>& langs) {
  constexpr int kWidth = 40;
  Elements segments;
  int used = 0;
  for (std::size_t i = 0; i < langs.size(); ++i) {
    int cells = (i + 1 == langs.size()) ? kWidth - used : std::max(1, kWidth * langs[i].percentage / 100);
    cells = std::max(0, std::min(cells, kWidth - used));
    used += cells;
    std::string bar;
    for (int c = 0; c < cells; ++c) bar += "█";
    segments.push_back(text(bar) | color(hexColor(langs[i].color)));
  }
  Elements legend;
  for (const auto& l : langs)
    legend.push_back(hbox({text("● ") | color(hexColor(l.color)), t(l.name + " " + std::to_string(l.percentage) + "%  ", Tone::Fg)}));
  return vbox({hbox(std::move(segments)), hflow(std::move(legend))});
}

Element heatmap(const github::ContributionCalendar& cal, int width) {
  const int maxWeeks = std::max(0, std::min<int>(static_cast<int>(cal.weeks.size()), width - 6));
  const std::size_t first = cal.weeks.size() - static_cast<std::size_t>(maxWeeks);
  Elements rows;
  for (int weekday = 0; weekday < 7; ++weekday) {
    Elements cells;
    for (std::size_t w = first; w < cal.weeks.size(); ++w) {
      int count = -1;
      for (const auto& d : cal.weeks[w].days)
        if (d.weekday == weekday) count = d.count;
      cells.push_back(count < 0 ? text(" ") : text("■") | color(heatColor(count)));
    }
    rows.push_back(hbox(std::move(cells)));
  }
  Elements legend = {t("Less ", Tone::Muted)};
  for (int c : {0, 1, 3, 6, 10}) legend.push_back(text("■") | color(heatColor(c)));
  legend.push_back(t(" More", Tone::Muted));
  rows.push_back(hbox(std::move(legend)));
  return vbox(std::move(rows));
}
}  // namespace

Element renderGitHubStats(const GitHubView& view, const RenderContext& ctx) {
  Elements rows = {heading("GitHub Statistics")};
  if (view.status == GitHubView::Status::Loading) {
    rows.push_back(hbox({t(std::string(kSpinnerFrames[view.spinnerFrame % 10]) + " ", Tone::Blue),
                         t("Fetching GitHub stats...", Tone::Muted)}));
    return vbox(std::move(rows));
  }
  if (view.status == GitHubView::Status::Failed || !view.data) {
    rows.push_back(hbox({branch(true), t("Unable to fetch GitHub data. API may be rate limited or unreachable.", Tone::Red)}));
    return vbox(std::move(rows));
  }
  const auto& d = *view.data;
  const auto& s = d.stats;

  Elements who = {t(std::string("@") + github::kUsername, Tone::Blue)};
  if (d.hasFullData) who.push_back(t("  (Last Year)", Tone::Green));
  rows.push_back(hbox(std::move(who)));
  rows.push_back(blank());

  Elements grid1, grid2;
  if (d.hasFullData) {
    grid1 = {stat(s.commits, "Commits"), stat(s.prs, "Pull Requests"), stat(s.issues, "Issues"), stat(s.contributions, "Contributions")};
  }
  grid2 = {stat(s.stars, "Total Stars"), stat(s.forks, "Total Forks"), stat(s.repos, "Repositories"), stat(s.followers, "Followers")};
  if (!grid1.empty()) { rows.push_back(indent(hbox(std::move(grid1)), 4)); rows.push_back(blank()); }
  rows.push_back(indent(hbox(std::move(grid2)), 4));
  rows.push_back(blank());

  if (!d.languageStats.empty()) {
    rows.push_back(hbox({branch(false), t("cat ", Tone::Yellow), t("languages", Tone::Blue)}));
    rows.push_back(indent(languageBar(d.languageStats), 4));
    rows.push_back(blank());
  }

  if (d.calendar && ctx.width >= 80) {
    rows.push_back(hbox({branch(false), t("cat ", Tone::Yellow), t("contributions.heatmap", Tone::Blue)}));
    rows.push_back(indent(heatmap(*d.calendar, ctx.width - 8), 4));
    rows.push_back(indent(t(std::to_string(s.contributions) + " contributions this year", Tone::Green), 4));
    rows.push_back(blank());
  }

  rows.push_back(hbox({branch(true), t("cat ", Tone::Yellow), t(d.isPinned ? "pinned-repos" : "top-repos", Tone::Blue)}));
  for (const auto& r : d.topRepos) {
    Elements line = {t(r.name, Tone::Blue) | bold};
    if (r.language) {
      line.push_back(text("  ● ") | color(hexColor(r.languageColor.value_or("#8b8b8b"))));
      line.push_back(t(*r.language, Tone::Muted));
    }
    line.push_back(t("  ★ " + std::to_string(r.stars), Tone::Yellow));
    line.push_back(t("  ⑂ " + std::to_string(r.forks), Tone::Cyan));
    rows.push_back(indent(hbox(std::move(line)), 4));
    if (r.description && !r.description->empty()) rows.push_back(indent(para(*r.description, Tone::Muted), 6));
    if (!r.url.empty()) rows.push_back(indent(t(r.url, Tone::Muted) | dim, 6));
  }
  return vbox(std::move(rows));
}

}  // namespace itsme::outputs
```
Add `src/outputs/GitHubStats.cpp` to `itsme_ui`.

- [ ] **Step 5: Wire into the App**

`src/app/BlockRenderer.hpp` — replace `BlockRuntime` with:
```cpp
struct BlockRuntime {
  std::string timeString;
  int typewriterRevealed = -1;
  int spinnerFrame = 0;
  std::shared_ptr<core::AsyncValue<std::optional<github::GitHubStatsData>>> githubFetch;  // null = offline
  std::shared_ptr<core::AsyncValue<github::ProjectStatsMap>> projectsFetch;
};
```
(add `#include "core/AsyncValue.hpp"` and `#include <optional>`).

`src/app/BlockRenderer.cpp` — in `renderComponent`, replace the `projects` and `github` lines with:
```cpp
  if (name == "projects") {
    const github::ProjectStatsMap* stats =
        (rt.projectsFetch && rt.projectsFetch->ready()) ? &rt.projectsFetch->get() : nullptr;
    return renderProjects(stats);
  }
  if (name == "github") {
    GitHubView view;
    view.spinnerFrame = rt.spinnerFrame;
    if (!rt.githubFetch) view.status = GitHubView::Status::Failed;
    else if (!rt.githubFetch->ready()) view.status = GitHubView::Status::Loading;
    else {
      view.data = rt.githubFetch->get();
      view.status = view.data ? GitHubView::Status::Ready : GitHubView::Status::Failed;
    }
    return renderGitHubStats(view, ctx);
  }
```
(add `#include "outputs/GitHubStats.hpp"`).

`src/app/App.hpp` — change constructor and add members:
```cpp
  App(Options opts, std::shared_ptr<const github::Client> client);
  ...
 protected:
  void startFetches(const core::Block& block);
  std::shared_ptr<const github::Client> client_;
  struct Redraw {  // shared with worker threads; screen pointer cleared on exit
    std::mutex mutex;
    ftxui::ScreenInteractive* screen = nullptr;
    void post();
  };
  std::shared_ptr<Redraw> redraw_ = std::make_shared<Redraw>();
```
(add `#include <mutex>` and `#include "github/Client.hpp"`.)

`src/app/App.cpp` changes:
```cpp
App::App(Options opts, std::shared_ptr<const github::Client> client)
    : opts_(opts), state_(core::initialState()), client_(std::move(client)) {
  outputs::setTrueColor(!opts_.noColor);
}

void App::Redraw::post() {
  std::lock_guard<std::mutex> lock(mutex);
  if (screen) screen->PostEvent(Event::Custom);
}

int App::run() {
  auto screen = ScreenInteractive::Fullscreen();
  screen_ = &screen;
  { std::lock_guard<std::mutex> lock(redraw_->mutex); redraw_->screen = &screen; }
  screen.Loop(component());
  { std::lock_guard<std::mutex> lock(redraw_->mutex); redraw_->screen = nullptr; }
  screen_ = nullptr;
  return 0;
}

void App::requestRedraw() { redraw_->post(); }

void App::onBlockAdded(const core::Block& block) {
  if (block.execution.kind != core::ExecKind::Component) return;
  if (block.execution.componentName == "time") runtime_[block.id].timeString = clockHHMMSS(localNow());
  startFetches(block);
}

void App::startFetches(const core::Block& block) {
  if (!client_) return;
  auto client = client_;
  auto redraw = redraw_;
  const auto& name = block.execution.componentName;
  if (name == "github") {
    runtime_[block.id].githubFetch = core::AsyncValue<std::optional<github::GitHubStatsData>>::start(
        [client] { return client->fetchStats(); }, [redraw] { redraw->post(); });
  } else if (name == "projects") {
    std::vector<std::string> repos;
    for (const auto& p : data::projects())
      if (p.github) repos.push_back(*p.github);
    runtime_[block.id].projectsFetch = core::AsyncValue<github::ProjectStatsMap>::start(
        [client, repos] { return client->fetchProjectStats(repos); }, [redraw] { redraw->post(); });
  }
}
```
(add `#include "data/Portfolio.hpp"`.) Update the smoke test constructor calls to `App app(Options{true, false}, nullptr);`.

`src/main.cpp`: add `#include <memory>`, `#include "github/Client.hpp"`, `#include "github/CurlHttp.hpp"` and replace the last two lines with:
```cpp
  auto client = std::make_shared<const itsme::github::Client>(itsme::github::tokenFromEnv(), itsme::github::curlHttp());
  itsme::app::App app(opts, client);
  return app.run();
```

- [ ] **Step 6: Build, test, run**

Run: `cmake --preset default && cmake --build --preset default && ctest --preset default` (reconfigure needed: curl is fetched/built on Windows, a few minutes).
Expected: all tests pass. Run `./build/default/itsme`, type `github` → spinner line, then stats appear (REST without token). `GITHUB_TOKEN=... ./build/default/itsme` → commits/PRs/heatmap shown. `projects` → stars/forks/watchers appear after a moment.

- [ ] **Step 7: Commit**

```bash
git add -A
git commit -m "feat(github): libcurl client, async fetch and GitHub stats output"
```

---

### Task 12: Ticker, boot sequence, typewriter, live clock and spinner

**Files:**
- Create: `src/effects/Ticker.hpp`, `src/effects/Ticker.cpp`, `src/effects/Boot.hpp`, `src/effects/Boot.cpp`, `src/effects/Typewriter.hpp`, `src/effects/Typewriter.cpp`
- Modify: `src/app/App.hpp`, `src/app/App.cpp`, `CMakeLists.txt`, `tests/CMakeLists.txt`
- Test: `tests/effects/boot_typewriter_test.cpp`, `tests/effects/ticker_test.cpp`

**Interfaces:**
- Produces (namespace `itsme::effects`):
  - `class Ticker { Ticker(std::chrono::milliseconds interval, std::function<void()> onTick); ~Ticker(); void setInterval(std::chrono::milliseconds); }` — background thread, stops in destructor.
  - `class BootSequence { void advance(int ms); bool done() const; std::vector<std::string> visibleLines() const; static constexpr int kDurationMs = 1100; }` — lines appear at 0/300/600/900 ms.
  - `class Typewriter { explicit Typewriter(int totalChars, int msPerChar = 15); void advance(int ms); int revealed() const; bool done() const; void finish(); }`
- App: handles tick event `Event::Special("itsme-tick")`; renders the boot screen while `boot_` is active; drives `runtime_[0].typewriterRevealed` from `typewriter_`; increments `spinnerFrame` on loading github blocks; ticker interval 50 ms while animating, 1000 ms idle.

- [ ] **Step 1: Write the failing tests**

`tests/effects/boot_typewriter_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include "effects/Boot.hpp"
#include "effects/Typewriter.hpp"

using namespace itsme::effects;

TEST_CASE("boot sequence reveals lines over time") {
  BootSequence boot;
  CHECK(boot.visibleLines().size() == 1);
  CHECK_FALSE(boot.done());
  boot.advance(299);
  CHECK(boot.visibleLines().size() == 1);
  boot.advance(1);
  CHECK(boot.visibleLines().size() == 2);
  boot.advance(600);
  CHECK(boot.visibleLines().size() == 4);
  CHECK(boot.visibleLines()[0].find("Booting portfolio OS") != std::string::npos);
  CHECK_FALSE(boot.done());
  boot.advance(200);
  CHECK(boot.done());
}

TEST_CASE("typewriter reveals characters at a fixed rate and can finish early") {
  Typewriter tw(100, 15);
  CHECK(tw.revealed() == 0);
  tw.advance(14);
  CHECK(tw.revealed() == 0);
  tw.advance(1);
  CHECK(tw.revealed() == 1);
  tw.advance(150);
  CHECK(tw.revealed() == 11);
  tw.advance(100000);
  CHECK(tw.revealed() == 100);
  CHECK(tw.done());
  Typewriter tw2(10);
  tw2.finish();
  CHECK(tw2.done());
  CHECK(tw2.revealed() == 10);
}
```

`tests/effects/ticker_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include <atomic>
#include <chrono>
#include <thread>
#include "effects/Ticker.hpp"

TEST_CASE("ticker fires repeatedly and stops on destruction") {
  std::atomic<int> ticks{0};
  {
    itsme::effects::Ticker ticker(std::chrono::milliseconds(10), [&] { ticks.fetch_add(1); });
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
  }
  const int afterStop = ticks.load();
  CHECK(afterStop >= 3);
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  CHECK(ticks.load() == afterStop);
}
```

`tests/CMakeLists.txt`: add both via `target_sources`.

- [ ] **Step 2: Run to verify failure** → missing headers.

- [ ] **Step 3: Implement the effects**

`src/effects/Ticker.hpp`:
```cpp
#pragma once
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace itsme::effects {

class Ticker {
 public:
  Ticker(std::chrono::milliseconds interval, std::function<void()> onTick);
  ~Ticker();
  Ticker(const Ticker&) = delete;
  Ticker& operator=(const Ticker&) = delete;
  void setInterval(std::chrono::milliseconds interval);

 private:
  void loop();
  std::function<void()> onTick_;
  std::chrono::milliseconds interval_;
  bool stop_ = false;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::thread thread_;
};

}  // namespace itsme::effects
```

`src/effects/Ticker.cpp`:
```cpp
#include "effects/Ticker.hpp"

namespace itsme::effects {

Ticker::Ticker(std::chrono::milliseconds interval, std::function<void()> onTick)
    : onTick_(std::move(onTick)), interval_(interval), thread_([this] { loop(); }) {}

Ticker::~Ticker() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stop_ = true;
  }
  cv_.notify_all();
  if (thread_.joinable()) thread_.join();
}

void Ticker::setInterval(std::chrono::milliseconds interval) {
  std::lock_guard<std::mutex> lock(mutex_);
  interval_ = interval;
}

void Ticker::loop() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (!stop_) {
    const auto wait = interval_;
    if (cv_.wait_for(lock, wait, [this] { return stop_; })) break;
    lock.unlock();
    onTick_();
    lock.lock();
  }
}

}  // namespace itsme::effects
```

`src/effects/Boot.hpp`:
```cpp
#pragma once
#include <string>
#include <vector>

namespace itsme::effects {

class BootSequence {
 public:
  static constexpr int kDurationMs = 1100;
  void advance(int ms) { elapsed_ += ms; }
  bool done() const { return elapsed_ >= kDurationMs; }
  std::vector<std::string> visibleLines() const;

 private:
  int elapsed_ = 0;
};

}  // namespace itsme::effects
```

`src/effects/Boot.cpp`:
```cpp
#include "effects/Boot.hpp"
#include <array>
#include <utility>

namespace itsme::effects {

namespace {
constexpr std::array<std::pair<int, const char*>, 4> kLines = {{
    {0, "[  OK  ] Booting portfolio OS..."},
    {300, "[  OK  ] Loading kernel modules: devops cloud ai"},
    {600, "[  OK  ] Mounting ~/portfolio"},
    {900, "[  OK  ] Starting Portfolio-CLI shell"},
}};
}  // namespace

std::vector<std::string> BootSequence::visibleLines() const {
  std::vector<std::string> out;
  for (const auto& [at, line] : kLines)
    if (elapsed_ >= at) out.emplace_back(line);
  return out;
}

}  // namespace itsme::effects
```

`src/effects/Typewriter.hpp`:
```cpp
#pragma once

namespace itsme::effects {

class Typewriter {
 public:
  explicit Typewriter(int totalChars, int msPerChar = 15) : total_(totalChars), msPerChar_(msPerChar) {}
  void advance(int ms);
  int revealed() const { return revealed_; }
  bool done() const { return revealed_ >= total_; }
  void finish() { revealed_ = total_; }

 private:
  int total_;
  int msPerChar_;
  int revealed_ = 0;
  int carry_ = 0;  // leftover ms below one character
};

}  // namespace itsme::effects
```

`src/effects/Typewriter.cpp`:
```cpp
#include "effects/Typewriter.hpp"

namespace itsme::effects {

void Typewriter::advance(int ms) {
  if (done()) return;
  carry_ += ms;
  const int chars = msPerChar_ > 0 ? carry_ / msPerChar_ : total_;
  carry_ -= chars * msPerChar_;
  revealed_ += chars;
  if (revealed_ > total_) revealed_ = total_;
}

}  // namespace itsme::effects
```

Add the three `.cpp` files to `itsme_ui` in `CMakeLists.txt`.

- [ ] **Step 4: Integrate into the App**

`src/app/App.hpp` — add includes `<chrono>`, `<memory>`, `<optional>`, `"effects/Boot.hpp"`, `"effects/Ticker.hpp"`, `"effects/Typewriter.hpp"` and these protected members/methods:
```cpp
  virtual void onTick(int elapsedMs);
  bool animating() const;
  void updateTickerRate();
  std::unique_ptr<effects::Ticker> ticker_;
  std::optional<effects::BootSequence> boot_;
  std::optional<effects::Typewriter> typewriter_;  // drives the seeded welcome block (id 0)
  std::chrono::steady_clock::time_point lastTick_ = std::chrono::steady_clock::now();
```

`src/app/App.cpp` changes:

Add to the anonymous namespace:
```cpp
const Event kTick = Event::Special("itsme-tick");
constexpr auto kFastTick = std::chrono::milliseconds(50);
constexpr auto kIdleTick = std::chrono::milliseconds(1000);
```

Constructor body becomes:
```cpp
  outputs::setTrueColor(!opts_.noColor);
  if (!opts_.noBoot) {
    boot_.emplace();
    typewriter_.emplace(outputs::welcomeTypewriterLength());
    runtime_[0].typewriterRevealed = 0;
  }
```

`run()` creates the ticker before the loop and destroys it after:
```cpp
int App::run() {
  auto screen = ScreenInteractive::Fullscreen();
  screen_ = &screen;
  { std::lock_guard<std::mutex> lock(redraw_->mutex); redraw_->screen = &screen; }
  auto redraw = redraw_;
  ticker_ = std::make_unique<effects::Ticker>(animating() ? kFastTick : kIdleTick, [redraw] {
    std::lock_guard<std::mutex> lock(redraw->mutex);
    if (redraw->screen) redraw->screen->PostEvent(kTick);
  });
  lastTick_ = std::chrono::steady_clock::now();
  screen.Loop(component());
  ticker_.reset();
  { std::lock_guard<std::mutex> lock(redraw_->mutex); redraw_->screen = nullptr; }
  screen_ = nullptr;
  return 0;
}
```

New methods:
```cpp
bool App::animating() const {
  if (boot_ && !boot_->done()) return true;
  if (typewriter_ && !typewriter_->done()) return true;
  for (const auto& [id, rt] : runtime_)
    if (rt.githubFetch && !rt.githubFetch->ready()) return true;
  return false;
}

void App::updateTickerRate() {
  if (ticker_) ticker_->setInterval(animating() ? kFastTick : kIdleTick);
}

void App::onTick(int elapsedMs) {
  if (boot_ && !boot_->done()) {
    boot_->advance(elapsedMs);
    return;
  }
  if (typewriter_ && !typewriter_->done()) {
    typewriter_->advance(elapsedMs);
    runtime_[0].typewriterRevealed = typewriter_->done() ? -1 : typewriter_->revealed();
  }
  for (auto& [id, rt] : runtime_)
    if (rt.githubFetch && !rt.githubFetch->ready()) rt.spinnerFrame = (rt.spinnerFrame + 1) % 10;
}
```

At the top of `onEvent`, before the `Return` check:
```cpp
  if (e == kTick) {
    const auto now = std::chrono::steady_clock::now();
    const int elapsed = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTick_).count());
    lastTick_ = now;
    onTick(elapsed);
    updateTickerRate();
    return true;
  }
  if (boot_ && !boot_->done()) return true;  // swallow input during boot
  if (typewriter_ && !typewriter_->done() && !e.is_mouse() && e != Event::Custom) {
    typewriter_->finish();
    runtime_[0].typewriterRevealed = -1;
  }
```
Also call `updateTickerRate();` at the end of `submit()` (a `github` fetch starts the spinner).

At the top of `render()`:
```cpp
  if (boot_ && !boot_->done()) {
    Elements lines;
    for (const auto& l : boot_->visibleLines()) lines.push_back(t(l, Tone::Green));
    return vbox(std::move(lines)) | bgcolor(outputs::bgColor()) | flex;
  }
```

- [ ] **Step 5: Build, test, run**

Run: `cmake --build --preset default && ctest --preset default` → all pass.
Run `./build/default/itsme`: four boot lines appear over ~1 s, then the banner with the welcome text typing itself; any key finishes the typing; the clock in the title bar changes each minute; `github` shows a spinning braille spinner until data arrives. `./build/default/itsme --no-boot` starts instantly.

- [ ] **Step 6: Commit**

```bash
git add -A
git commit -m "feat(effects): ticker-driven boot sequence, typewriter welcome and spinner"
```

---

### Task 13: Matrix rain and hack overlays

**Files:**
- Create: `src/effects/Matrix.hpp`, `src/effects/Matrix.cpp`, `src/effects/Hack.hpp`, `src/effects/Hack.cpp`
- Modify: `src/app/App.hpp`, `src/app/App.cpp`, `CMakeLists.txt`, `tests/CMakeLists.txt`
- Test: `tests/effects/overlays_test.cpp`

**Interfaces:**
- Produces (namespace `itsme::effects`):
  - `class MatrixRain { explicit MatrixRain(unsigned seed = 42); void resize(int w, int h); void advance(int ms); ftxui::Element render() const; int width() const; int height() const; std::string currentQuote() const; }` — `advance` steps the rain every 50 ms of accumulated time and cycles quotes every 4200 ms.
  - `class HackSequence { void advance(int ms); int elapsed() const; bool finished() const; ftxui::Element render(int width) const; static constexpr int kAutoExitMs = 14000; }`
- App: `enum class Overlay { None, Matrix, Hack }`; `performAction` starts an overlay for `Action::Matrix` / `Action::Hack`; any key (not tick/custom/mouse) closes it; `render()` returns the overlay while active; `animating()` is true while an overlay is open.

- [ ] **Step 1: Write the failing tests**

`tests/effects/overlays_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include "../RenderHelper.hpp"
#include "effects/Hack.hpp"
#include "effects/Matrix.hpp"

using namespace itsme::effects;

TEST_CASE("matrix rain fills the grid and cycles quotes") {
  MatrixRain rain(7);
  rain.resize(40, 12);
  CHECK(rain.width() == 40);
  CHECK(rain.height() == 12);
  for (int i = 0; i < 40; ++i) rain.advance(50);
  auto s = renderPlain(rain.render(), 40, 12);
  CHECK(s.find("PRESS ESC OR ANY KEY TO EXIT") != std::string::npos);
  CHECK(rain.currentQuote() == "Wake up, Neo...");
  rain.advance(4200);
  CHECK(rain.currentQuote() == "The Matrix has you.");
  // some glyph other than space/hint must be drawn
  auto glyphs = renderPlain(rain.render(), 40, 12);
  bool anyGlyph = false;
  for (const char* g : {"ｱ", "0", "A", "#", "ﾈ"}) if (glyphs.find(g) != std::string::npos) anyGlyph = true;
  CHECK(anyGlyph);
}

TEST_CASE("hack sequence timeline") {
  HackSequence hack;
  auto s0 = renderPlain(hack.render(100), 100, 40);
  CHECK(s0.find("Establishing encrypted tunnel") != std::string::npos);
  CHECK(s0.find("Resolved IP") == std::string::npos);
  hack.advance(600);
  auto s1 = renderPlain(hack.render(100), 100, 40);
  CHECK(s1.find("Resolved IP: 76.76.21.21") != std::string::npos);
  CHECK(s1.find("ACCESS GRANTED") == std::string::npos);
  hack.advance(3000);
  auto s2 = renderPlain(hack.render(100), 100, 40);
  CHECK(s2.find("BYPASSING CLOUDFLARE WAF") != std::string::npos);
  hack.advance(6000);  // 9600 ms total
  auto s3 = renderPlain(hack.render(100), 100, 40);
  CHECK(s3.find("ACCESS GRANTED") != std::string::npos);
  CHECK_FALSE(hack.finished());
  hack.advance(5000);
  CHECK(hack.finished());
  auto s4 = renderPlain(hack.render(100), 100, 40);
  CHECK(s4.find("nice try") != std::string::npos);
  CHECK(s4.find("press ESC or any key to exit") != std::string::npos);
}
```

`tests/CMakeLists.txt`: add via `target_sources`.

- [ ] **Step 2: Run to verify failure** → missing headers.

- [ ] **Step 3: Implement the effects**

`src/effects/Matrix.hpp`:
```cpp
#pragma once
#include <random>
#include <string>
#include <vector>
#include <ftxui/dom/elements.hpp>

namespace itsme::effects {

class MatrixRain {
 public:
  explicit MatrixRain(unsigned seed = 42);
  void resize(int w, int h);
  void advance(int ms);
  ftxui::Element render() const;
  int width() const { return w_; }
  int height() const { return h_; }
  std::string currentQuote() const;

 private:
  struct Column {
    float head = 0;
    float speed = 1;
    int length = 8;
  };
  void step();
  void resetColumn(Column& c, bool aboveScreen);
  const std::string& randomGlyph();

  std::mt19937 rng_;
  int w_ = 0, h_ = 0;
  std::vector<Column> cols_;
  std::vector<std::string> cells_;  // w_*h_ glyphs
  int stepCarry_ = 0;
  int quoteElapsed_ = 0;
  std::size_t quoteIndex_ = 0;
};

}  // namespace itsme::effects
```

`src/effects/Matrix.cpp`:
```cpp
#include "effects/Matrix.hpp"
#include <array>
#include "core/Strings.hpp"
#include "outputs/Theme.hpp"

namespace itsme::effects {
using namespace ftxui;

namespace {
const std::vector<std::string>& glyphs() {
  static const std::vector<std::string> g = itsme::core::utf8Chars(
      "ｦｧｨｩｪｫｬｭｮｯｰｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜﾝ0123456789ABCDEF!@#$%^&*");
  return g;
}

constexpr std::array<const char*, 8> kQuotes = {
    "Wake up, Neo...", "The Matrix has you.", "Follow the white rabbit.", "Knock, knock, Neo.",
    "There is no spoon.", "Free your mind.", "01001000 01100101 01101100 01101100 01101111",
    "// TODO: escape the simulation",
};
constexpr int kStepMs = 50;
constexpr int kQuoteCycleMs = 4200;
constexpr int kQuoteVisibleMs = 3800;
}  // namespace

MatrixRain::MatrixRain(unsigned seed) : rng_(seed) {}

const std::string& MatrixRain::randomGlyph() {
  std::uniform_int_distribution<std::size_t> d(0, glyphs().size() - 1);
  return glyphs()[d(rng_)];
}

void MatrixRain::resetColumn(Column& c, bool aboveScreen) {
  std::uniform_real_distribution<float> speed(0.4f, 1.4f);
  std::uniform_int_distribution<int> length(4, 14);
  std::uniform_real_distribution<float> start(0.0f, static_cast<float>(h_ > 0 ? h_ : 1));
  c.speed = speed(rng_);
  c.length = length(rng_);
  c.head = aboveScreen ? -start(rng_) : start(rng_) - static_cast<float>(h_);
}

void MatrixRain::resize(int w, int h) {
  w_ = w < 1 ? 1 : w;
  h_ = h < 1 ? 1 : h;
  cols_.assign(static_cast<std::size_t>(w_), Column{});
  for (auto& c : cols_) resetColumn(c, true);
  cells_.assign(static_cast<std::size_t>(w_ * h_), " ");
  for (auto& cell : cells_) cell = randomGlyph();
}

void MatrixRain::step() {
  std::uniform_real_distribution<float> chance(0.0f, 1.0f);
  for (int x = 0; x < w_; ++x) {
    Column& c = cols_[static_cast<std::size_t>(x)];
    c.head += c.speed;
    const int headRow = static_cast<int>(c.head);
    if (headRow >= 0 && headRow < h_) cells_[static_cast<std::size_t>(headRow * w_ + x)] = randomGlyph();
    if (c.head - static_cast<float>(c.length) > static_cast<float>(h_) && chance(rng_) > 0.975f) resetColumn(c, true);
  }
}

void MatrixRain::advance(int ms) {
  stepCarry_ += ms;
  while (stepCarry_ >= kStepMs) {
    stepCarry_ -= kStepMs;
    step();
  }
  quoteElapsed_ += ms;
  while (quoteElapsed_ >= kQuoteCycleMs) {
    quoteElapsed_ -= kQuoteCycleMs;
    quoteIndex_ = (quoteIndex_ + 1) % kQuotes.size();
  }
}

std::string MatrixRain::currentQuote() const { return kQuotes[quoteIndex_]; }

Element MatrixRain::render() const {
  const Color head = Color::White;
  const Color bright = outputs::matrixGreen();
  const Color mid = outputs::trueColor() ? Color::RGB(0x00, 0xaa, 0x28) : Color::Green;
  const Color dimC = outputs::trueColor() ? Color::RGB(0x00, 0x5a, 0x15) : Color::GreenLight;

  Elements rows;
  for (int y = 0; y < h_; ++y) {
    Elements runs;
    std::string run;
    int runClass = -1;
    auto flush = [&] {
      if (run.empty()) return;
      Element e = text(run);
      if (runClass == 1) e = e | color(head) | bold;
      else if (runClass == 2) e = e | color(bright);
      else if (runClass == 3) e = e | color(mid);
      else if (runClass == 4) e = e | color(dimC);
      runs.push_back(e);
      run.clear();
    };
    for (int x = 0; x < w_; ++x) {
      const Column& c = cols_[static_cast<std::size_t>(x)];
      const float dist = c.head - static_cast<float>(y);
      int cls = 0;
      if (dist >= 0 && dist < 1) cls = 1;
      else if (dist >= 1 && dist < static_cast<float>(c.length)) {
        const float f = dist / static_cast<float>(c.length);
        cls = f < 0.35f ? 2 : f < 0.7f ? 3 : 4;
      }
      if (cls != runClass) { flush(); runClass = cls; }
      run += cls == 0 ? std::string(" ") : cells_[static_cast<std::size_t>(y * w_ + x)];
    }
    flush();
    rows.push_back(hbox(std::move(runs)));
  }

  Element rain = vbox(std::move(rows));
  Elements layers = {rain};
  if (quoteElapsed_ < kQuoteVisibleMs)
    layers.push_back(vbox({filler(), text(" " + currentQuote() + " ") | bold | color(bright) | bgcolor(Color::Black) | center, filler()}));
  layers.push_back(vbox({filler(), text("[ PRESS ESC OR ANY KEY TO EXIT ]") | color(dimC) | center}));
  return dbox(std::move(layers)) | bgcolor(Color::Black);
}

}  // namespace itsme::effects
```

`src/effects/Hack.hpp`:
```cpp
#pragma once
#include <ftxui/dom/elements.hpp>

namespace itsme::effects {

class HackSequence {
 public:
  static constexpr int kAutoExitMs = 14000;
  void advance(int ms) { elapsed_ += ms; }
  int elapsed() const { return elapsed_; }
  bool finished() const { return elapsed_ >= kAutoExitMs; }
  ftxui::Element render(int width) const;

 private:
  int elapsed_ = 0;
};

}  // namespace itsme::effects
```

`src/effects/Hack.cpp`:
```cpp
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
  bool boldCenter;
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
    rows.push_back(hbox({gauge(progress) | color(tone(b.tone)) | flex, text(" " + std::to_string(pct) + "%") | color(tone(Tone::Fg))}));
  }

  if (elapsed_ >= kSep2At) rows.push_back(text(std::string(60, '-')) | color(sepColor));
  if (elapsed_ >= kGrantedAt) {
    const bool flash = elapsed_ >= kGrantedAt + 300 && elapsed_ < kGrantedAt + 650 && ((elapsed_ / 70) % 2 == 0);
    rows.push_back(text("★★★  ACCESS GRANTED  ★★★") | bold | color(flash ? tone(Tone::Red) : green) | center);
  }
  for (const auto& l : kOutro) {
    if (elapsed_ < l.at) continue;
    Element e = text(l.text) | color(tone(l.tone));
    if (l.boldCenter) e = e | dim | center;
    rows.push_back(e);
  }

  Element box = vbox(std::move(rows)) | size(WIDTH, EQUAL, boxWidth);
  return vbox({filler(), hbox({filler(), box, filler()}), filler()}) | bgcolor(Color::Black);
}

}  // namespace itsme::effects
```

Add `src/effects/Matrix.cpp` and `src/effects/Hack.cpp` to `itsme_ui`.

- [ ] **Step 4: Integrate into the App**

`src/app/App.hpp` — add includes `"effects/Hack.hpp"`, `"effects/Matrix.hpp"` and protected members:
```cpp
  enum class Overlay { None, Matrix, Hack };
  void closeOverlay();
  Overlay overlay_ = Overlay::None;
  std::optional<effects::MatrixRain> matrix_;
  std::optional<effects::HackSequence> hack_;
```

`src/app/App.cpp`:

`performAction` becomes:
```cpp
void App::performAction(core::Action action, int /*blockId*/) {
  const int w = screen_ ? screen_->dimx() : width_;
  const int h = screen_ ? screen_->dimy() : 24;
  if (action == core::Action::Matrix) {
    matrix_.emplace(static_cast<unsigned>(rng_()));
    matrix_->resize(w, h);
    overlay_ = Overlay::Matrix;
  } else if (action == core::Action::Hack) {
    hack_.emplace();
    overlay_ = Overlay::Hack;
  }
}

void App::closeOverlay() {
  overlay_ = Overlay::None;
  matrix_.reset();
  hack_.reset();
}
```

In `animating()` add as the first line: `if (overlay_ != Overlay::None) return true;`

In `onTick(int elapsedMs)` add at the top:
```cpp
  if (overlay_ == Overlay::Matrix && matrix_) {
    const int w = screen_ ? screen_->dimx() : width_;
    const int h = screen_ ? screen_->dimy() : 24;
    if (w != matrix_->width() || h != matrix_->height()) matrix_->resize(w, h);
    matrix_->advance(elapsedMs);
    return;
  }
  if (overlay_ == Overlay::Hack && hack_) {
    hack_->advance(elapsedMs);
    if (hack_->finished()) closeOverlay();
    return;
  }
```

In `onEvent`, right after the `kTick` block:
```cpp
  if (overlay_ != Overlay::None) {
    if (e == Event::Custom || e.is_mouse()) return true;
    closeOverlay();
    updateTickerRate();
    return true;
  }
```

In `render()`, right after the boot branch:
```cpp
  if (overlay_ == Overlay::Matrix && matrix_) return matrix_->render();
  if (overlay_ == Overlay::Hack && hack_) return hack_->render(width_);
```

- [ ] **Step 5: Build, test, run**

Run: `cmake --build --preset default && ctest --preset default` → all pass.
Run `./build/default/itsme`: `matrix` fills the screen with falling glyphs and cycling quotes; any key returns to the shell with the echo block intact. `hack` plays the scripted sequence with four progress bars and exits by itself after ~14 s or on a key.

- [ ] **Step 6: Commit**

```bash
git add -A
git commit -m "feat(effects): matrix rain and hack sequence overlays"
```

---

### Task 14: Resume opener, README, CI and release workflow

**Files:**
- Create: `src/app/Opener.hpp`, `src/app/Opener.cpp`, `.github/workflows/ci.yml`, `.clang-format`
- Modify: `src/app/App.cpp` (`performAction` for `OpenResume`), `README.md` (full), `CMakeLists.txt`, `tests/CMakeLists.txt`
- Test: `tests/app/opener_test.cpp`

**Interfaces:**
- Produces (namespace `itsme::app`): `bool isSshSession()` (true when `SSH_CONNECTION` or `SSH_TTY` is set), `std::string openCommand(const std::string& url)` (platform command line, empty when unsupported), `bool openUrl(const std::string& url)` (false in SSH sessions or when the launcher fails).

- [ ] **Step 1: Write the failing test**

`tests/app/opener_test.cpp`:
```cpp
#include <catch2/catch_test_macros.hpp>
#include "app/Opener.hpp"

using namespace itsme::app;

TEST_CASE("openCommand builds a platform launcher") {
  const std::string cmd = openCommand("https://itsme.dfanso.dev/resume.pdf");
  CHECK(cmd.find("https://itsme.dfanso.dev/resume.pdf") != std::string::npos);
#ifdef _WIN32
  CHECK(cmd.rfind("start \"\" ", 0) == 0);
#elif __APPLE__
  CHECK(cmd.rfind("open ", 0) == 0);
#else
  CHECK(cmd.rfind("xdg-open ", 0) == 0);
#endif
}

TEST_CASE("openCommand refuses unsafe URLs") {
  CHECK(openCommand("https://x.y/a\"; rm -rf /").empty());
  CHECK(openCommand("ftp://x.y").empty());
  CHECK(openCommand("").empty());
}
```

`tests/CMakeLists.txt`: add via `target_sources`.

- [ ] **Step 2: Run to verify failure** → missing header.

- [ ] **Step 3: Implement**

`src/app/Opener.hpp`:
```cpp
#pragma once
#include <string>

namespace itsme::app {
bool isSshSession();
std::string openCommand(const std::string& url);  // "" when the URL is not a plain http(s) URL
bool openUrl(const std::string& url);
}  // namespace itsme::app
```

`src/app/Opener.cpp`:
```cpp
#include "app/Opener.hpp"
#include <cctype>
#include <cstdlib>

namespace itsme::app {

bool isSshSession() { return std::getenv("SSH_CONNECTION") != nullptr || std::getenv("SSH_TTY") != nullptr; }

std::string openCommand(const std::string& url) {
  const bool https = url.rfind("https://", 0) == 0;
  const bool http = url.rfind("http://", 0) == 0;
  if (!https && !http) return "";
  for (unsigned char c : url) {
    const bool ok = std::isalnum(c) || c == ':' || c == '/' || c == '.' || c == '-' || c == '_' || c == '?' ||
                    c == '=' || c == '&' || c == '%' || c == '#' || c == '~' || c == '+';
    if (!ok) return "";
  }
#ifdef _WIN32
  return "start \"\" " + url;
#elif __APPLE__
  return "open " + url + " >/dev/null 2>&1";
#else
  return "xdg-open " + url + " >/dev/null 2>&1";
#endif
}

bool openUrl(const std::string& url) {
  if (isSshSession()) return false;
  const std::string cmd = openCommand(url);
  if (cmd.empty()) return false;
  return std::system(cmd.c_str()) == 0;
}

}  // namespace itsme::app
```

`src/app/App.cpp` — add `#include "app/Opener.hpp"` and, in `performAction`, before the Matrix branch:
```cpp
  if (action == core::Action::OpenResume) {
    auto& lines = state_.blocks.back().execution.text->lines;
    const std::string& url = data::profile().resumeUrl;
    lines.push_back(url);
    if (isSshSession()) lines.push_back("(open the link above in your browser)");
    else if (!openUrl(url)) lines.push_back("(could not launch a browser; open the link above manually)");
    return;
  }
```

`.clang-format`:
```yaml
BasedOnStyle: Google
ColumnLimit: 120
IndentWidth: 2
DerivePointerAlignment: false
PointerAlignment: Left
AllowShortFunctionsOnASingleLine: Inline
AllowShortIfStatementsOnASingleLine: WithoutElse
```

`.github/workflows/ci.yml`:
```yaml
name: CI
on:
  push:
    branches: [main]
    tags: ['v*']
  pull_request:

jobs:
  build:
    name: ${{ matrix.name }}
    runs-on: ${{ matrix.os }}
    strategy:
      fail-fast: false
      matrix:
        include:
          - { name: ubuntu-gcc,   os: ubuntu-latest,  preset: ci,   cc: gcc,   cxx: g++ }
          - { name: ubuntu-clang, os: ubuntu-latest,  preset: ci,   cc: clang, cxx: clang++ }
          - { name: macos,        os: macos-latest,   preset: ci,   cc: clang, cxx: clang++ }
          - { name: windows-msvc, os: windows-latest, preset: msvc }
    steps:
      - uses: actions/checkout@v4
      - name: Install deps (Ubuntu)
        if: runner.os == 'Linux'
        run: sudo apt-get update && sudo apt-get install -y ninja-build libcurl4-openssl-dev
      - name: Install deps (macOS)
        if: runner.os == 'macOS'
        run: brew install ninja
      - name: Configure
        env: { CC: '${{ matrix.cc }}', CXX: '${{ matrix.cxx }}' }
        run: cmake --preset ${{ matrix.preset }}
      - name: Build
        run: cmake --build --preset ${{ matrix.preset }}
      - name: Test
        run: ctest --preset ${{ matrix.preset }}

  release:
    if: startsWith(github.ref, 'refs/tags/v')
    needs: build
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        include:
          - { os: ubuntu-latest,  preset: release, artifact: itsme-linux-x64,   bin: build/release/itsme }
          - { os: macos-latest,   preset: release, artifact: itsme-macos-arm64, bin: build/release/itsme }
          - { os: windows-latest, preset: msvc,    artifact: itsme-windows-x64, bin: build/msvc/Release/itsme.exe }
    steps:
      - uses: actions/checkout@v4
      - if: runner.os == 'Linux'
        run: sudo apt-get update && sudo apt-get install -y ninja-build libcurl4-openssl-dev
      - if: runner.os == 'macOS'
        run: brew install ninja
      - run: cmake --preset ${{ matrix.preset }} -DITSME_BUILD_TESTS=OFF
      - run: cmake --build --preset ${{ matrix.preset }}
      - run: cp "${{ matrix.bin }}" "${{ matrix.artifact }}${{ runner.os == 'Windows' && '.exe' || '' }}"
        shell: bash
      - uses: softprops/action-gh-release@v2
        with:
          files: ${{ matrix.artifact }}*
```

`README.md`:
```markdown
# itsme.dfanso.dev — TUI edition

A native terminal version of [itsme.dfanso.dev](https://itsme.dfanso.dev), Leo Felcianas' terminal-style
portfolio, written in C++17 with [FTXUI](https://github.com/ArthurSonzogni/FTXUI).

Same commands, same Tokyo Night colours, same easter eggs — in your own terminal.

## Run

Download a binary from the Releases page, or build from source (below), then:

```
itsme                # boot animation, then the shell
itsme --no-boot      # skip boot + typewriter
itsme --no-color     # 16-colour palette (also honours NO_COLOR)
GITHUB_TOKEN=ghp_... itsme   # unlocks commits/PRs/issues + contribution heatmap in `github`
```

Type `help` to list commands. Tab completes, ↑/↓ browse history, Ctrl+L clears, Ctrl+C / Ctrl+D exit.
Use a Unicode-capable terminal (Windows Terminal, not legacy conhost).

## Build

Requires CMake ≥ 3.20, a C++17 compiler and Ninja. Dependencies (FTXUI, nlohmann/json, Catch2, and libcurl
when it is not installed) are fetched automatically.

```
cmake --preset default
cmake --build --preset default
ctest --preset default
./build/default/itsme
```

Presets: `default` (Ninja, RelWithDebInfo), `release`, `ci` (warnings as errors), `msvc` (Visual Studio 2022).

## SSH mode

The binary reads stdin and writes ANSI to stdout, so it works unchanged as an SSH forced command:

```
# /etc/ssh/sshd_config.d/itsme.conf
Match User guest
    ForceCommand /usr/local/bin/itsme --no-boot
    PermitTTY yes
    PasswordAuthentication yes
    PermitEmptyPasswords yes
    AllowTcpForwarding no
    X11Forwarding no
```

In an SSH session the `resume` command prints the PDF URL instead of launching a browser.

## Layout

- `src/core` — command registry, dispatcher, state (pure, tested)
- `src/data` — portfolio content
- `src/outputs` — FTXUI renderers per command
- `src/github` — GitHub GraphQL/REST client and parsers
- `src/effects` — boot, typewriter, matrix, hack
- `src/app` — the interactive shell

## License

MIT — see `LICENSE`.
```

Add `src/app/Opener.cpp` to `itsme_ui`.

- [ ] **Step 4: Build, test, run**

Run: `cmake --build --preset default && ctest --preset default` → all pass.
Run `./build/default/itsme`, type `resume` → "Opening resume..." plus the URL; the PDF opens in the browser. `SSH_TTY=1 ./build/default/itsme` → the URL with the "(open the link above…)" note and no browser launch.

- [ ] **Step 5: Commit and push; verify CI**

```bash
git add -A
git commit -m "feat: resume opener, README, CI matrix and release workflow"
git push
gh run watch --exit-status
```
Expected: all four CI jobs green.

---

## Self-Review

**Spec coverage**

| Spec section | Task |
|---|---|
| §3 layout, dependency rule | 1, 2, 7, 8, 11 (`itsme_core` / `itsme_ui` / `itsme_net` split) |
| §4 registry + `executeLine` | 2, 4 |
| §5 terminal state, history | 5 |
| §6 UI: title bar, output pane, input line, keys, prompt variants, wrap/narrow | 9 (keys, prompt, scroll), 7 (narrow banner), 11 (heatmap hidden < 80 cols) |
| §7 outputs incl. `resume` | 7, 8, 14 |
| §8 GitHub client, async, rendering, fallback, timeout | 10, 11 |
| §9 effects: boot, typewriter, matrix, hack, `--no-boot` | 12, 13 |
| §10 error handling: unknown cmd, network failure, no token, narrow, Ctrl+C restore, non-TTY, curl fallback build | 4, 11, 9 (TTY check, FTXUI teardown), 11 (Dependencies.cmake) |
| §11 tests: unit, smoke, CI warnings-as-errors | every task; `ci` preset in 1 |
| §12 build, CI, release, README SSH recipe | 1, 14 |

Gaps accepted, per spec non-goals: no CRT overlay, no profile image, no PDF generation.

**Placeholder scan** — no TBD/TODO; every code step is complete. Task 1's `main.cpp` and Task 9's `github` fallback line are real interim implementations replaced by later tasks, called out at the point of replacement.

**Type consistency** — checked: `core::Tone` / `CommandKind` / `Action` / `ExecKind` (Task 2) used unchanged in 4–14; `BlockRuntime` fields (`timeString`, `typewriterRevealed`, `spinnerFrame`, `githubFetch`, `projectsFetch`) match between Task 11's definition and Tasks 12/13 usage; `App` constructor is `App(Options, std::shared_ptr<const github::Client>)` from Task 11 onward and the Task 9 smoke test is updated in Task 11; `outputs::RenderContext{width, hour}` is aggregate-initialised in that order everywhere; `github::ProjectStatsMap` is defined in Task 8 and consumed in 8, 11; `effects::Ticker/BootSequence/Typewriter/MatrixRain/HackSequence` signatures match their App usage.
