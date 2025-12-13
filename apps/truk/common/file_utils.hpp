#pragma once

#include <string>

namespace truk::common {

std::string read_file(const std::string &path);
bool write_file(const std::string &path, const std::string &content);

std::string get_directory(const std::string &file_path);
std::string resolve_path(const std::string &import_path,
                         const std::string &current_file_path);
std::string canonicalize_path(const std::string &path);

} // namespace truk::common
