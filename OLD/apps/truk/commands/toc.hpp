#pragma once

#include <string>
#include <vector>

namespace truk::commands {

struct toc_options_s {
  std::string input_file;
  std::string output_file;
  std::vector<std::string> include_paths;
};

int toc(const toc_options_s &opts);

} // namespace truk::commands
