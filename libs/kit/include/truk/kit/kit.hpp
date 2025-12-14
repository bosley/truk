#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace truk::kit {

/*

This kit_c object will take in a kit path (file path)
and attempt to load it, getting all of the defined objects from it
throwing a kit_exception_c variant on parse/include error for caller to collect
if no exception, then good. response will be everything caller needs to
construct the project

*/

enum class exception_e {
  PARSE_ERROR,   // TODO: add errors as needed
  INCLUDE_ERROR, // note: kit is-a lexer/parser for .kit files so we will want
                 // to pass source info for markup
  UNKNOWN_ERROR
};

class kit_exception_c : public std::exception {
public:
  kit_exception_c() = delete;
  kit_exception_c(exception_e type, std::size_t at_index,
                  const std::string &message)
      : _type(type), _message(message), _at(at_index) {}

  virtual const char *what() const noexcept override {
    return _message.c_str();
  }

  exception_e type() const noexcept { return _type; }

  std::size_t at() const noexcept { return _at; }

private:
  exception_e _type;
  std::string _message;
  std::size_t _at; // where error occurred
};

enum class target_type_e {
  LIBRARY,
  APPLICATION,
};

class target_base_if {
public:
  target_base_if() = delete;
  target_base_if(const target_type_e &type) : type(type) {}

  const target_type_e type;
};

class target_application_c : public target_base_if {
public:
  target_application_c() = delete;
  target_application_c(
      const std::string &source_entry_file_path,
      const std::string &output_file_path,
      const std::optional<std::vector<std::string>> &libraries = std::nullopt,
      const std::optional<std::vector<std::string>> &library_paths =
          std::nullopt,
      const std::optional<std::vector<std::string>> &include_paths =
          std::nullopt)
      : target_base_if(target_type_e::APPLICATION),
        source_entry_file_path(source_entry_file_path),
        output_file_path(output_file_path), libraries(libraries),
        library_paths(library_paths), include_paths(include_paths) {}

  std::string source_entry_file_path;
  std::string output_file_path;
  std::optional<std::vector<std::string>> libraries;
  std::optional<std::vector<std::string>> library_paths;
  std::optional<std::vector<std::string>> include_paths;
};

class target_library_c : public target_base_if {
public:
  target_library_c() = delete;
  target_library_c(
      const std::string &source_entry_file_path,
      const std::optional<std::vector<std::string>> &depends = std::nullopt,
      const std::optional<std::string> &test_file_path = std::nullopt,
      const std::optional<std::vector<std::string>> &include_paths =
          std::nullopt)
      : target_base_if(target_type_e::LIBRARY),
        source_entry_file_path(source_entry_file_path), depends(depends),
        test_file_path(test_file_path), include_paths(include_paths) {}

  std::string source_entry_file_path;
  std::optional<std::vector<std::string>> depends;
  std::optional<std::string> test_file_path;
  std::optional<std::vector<std::string>> include_paths;
};

struct kit_config_s {
  std::string project_name;
  std::filesystem::path kit_file_directory;
  std::vector<std::pair<std::string, target_library_c>> libraries;
  std::vector<std::pair<std::string, target_application_c>> applications;
};

struct build_order_s {
  std::vector<std::pair<std::string, target_library_c>> libraries;
  std::vector<std::pair<std::string, target_application_c>> applications;
};

kit_config_s parse_kit_file(const std::filesystem::path &kit_path);

build_order_s resolve_build_order(const kit_config_s &config);

std::optional<std::filesystem::path>
find_kit_file(const std::filesystem::path &start_dir);

std::filesystem::path resolve_path(const std::filesystem::path &base,
                                   const std::string &relative);

} // namespace truk::kit
