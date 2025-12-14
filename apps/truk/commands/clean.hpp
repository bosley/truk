#pragma once

#include <string>
#include <filesystem>

namespace truk::commands {

struct clean_options_s {
  std::filesystem::path target_dir;
};

int clean(const clean_options_s &opts);

} // namespace truk::commands
