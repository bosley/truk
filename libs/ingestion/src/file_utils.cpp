#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <truk/ingestion/file_utils.hpp>

namespace truk::ingestion {

std::string read_file(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open file: " + path);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

bool write_file(const std::string &path, const std::string &content) {
  std::ofstream out(path);
  if (!out.is_open()) {
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

std::string
resolve_path_with_search(const std::string &import_path,
                         const std::string &current_file_path,
                         const std::vector<std::string> &search_paths) {
  std::filesystem::path current_dir = get_directory(current_file_path);
  std::filesystem::path resolved = current_dir / import_path;

  if (std::filesystem::exists(resolved)) {
    return resolved.string();
  }

  for (const auto &search_path : search_paths) {
    std::filesystem::path search_resolved =
        std::filesystem::path(search_path) / import_path;
    if (std::filesystem::exists(search_resolved)) {
      return search_resolved.string();
    }
  }

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

} // namespace truk::ingestion
