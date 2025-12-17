#pragma once

#include <string>
#include <vector>

namespace truk::commands {

struct test_options_s {
  std::string input_file;
  std::vector<std::string> include_paths;
  std::vector<std::string> library_paths;
  std::vector<std::string> libraries;
  std::vector<std::string> rpaths;
};

int test(const test_options_s &opts);

} // namespace truk::commands
