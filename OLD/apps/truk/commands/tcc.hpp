#pragma once

#include <string>
#include <vector>

namespace truk::commands {

struct tcc_options_s {
  std::string input_file;
  std::string output_file;
  std::vector<std::string> include_paths;
  std::vector<std::string> library_paths;
  std::vector<std::string> libraries;
  std::vector<std::string> rpaths;
};

int tcc(const tcc_options_s &opts);

} // namespace truk::commands
