#pragma once
#include <string>

namespace itsme::app {
bool isSshSession();
std::string openCommand(const std::string& url);  // "" when the URL is not a plain http(s) URL
bool openUrl(const std::string& url);
}  // namespace itsme::app
