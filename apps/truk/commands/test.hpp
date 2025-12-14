#pragma once

#include <string>
#include <filesystem>
#include <optional>

namespace truk::commands {

struct test_options_s {
  std::filesystem::path target_dir;
  std::optional<std::string> specific_test;
};

int test(const test_options_s &opts);

} // namespace truk::commands
