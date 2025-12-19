#pragma once

#include <string>
#include <truk/core/error_display.hpp>
#include <vector>

namespace truk::core {

enum class error_phase_e {
  PARSING,
  IMPORT_RESOLUTION,
  TYPE_CHECKING,
  CODE_EMISSION,
  C_COMPILATION,
  FILE_IO,
  UNKNOWN
};

struct compilation_error_s {
  error_phase_e phase;
  std::string message;
  std::string file_path;
  std::size_t source_index;
  std::size_t line;
  std::size_t column;
  bool has_source_location;

  compilation_error_s(error_phase_e p, std::string msg, std::string file = "",
                      std::size_t idx = 0, std::size_t ln = 0,
                      std::size_t col = 0, bool has_loc = false)
      : phase(p), message(std::move(msg)), file_path(std::move(file)),
        source_index(idx), line(ln), column(col), has_source_location(has_loc) {
  }
};

class error_reporter_c {
public:
  error_reporter_c();
  ~error_reporter_c();

  void set_color_mode(bool enabled);

  void report_parse_error(const std::string &file_path,
                          const std::string &source, std::size_t line,
                          std::size_t column, const std::string &message);

  void report_import_error(const std::string &file_path,
                           const std::string &message, std::size_t line = 0,
                           std::size_t column = 0);

  void report_import_error_with_type(const std::string &file_path,
                                     const std::string &message,
                                     std::size_t line, std::size_t column,
                                     bool is_parse_error);

  void report_typecheck_error(const std::string &file_path,
                              const std::string &source,
                              std::size_t source_index,
                              const std::string &message);

  void report_emission_error(const std::string &file_path,
                             const std::string &source,
                             std::size_t source_index,
                             const std::string &message,
                             const std::string &phase_context = "");

  void report_compilation_error(const std::string &message);

  void report_file_error(const std::string &file_path,
                         const std::string &message);

  void report_generic_error(error_phase_e phase, const std::string &message);

  bool has_errors() const { return !_errors.empty(); }
  std::size_t error_count() const { return _errors.size(); }
  const std::vector<compilation_error_s> &errors() const { return _errors; }

  void clear();

  void print_summary() const;

private:
  error_display_c _display;
  std::vector<compilation_error_s> _errors;

  const char *phase_name(error_phase_e phase) const;
};

} // namespace truk::core
