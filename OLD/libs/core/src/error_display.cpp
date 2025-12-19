#include <algorithm>
#include <cstdlib>
#include <fmt/color.h>
#include <fmt/core.h>
#include <sstream>
#include <truk/core/error_display.hpp>

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define STDERR_FILENO 2
#else
#include <unistd.h>
#endif

namespace truk::core {

error_display_c::error_display_c()
    : _use_color(isatty(STDERR_FILENO) != 0), _context_lines_before(1),
      _context_lines_after(1) {
  const char *no_color = std::getenv("NO_COLOR");
  if (no_color != nullptr && no_color[0] != '\0') {
    _use_color = false;
  }
}

error_display_c::~error_display_c() = default;

void error_display_c::set_color_mode(bool enabled) { _use_color = enabled; }

bool error_display_c::should_use_color() const { return _use_color; }

void error_display_c::set_context_lines(std::size_t before, std::size_t after) {
  _context_lines_before = before;
  _context_lines_after = after;
}

void error_display_c::show_error(const std::string &filename,
                                 const std::string &source, std::size_t line,
                                 std::size_t column,
                                 const std::string &message) {
  source_location_s location(filename, line, column, message,
                             error_severity_e::ERROR);
  show(source, location);
}

void error_display_c::show_warning(const std::string &filename,
                                   const std::string &source, std::size_t line,
                                   std::size_t column,
                                   const std::string &message) {
  source_location_s location(filename, line, column, message,
                             error_severity_e::WARNING);
  show(source, location);
}

void error_display_c::show_note(const std::string &filename,
                                const std::string &source, std::size_t line,
                                std::size_t column,
                                const std::string &message) {
  source_location_s location(filename, line, column, message,
                             error_severity_e::NOTE);
  show(source, location);
}

void error_display_c::show(const std::string &source,
                           const source_location_s &location) {
  print_severity_header(location);
  print_location(location);

  auto lines = split_lines(source);
  if (location.line == 0 || location.line > lines.size()) {
    fmt::print(stderr, "\n");
    return;
  }

  std::size_t end_line =
      std::min(location.line + _context_lines_after, lines.size());

  std::size_t line_number_width = calculate_line_number_width(end_line);

  print_source_context(lines, location, line_number_width);
}

void error_display_c::show_error_at_index(const std::string &filename,
                                          const std::string &source,
                                          std::size_t source_index,
                                          const std::string &message) {
  std::size_t line, column;
  source_index_to_line_column(source, source_index, line, column);
  show_error(filename, source, line, column, message);
}

void error_display_c::source_index_to_line_column(const std::string &source,
                                                  std::size_t source_index,
                                                  std::size_t &out_line,
                                                  std::size_t &out_column) {
  out_line = 1;
  out_column = 1;

  for (std::size_t i = 0; i < source_index && i < source.size(); ++i) {
    if (source[i] == '\n') {
      ++out_line;
      out_column = 1;
    } else if (source[i] == '\r') {
      ++out_line;
      out_column = 1;
      if (i + 1 < source.size() && source[i + 1] == '\n') {
        ++i;
      }
    } else {
      ++out_column;
    }
  }
}

std::vector<std::string>
error_display_c::split_lines(const std::string &source) const {
  std::vector<std::string> lines;
  std::string current_line;

  for (std::size_t i = 0; i < source.size(); ++i) {
    char c = source[i];
    if (c == '\n') {
      lines.push_back(current_line);
      current_line.clear();
    } else if (c == '\r') {
      if (i + 1 < source.size() && source[i + 1] == '\n') {
        ++i;
      }
      lines.push_back(current_line);
      current_line.clear();
    } else {
      current_line += c;
    }
  }

  if (!current_line.empty() || source.empty() || source.back() == '\n' ||
      source.back() == '\r') {
    lines.push_back(current_line);
  }

  return lines;
}

std::size_t
error_display_c::calculate_line_number_width(std::size_t max_line) const {
  std::size_t width = 1;
  std::size_t n = max_line;
  while (n >= 10) {
    ++width;
    n /= 10;
  }
  return width;
}

std::string error_display_c::expand_tabs(const std::string &line,
                                         std::size_t tab_width) const {
  std::string result;
  std::size_t col = 0;
  for (char c : line) {
    if (c == '\t') {
      std::size_t spaces = tab_width - (col % tab_width);
      result.append(spaces, ' ');
      col += spaces;
    } else {
      result += c;
      ++col;
    }
  }
  return result;
}

std::size_t error_display_c::visual_column(const std::string &line,
                                           std::size_t byte_column,
                                           std::size_t tab_width) const {
  std::size_t visual_col = 0;
  std::size_t byte_pos = 0;

  for (std::size_t i = 0; i < line.size() && byte_pos < byte_column; ++i) {
    unsigned char c = static_cast<unsigned char>(line[i]);

    if (c == '\t') {
      std::size_t spaces = tab_width - (visual_col % tab_width);
      visual_col += spaces;
      ++byte_pos;
    } else if ((c & 0x80) == 0) {
      ++visual_col;
      ++byte_pos;
    } else if ((c & 0xE0) == 0xC0) {
      ++visual_col;
      ++byte_pos;
      if (i + 1 < line.size())
        ++i;
    } else if ((c & 0xF0) == 0xE0) {
      ++visual_col;
      ++byte_pos;
      if (i + 1 < line.size())
        ++i;
      if (i + 1 < line.size())
        ++i;
    } else if ((c & 0xF8) == 0xF0) {
      ++visual_col;
      ++byte_pos;
      if (i + 1 < line.size())
        ++i;
      if (i + 1 < line.size())
        ++i;
      if (i + 1 < line.size())
        ++i;
    } else {
      ++byte_pos;
    }
  }

  return visual_col;
}

void error_display_c::print_severity_header(
    const source_location_s &location) const {
  if (_use_color) {
    switch (location.severity) {
    case error_severity_e::ERROR:
      fmt::print(stderr, fg(fmt::color::red) | fmt::emphasis::bold, "error");
      break;
    case error_severity_e::WARNING:
      fmt::print(stderr, fg(fmt::color::yellow) | fmt::emphasis::bold,
                 "warning");
      break;
    case error_severity_e::NOTE:
      fmt::print(stderr, fg(fmt::color::cyan) | fmt::emphasis::bold, "note");
      break;
    }
    fmt::print(stderr, fmt::emphasis::bold, ": ");
    fmt::print(stderr, "{}\n", location.message);
  } else {
    switch (location.severity) {
    case error_severity_e::ERROR:
      fmt::print(stderr, "error: {}\n", location.message);
      break;
    case error_severity_e::WARNING:
      fmt::print(stderr, "warning: {}\n", location.message);
      break;
    case error_severity_e::NOTE:
      fmt::print(stderr, "note: {}\n", location.message);
      break;
    }
  }
}

void error_display_c::print_location(const source_location_s &location) const {
  if (_use_color) {
    fmt::print(stderr, fg(fmt::color::cyan) | fmt::emphasis::bold, "  --> ");
    fmt::print(stderr, "{}:{}:{}\n", location.filename, location.line,
               location.column);
  } else {
    fmt::print(stderr, "  --> {}:{}:{}\n", location.filename, location.line,
               location.column);
  }
}

void error_display_c::print_source_context(
    const std::vector<std::string> &lines, const source_location_s &location,
    std::size_t line_number_width) const {

  std::size_t start_line = location.line > _context_lines_before + 1
                               ? location.line - _context_lines_before - 1
                               : 0;
  std::size_t end_line =
      std::min(location.line + _context_lines_after, lines.size());

  if (_use_color) {
    fmt::print(stderr, fg(fmt::color::cyan) | fmt::emphasis::bold, "{:>{}}", "",
               line_number_width);
    fmt::print(stderr, fg(fmt::color::cyan) | fmt::emphasis::bold, " |\n");
  } else {
    fmt::print(stderr, "{:>{}}", "", line_number_width);
    fmt::print(stderr, " |\n");
  }

  for (std::size_t i = start_line; i < end_line; ++i) {
    std::size_t line_num = i + 1;
    std::string line_content = expand_tabs(lines[i]);

    if (_use_color) {
      fmt::print(stderr, fg(fmt::color::cyan) | fmt::emphasis::bold, "{:>{}}",
                 line_num, line_number_width);
      fmt::print(stderr, fg(fmt::color::cyan) | fmt::emphasis::bold, " | ");
    } else {
      fmt::print(stderr, "{:>{}}", line_num, line_number_width);
      fmt::print(stderr, " | ");
    }

    fmt::print(stderr, "{}\n", line_content);

    if (line_num == location.line) {
      if (_use_color) {
        fmt::print(stderr, fg(fmt::color::cyan) | fmt::emphasis::bold, "{:>{}}",
                   "", line_number_width);
        fmt::print(stderr, fg(fmt::color::cyan) | fmt::emphasis::bold, " | ");
      } else {
        fmt::print(stderr, "{:>{}}", "", line_number_width);
        fmt::print(stderr, " | ");
      }

      std::size_t visual_col = visual_column(lines[i], location.column - 1);

      if (_use_color) {
        fmt::print(stderr, "{:>{}}", "", visual_col);
        fmt::print(stderr, fg(fmt::color::red) | fmt::emphasis::bold, "^");
        fmt::print(stderr, "\n");
      } else {
        fmt::print(stderr, "{:>{}}", "", visual_col);
        fmt::print(stderr, "^\n");
      }
    }
  }

  if (_use_color) {
    fmt::print(stderr, fg(fmt::color::cyan) | fmt::emphasis::bold, "{:>{}}", "",
               line_number_width);
    fmt::print(stderr, fg(fmt::color::cyan) | fmt::emphasis::bold, " |\n");
  } else {
    fmt::print(stderr, "{:>{}}", "", line_number_width);
    fmt::print(stderr, " |\n");
  }
}

} // namespace truk::core
