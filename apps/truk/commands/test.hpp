#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace truk::commands {

struct test_options_s {
  std::filesystem::path target_dir;
  std::optional<std::string> specific_test;
};

int test(const test_options_s &opts);

} // namespace truk::commands
