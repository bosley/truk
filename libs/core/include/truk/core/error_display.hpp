#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace truk::core {

enum class error_severity_e { ERROR, WARNING, NOTE };

struct source_location_s {
  std::string filename;
  std::size_t line;
  std::size_t column;
  std::string message;
  error_severity_e severity;

  source_location_s(std::string fname, std::size_t ln, std::size_t col,
                    std::string msg,
                    error_severity_e sev = error_severity_e::ERROR)
      : filename(std::move(fname)), line(ln), column(col),
        message(std::move(msg)), severity(sev) {}
};

class error_display_c {
public:
  error_display_c();
  ~error_display_c();

  void set_color_mode(bool enabled);
  bool should_use_color() const;

  void show_error(const std::string &filename, const std::string &source,
                  std::size_t line, std::size_t column,
                  const std::string &message);

  void show_warning(const std::string &filename, const std::string &source,
                    std::size_t line, std::size_t column,
                    const std::string &message);

  void show_note(const std::string &filename, const std::string &source,
                 std::size_t line, std::size_t column,
                 const std::string &message);

  void show(const std::string &source, const source_location_s &location);

  void show_error_at_index(const std::string &filename,
                           const std::string &source, std::size_t source_index,
                           const std::string &message);

  void set_context_lines(std::size_t before, std::size_t after);

  static void source_index_to_line_column(const std::string &source,
                                          std::size_t source_index,
                                          std::size_t &out_line,
                                          std::size_t &out_column);

private:
  bool _use_color;
  std::size_t _context_lines_before;
  std::size_t _context_lines_after;

  std::vector<std::string> split_lines(const std::string &source) const;
  std::size_t calculate_line_number_width(std::size_t max_line) const;
  std::string expand_tabs(const std::string &line,
                          std::size_t tab_width = 4) const;
  std::size_t visual_column(const std::string &line, std::size_t byte_column,
                            std::size_t tab_width = 4) const;

  void print_severity_header(const source_location_s &location) const;
  void print_location(const source_location_s &location) const;
  void print_source_context(const std::vector<std::string> &lines,
                            const source_location_s &location,
                            std::size_t line_number_width) const;
};

} // namespace truk::core
