#include <truk/emitc/output_buffer.hpp>

namespace truk::emitc {

void output_buffer_c::emit_line(const std::string &line) {
  _buffer << make_indent() << line << "\n";
}

void output_buffer_c::emit_statement(const std::string &stmt) {
  _buffer << make_indent() << stmt << ";\n";
}

void output_buffer_c::emit_expression(const std::string &expr) {
  _buffer << expr;
}

void output_buffer_c::emit_block_start() {
  _buffer << " {\n";
  indent();
}

void output_buffer_c::emit_block_end() {
  dedent();
  _buffer << make_indent() << "}";
}

void output_buffer_c::indent() { _indent_level++; }

void output_buffer_c::dedent() {
  if (_indent_level > 0) {
    _indent_level--;
  }
}

int output_buffer_c::current_indent() const { return _indent_level; }

std::string output_buffer_c::get_content() const { return _buffer.str(); }

void output_buffer_c::clear() {
  _buffer.str("");
  _buffer.clear();
  _indent_level = 0;
}

std::string output_buffer_c::make_indent() const {
  return std::string(_indent_level * 2, ' ');
}

} // namespace truk::emitc
