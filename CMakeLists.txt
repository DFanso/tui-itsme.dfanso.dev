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

if(MINGW)
  # Avoid picking up mismatched libstdc++/libgcc/winpthread DLLs from PATH at runtime.
  add_link_options(-static -static-libgcc -static-libstdc++)
endif()

add_library(itsme_core STATIC
  src/core/Version.cpp
  src/core/Commands.cpp
  src/core/InputHelpers.cpp
  src/core/TerminalState.cpp
  src/core/LineEditor.cpp
  src/data/Portfolio.cpp
  src/github/Parse.cpp
  src/github/Client.cpp)
target_include_directories(itsme_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(itsme_core PUBLIC nlohmann_json::nlohmann_json)
target_compile_definitions(itsme_core PUBLIC ITSME_VERSION="${PROJECT_VERSION}")
itsme_set_warnings(itsme_core)

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
  src/outputs/Misc.cpp
  src/outputs/Help.cpp
  src/outputs/Ls.cpp
  src/outputs/Skills.cpp
  src/outputs/Experience.cpp
  src/outputs/Projects.cpp
  src/outputs/Neofetch.cpp
  src/app/Clock.cpp
  src/app/Prompt.cpp
  src/app/TitleBar.cpp
  src/app/BlockRenderer.cpp
  src/app/App.cpp
  src/outputs/GitHubStats.cpp
  src/effects/Ticker.cpp
  src/effects/Boot.cpp
  src/effects/Typewriter.cpp
  src/effects/Matrix.cpp
  src/effects/Hack.cpp)
target_link_libraries(itsme_ui PUBLIC itsme_core ftxui::component ftxui::dom ftxui::screen)
itsme_set_warnings(itsme_ui)

add_library(itsme_net STATIC src/github/CurlHttp.cpp)
target_link_libraries(itsme_net PUBLIC itsme_core CURL::libcurl)
itsme_set_warnings(itsme_net)

add_executable(itsme src/main.cpp)
target_link_libraries(itsme PRIVATE itsme_ui itsme_net)
itsme_set_warnings(itsme)

if(ITSME_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
