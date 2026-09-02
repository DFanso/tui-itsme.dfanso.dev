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
