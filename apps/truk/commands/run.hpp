#pragma once

#include <string>
#include <vector>

namespace truk::commands {

struct run_options_s {
  std::string input_file;
  std::vector<std::string> include_paths;
  std::vector<std::string> library_paths;
  std::vector<std::string> libraries;
  std::vector<std::string> rpaths;
  int argc;
  char **argv;
};

int run(const run_options_s &opts);

} // namespace truk::commands
