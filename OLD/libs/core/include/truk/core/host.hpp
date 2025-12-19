#pragma once

#include "exceptions.hpp"
#include <filesystem>
#include <string>
#include <unordered_set>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#define TRUK_PLATFORM_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
#define TRUK_PLATFORM_MACOS
#elif defined(__linux__)
#define TRUK_PLATFORM_LINUX
#else
#define TRUK_PLATFORM_UNKNOWN
#endif

namespace truk::core {

enum class platform_e { WINDOWS, MACOS, LINUX, UNKNOWN };

enum class host_error_e : int { UNKNOWN_PLATFORM = 1 };

class host_c {
public:
  host_c() : _initial_working_directory(std::filesystem::current_path()) {
#if defined(TRUK_PLATFORM_WINDOWS)
    _platform = platform_e::WINDOWS;
#elif defined(TRUK_PLATFORM_MACOS)
    _platform = platform_e::MACOS;
#elif defined(TRUK_PLATFORM_LINUX)
    _platform = platform_e::LINUX;
#else
    _platform = platform_e::UNKNOWN;
#endif
    if (_platform == platform_e::UNKNOWN) {
      throw host_exception_c(static_cast<int>(host_error_e::UNKNOWN_PLATFORM),
                             "Unsupported platform detected at runtime");
    }
  }

  [[nodiscard]] platform_e get_platform() const noexcept { return _platform; }

  [[nodiscard]] std::filesystem::path
  get_initial_working_directory() const noexcept {
    return _initial_working_directory;
  }

  [[nodiscard]] std::filesystem::path get_current_working_directory() const {
    return std::filesystem::current_path();
  }

  void add_include_dir(const std::string &path) {
    if (_include_dirs.find(path) != _include_dirs.end()) {
      return;
    }
    _include_dirs.insert(path);
  }

  [[nodiscard]] const std::unordered_set<std::string> &
  get_include_dirs() const noexcept {
    return _include_dirs;
  }

  [[nodiscard]] bool has_include_dir(const std::string &path) const noexcept {
    return _include_dirs.find(path) != _include_dirs.end();
  }

private:
  platform_e _platform;
  std::filesystem::path _initial_working_directory;
  std::unordered_set<std::string> _include_dirs;
};

} // namespace truk::core
