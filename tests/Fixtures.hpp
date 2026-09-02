#pragma once
#include <fstream>
#include <sstream>
#include <string>

inline std::string readFixture(const char* name) {
  std::ifstream in(std::string(ITSME_FIXTURES_DIR) + "/" + name, std::ios::binary);
  std::stringstream ss;
  ss << in.rdbuf();
  return ss.str();
}
