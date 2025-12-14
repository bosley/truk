#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace truk::commands {

struct build_options_s {
  std::filesystem::path target_dir;
  std::optional<std::string> specific_target;
};

int build(const build_options_s &opts);

} // namespace truk::commands
