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
