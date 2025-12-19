#pragma once

#include <string>
#include <vector>

namespace truk::common {

struct parsed_args_s {
  std::string command;
  std::string input_file;
  std::string output_file;
  std::vector<std::string> include_paths;
  std::vector<std::string> library_paths;
  std::vector<std::string> libraries;
  std::vector<std::string> rpaths;
  std::vector<std::string> program_args;
};

parsed_args_s parse_args(int argc, char **argv);
void print_usage(const char *program_name);

} // namespace truk::common
