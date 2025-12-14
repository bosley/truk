#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace truk::core {

struct cache_entry_s {
  std::filesystem::path c_file;
  std::filesystem::path o_file;
  std::filesystem::path a_file;
  std::filesystem::path metadata_file;
};

struct build_metadata_s {
  std::unordered_map<std::string, std::filesystem::file_time_type>
      source_mtimes;
  std::filesystem::file_time_type artifact_mtime;
};

class cache_manager_c {
public:
  explicit cache_manager_c(const std::filesystem::path &project_root);

  cache_entry_s get_library_cache_paths(const std::string &lib_name) const;

  cache_entry_s get_application_cache_paths(const std::string &app_name) const;

  bool needs_rebuild(const std::string &lib_name,
                     const std::vector<std::string> &source_files) const;

  void update_metadata(const std::string &lib_name,
                       const std::vector<std::string> &source_files);

  void ensure_cache_directories();

  std::filesystem::path cache_root() const { return _cache_root; }

private:
  std::filesystem::path _project_root;
  std::filesystem::path _cache_root;

  std::optional<build_metadata_s>
  load_metadata(const std::filesystem::path &metadata_file) const;

  void save_metadata(const std::filesystem::path &metadata_file,
                     const build_metadata_s &metadata);

  std::filesystem::file_time_type
  get_file_mtime(const std::filesystem::path &file) const;
};

} // namespace truk::core
