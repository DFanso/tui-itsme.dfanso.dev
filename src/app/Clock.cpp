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
  char buf[16];
  std::snprintf(buf, sizeof buf, "%02d:%02d", t.hour, t.minute);
  return buf;
}

std::string clockHHMMSS(const LocalTime& t) {
  char buf[16];
  std::snprintf(buf, sizeof buf, "%02d:%02d:%02d", t.hour, t.minute, t.second);
  return buf;
}

}  // namespace itsme::app
