#include <cstdio>

#include "core/Version.hpp"

int main() {
  std::printf("itsme %s\n", itsme::version());
  return 0;
}
