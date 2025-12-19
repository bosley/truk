#include <fmt/core.h>
#include <truk/core/error_reporter.hpp>

namespace truk::core {

error_reporter_c::error_reporter_c() = default;

error_reporter_c::~error_reporter_c() = default;

void error_reporter_c::set_color_mode(bool enabled) {
  _display.set_color_mode(enabled);
}

void error_reporter_c::report_parse_error(const std::string &file_path,
                                          const std::string &source,
                                          std::size_t line, std::size_t column,
                                          const std::string &message) {
  _errors.emplace_back(error_phase_e::PARSING, message, file_path, 0, line,
                       column, true);
  _display.show_error(file_path, source, line, column, message);
}

void error_reporter_c::report_import_error(const std::string &file_path,
                                           const std::string &message,
                                           std::size_t line,
                                           std::size_t column) {
  report_import_error_with_type(file_path, message, line, column, false);
}

void error_reporter_c::report_import_error_with_type(
    const std::string &file_path, const std::string &message, std::size_t line,
    std::size_t column, bool is_parse_error) {
  error_phase_e phase = is_parse_error ? error_phase_e::PARSING
                                       : error_phase_e::IMPORT_RESOLUTION;

  _errors.emplace_back(phase, message, file_path, 0, line, column, line > 0);

  if (line > 0) {
    const char *error_type = is_parse_error ? "Parse error" : "Import error";
    fmt::print(stderr, "{} in '{}' at line {}, column {}: {}\n", error_type,
               file_path, line, column, message);
  } else {
    fmt::print(stderr, "Import error in '{}': {}\n", file_path, message);
  }
}

void error_reporter_c::report_typecheck_error(const std::string &file_path,
                                              const std::string &source,
                                              std::size_t source_index,
                                              const std::string &message) {
  std::size_t line = 0, column = 0;
  error_display_c::source_index_to_line_column(source, source_index, line,
                                               column);

  _errors.emplace_back(error_phase_e::TYPE_CHECKING, message, file_path,
                       source_index, line, column, true);
  _display.show_error_at_index(file_path, source, source_index, message);
}

void error_reporter_c::report_emission_error(const std::string &file_path,
                                             const std::string &source,
                                             std::size_t source_index,
                                             const std::string &message,
                                             const std::string &phase_context) {
  std::size_t line = 0, column = 0;
  error_display_c::source_index_to_line_column(source, source_index, line,
                                               column);

  std::string full_message = message;
  if (!phase_context.empty()) {
    full_message = message + " (" + phase_context + ")";
  }

  _errors.emplace_back(error_phase_e::CODE_EMISSION, full_message, file_path,
                       source_index, line, column, true);
  _display.show_error_at_index(file_path, source, source_index, full_message);
}

void error_reporter_c::report_compilation_error(const std::string &message) {
  _errors.emplace_back(error_phase_e::C_COMPILATION, message);
  fmt::print(stderr, "Compilation error: {}\n", message);
}

void error_reporter_c::report_file_error(const std::string &file_path,
                                         const std::string &message) {
  _errors.emplace_back(error_phase_e::FILE_IO, message, file_path);
  fmt::print(stderr, "File error in '{}': {}\n", file_path, message);
}

void error_reporter_c::report_generic_error(error_phase_e phase,
                                            const std::string &message) {
  _errors.emplace_back(phase, message);
  fmt::print(stderr, "{} error: {}\n", phase_name(phase), message);
}

void error_reporter_c::clear() { _errors.clear(); }

void error_reporter_c::print_summary() const {
  if (_errors.empty()) {
    return;
  }

  fmt::print(stderr, "\n");
  fmt::print(stderr, "Compilation failed with {} error(s)\n", _errors.size());
}

const char *error_reporter_c::phase_name(error_phase_e phase) const {
  switch (phase) {
  case error_phase_e::PARSING:
    return "Parse";
  case error_phase_e::IMPORT_RESOLUTION:
    return "Import";
  case error_phase_e::TYPE_CHECKING:
    return "Type";
  case error_phase_e::CODE_EMISSION:
    return "Emission";
  case error_phase_e::C_COMPILATION:
    return "C compilation";
  case error_phase_e::FILE_IO:
    return "File I/O";
  case error_phase_e::UNKNOWN:
  default:
    return "Unknown";
  }
}

} // namespace truk::core
