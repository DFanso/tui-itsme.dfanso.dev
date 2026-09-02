#include <catch2/catch_test_macros.hpp>
#include <string>

#include "core/Version.hpp"

TEST_CASE("version string matches the CMake project version") {
  REQUIRE(std::string(itsme::version()) == "0.1.0");
}
