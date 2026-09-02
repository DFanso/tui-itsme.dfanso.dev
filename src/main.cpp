#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

#include "app/App.hpp"
#include "core/Version.hpp"
#include "github/Client.hpp"
#include "github/CurlHttp.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace {
// True when both stdin and stdout are attached to a real terminal. On Windows _isatty() also
// reports true for NUL and other character devices, so GetConsoleMode is used instead.
bool interactiveTerminal() {
#ifdef _WIN32
  DWORD mode = 0;
  return GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &mode) != 0 &&
         GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode) != 0;
#else
  return isatty(STDOUT_FILENO) && isatty(STDIN_FILENO);
#endif
}

void usage() {
  std::printf(
      "itsme %s - terminal portfolio of Leo Felcianas\n\n"
      "usage: itsme [--no-boot] [--no-color] [--version] [--help]\n"
      "  --no-boot   skip the boot and typewriter animations\n"
      "  --no-color  use the 16-color palette (also honours NO_COLOR)\n",
      itsme::version());
}
}  // namespace

int main(int argc, char** argv) {
  itsme::app::Options opts;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--no-boot") == 0) {
      opts.noBoot = true;
    } else if (std::strcmp(argv[i], "--no-color") == 0) {
      opts.noColor = true;
    } else if (std::strcmp(argv[i], "--version") == 0) {
      std::printf("%s\n", itsme::version());
      return 0;
    } else if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
      usage();
      return 0;
    } else {
      std::fprintf(stderr, "itsme: unknown option '%s'\n", argv[i]);
      usage();
      return 2;
    }
  }
  if (std::getenv("NO_COLOR") != nullptr) opts.noColor = true;

  if (!interactiveTerminal()) {
    std::fprintf(stderr, "itsme: needs an interactive terminal (stdin/stdout is not a TTY).\n");
    return 1;
  }
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif
  auto client = std::make_shared<const itsme::github::Client>(itsme::github::tokenFromEnv(),
                                                              itsme::github::curlHttp());
  itsme::app::App app(opts, client);
  return app.run();
}
