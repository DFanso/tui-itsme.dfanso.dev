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
