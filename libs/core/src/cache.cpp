#include <truk/core/cache.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace truk::core {

namespace fs = std::filesystem;

cache_manager_c::cache_manager_c(const fs::path &project_root)
    : _project_root(project_root), _cache_root(project_root / ".cache") {}

cache_entry_s
cache_manager_c::get_library_cache_paths(const std::string &lib_name) const {
  cache_entry_s entry;
  fs::path lib_dir = _cache_root / "libraries" / lib_name;

  entry.c_file = lib_dir / (lib_name + ".c");
  entry.o_file = lib_dir / (lib_name + ".o");
  entry.a_file = lib_dir / (lib_name + ".a");
  entry.metadata_file = lib_dir / ".build_info.json";

  return entry;
}

cache_entry_s cache_manager_c::get_application_cache_paths(
    const std::string &app_name) const {
  cache_entry_s entry;
  fs::path app_dir = _cache_root / "applications" / app_name;

  entry.c_file = app_dir / (app_name + ".c");
  entry.o_file = app_dir / (app_name + ".o");
  entry.a_file = app_dir / (app_name + ".a");
  entry.metadata_file = app_dir / ".build_info.json";

  return entry;
}

bool cache_manager_c::needs_rebuild(
    const std::string &lib_name,
    const std::vector<std::string> &source_files) const {
  auto entry = get_library_cache_paths(lib_name);

  if (!fs::exists(entry.a_file)) {
    return true;
  }

  if (!fs::exists(entry.metadata_file)) {
    return true;
  }

  auto metadata = load_metadata(entry.metadata_file);
  if (!metadata.has_value()) {
    return true;
  }

  auto artifact_mtime = get_file_mtime(entry.a_file);

  for (const auto &source_file : source_files) {
    fs::path source_path(source_file);
    if (!fs::exists(source_path)) {
      return true;
    }

    auto source_mtime = get_file_mtime(source_path);

    if (source_mtime > artifact_mtime) {
      return true;
    }

    auto it = metadata->source_mtimes.find(source_file);
    if (it == metadata->source_mtimes.end()) {
      return true;
    }

    if (source_mtime != it->second) {
      return true;
    }
  }

  if (source_files.size() != metadata->source_mtimes.size()) {
    return true;
  }

  return false;
}

void cache_manager_c::update_metadata(
    const std::string &lib_name, const std::vector<std::string> &source_files) {
  auto entry = get_library_cache_paths(lib_name);

  build_metadata_s metadata;
  metadata.artifact_mtime = get_file_mtime(entry.a_file);

  for (const auto &source_file : source_files) {
    fs::path source_path(source_file);
    if (fs::exists(source_path)) {
      metadata.source_mtimes[source_file] = get_file_mtime(source_path);
    }
  }

  save_metadata(entry.metadata_file, metadata);
}

void cache_manager_c::ensure_cache_directories() {
  fs::create_directories(_cache_root / "libraries");
  fs::create_directories(_cache_root / "applications");
}

std::optional<build_metadata_s>
cache_manager_c::load_metadata(const fs::path &metadata_file) const {
  if (!fs::exists(metadata_file)) {
    return std::nullopt;
  }

  std::ifstream file(metadata_file);
  if (!file.is_open()) {
    return std::nullopt;
  }

  build_metadata_s metadata;
  std::string line;

  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '{' || line[0] == '}') {
      continue;
    }

    size_t colon_pos = line.find(':');
    if (colon_pos == std::string::npos) {
      continue;
    }

    std::string key = line.substr(0, colon_pos);
    std::string value = line.substr(colon_pos + 1);

    key.erase(0, key.find_first_not_of(" \t\""));
    key.erase(key.find_last_not_of(" \t\",") + 1);

    value.erase(0, value.find_first_not_of(" \t\""));
    value.erase(value.find_last_not_of(" \t\",") + 1);

    if (key == "artifact_mtime") {
      try {
        auto duration = std::chrono::nanoseconds(std::stoull(value));
        metadata.artifact_mtime = fs::file_time_type(duration);
      } catch (...) {
      }
    } else {
      try {
        auto duration = std::chrono::nanoseconds(std::stoull(value));
        metadata.source_mtimes[key] = fs::file_time_type(duration);
      } catch (...) {
      }
    }
  }

  return metadata;
}

void cache_manager_c::save_metadata(const fs::path &metadata_file,
                                    const build_metadata_s &metadata) {
  fs::create_directories(metadata_file.parent_path());

  std::ofstream file(metadata_file);
  if (!file.is_open()) {
    return;
  }

  file << "{\n";
  file << "  \"artifact_mtime\": \""
       << static_cast<long long>(
              metadata.artifact_mtime.time_since_epoch().count())
       << "\",\n";

  bool first = true;
  for (const auto &[source, mtime] : metadata.source_mtimes) {
    if (!first) {
      file << ",\n";
    }
    first = false;
    file << "  \"" << source << "\": \""
         << static_cast<long long>(mtime.time_since_epoch().count()) << "\"";
  }

  if (!metadata.source_mtimes.empty()) {
    file << "\n";
  }

  file << "}\n";
}

fs::file_time_type cache_manager_c::get_file_mtime(const fs::path &file) const {
  if (!fs::exists(file)) {
    return fs::file_time_type::min();
  }
  return fs::last_write_time(file);
}

} // namespace truk::core
