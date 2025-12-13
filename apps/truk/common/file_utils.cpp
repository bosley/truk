#include "file_utils.hpp"
#include <filesystem>
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

std::string get_directory(const std::string &file_path) {
  std::filesystem::path p(file_path);
  if (p.has_parent_path()) {
    return p.parent_path().string();
  }
  return ".";
}

std::string resolve_path(const std::string &import_path,
                         const std::string &current_file_path) {
  std::filesystem::path current_dir = get_directory(current_file_path);
  std::filesystem::path resolved = current_dir / import_path;
  return resolved.string();
}

std::string canonicalize_path(const std::string &path) {
  try {
    std::filesystem::path p(path);
    if (std::filesystem::exists(p)) {
      return std::filesystem::canonical(p).string();
    }
    return std::filesystem::weakly_canonical(p).string();
  } catch (...) {
    return path;
  }
}

} // namespace truk::common
