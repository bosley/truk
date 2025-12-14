#pragma once

#include <string>

namespace truk::commands {

struct new_options_s {
  std::string project_name;
};

int new_project(const new_options_s &opts);

} // namespace truk::commands
