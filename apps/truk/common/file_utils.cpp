#include "file_utils.hpp"
#include <fmt/core.h>
#include <fstream>
#include <sstream>

namespace truk::common {

std::string read_file(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    fmt::print(stderr, "Error: Could not open file '{}'\n", path);
    std::exit(1);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

bool write_file(const std::string &path, const std::string &content) {
  std::ofstream out(path);
  if (!out.is_open()) {
    fmt::print(stderr, "Error: Could not open output file '{}'\n", path);
    return false;
  }

  out << content;
  return true;
}

} // namespace truk::common
