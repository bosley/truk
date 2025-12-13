#pragma once

#include <string>

namespace truk::common {

std::string read_file(const std::string &path);
bool write_file(const std::string &path, const std::string &content);

} // namespace truk::common
