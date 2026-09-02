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
