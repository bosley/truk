#pragma once

#include <sstream>
#include <string>

namespace truk::emitc {

class output_buffer_c {
public:
  output_buffer_c() = default;
  ~output_buffer_c() = default;

  void emit_line(const std::string &line);
  void emit_statement(const std::string &stmt);
  void emit_expression(const std::string &expr);
  void emit_block_start();
  void emit_block_end();

  void indent();
  void dedent();
  int current_indent() const;

  std::string get_content() const;
  void clear();

private:
  std::string make_indent() const;

  std::stringstream _buffer;
  int _indent_level{0};
};

} // namespace truk::emitc
