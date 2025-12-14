#include "clean.hpp"
#include <filesystem>
#include <fmt/core.h>
#include <truk/kit/kit.hpp>

namespace truk::commands {

namespace fs = std::filesystem;

int clean(const clean_options_s &opts) {
  auto kit_path = kit::find_kit_file(opts.target_dir);
  if (!kit_path.has_value()) {
    fmt::print(stderr,
               "Error: No truk.kit found in '{}' or parent directories\n",
               opts.target_dir.string());
    return 1;
  }

  kit::kit_config_s config;
  try {
    config = kit::parse_kit_file(kit_path.value());
  } catch (const kit::kit_exception_c &e) {
    fmt::print(stderr, "Error parsing kit file: {}\n", e.what());
    return 1;
  }

  int removed_count = 0;

  for (const auto &[name, lib] : config.libraries) {
    fs::path lib_path(lib.output_file_path);
    if (fs::exists(lib_path)) {
      try {
        fs::remove(lib_path);
        fmt::print("Removed: {}\n", lib.output_file_path);
        removed_count++;
      } catch (const std::exception &e) {
        fmt::print(stderr, "Warning: Failed to remove {}: {}\n",
                   lib.output_file_path, e.what());
      }
    }

    std::string test_exe = "build/test_" + name;
    if (fs::exists(test_exe)) {
      try {
        fs::remove(test_exe);
        fmt::print("Removed: {}\n", test_exe);
        removed_count++;
      } catch (const std::exception &e) {
        fmt::print(stderr, "Warning: Failed to remove {}: {}\n", test_exe,
                   e.what());
      }
    }
  }

  for (const auto &[name, app] : config.applications) {
    fs::path app_path(app.output_file_path);
    if (fs::exists(app_path)) {
      try {
        fs::remove(app_path);
        fmt::print("Removed: {}\n", app.output_file_path);
        removed_count++;
      } catch (const std::exception &e) {
        fmt::print(stderr, "Warning: Failed to remove {}: {}\n",
                   app.output_file_path, e.what());
      }
    }
  }

  fs::path build_dir = config.kit_file_directory / "build";
  if (fs::exists(build_dir) && fs::is_directory(build_dir)) {
    try {
      if (fs::is_empty(build_dir)) {
        fs::remove(build_dir);
        fmt::print("Removed empty build directory\n");
      }
    } catch (const std::exception &e) {
    }
  }

  if (removed_count == 0) {
    fmt::print("No build artifacts to clean\n");
  } else {
    fmt::print("Cleaned {} build artifact(s)\n", removed_count);
  }

  return 0;
}

} // namespace truk::commands
