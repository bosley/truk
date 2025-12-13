#pragma once

#include <language/node.hpp>
#include <stdexcept>
#include <string>
#include <truk/ingestion/tokenize.hpp>
#include <vector>

namespace truk::ingestion {

class parse_error : public std::runtime_error {
public:
  parse_error(const std::string &message, std::size_t line, std::size_t column)
      : std::runtime_error(message), _line(line), _column(column) {}

  std::size_t line() const { return _line; }
  std::size_t column() const { return _column; }

private:
  std::size_t _line;
  std::size_t _column;
};

struct parse_result_s {
  std::vector<language::nodes::base_ptr> declarations;
  bool success{true};
  std::string error_message;
  std::size_t error_line{0};
  std::size_t error_column{0};
  const char *source_data{nullptr};
  std::size_t source_len{0};
};

class parser_c {
public:
  parser_c() = delete;
  parser_c(const char *data, std::size_t len);
  ~parser_c();

  std::vector<token_s> tokenize();
  parse_result_s parse();
  language::nodes::type_ptr parse_type();

private:
  const char *_data{nullptr};
  std::size_t _len{0};
  std::vector<token_s> _tokens;
  std::size_t _current{0};

  const token_s &peek() const;
  const token_s &previous() const;
  const token_s &advance();
  bool is_at_end() const;
  bool check(token_type_e type) const;
  bool check_keyword(language::keywords_e keyword) const;
  bool match(token_type_e type);
  bool match_keyword(language::keywords_e keyword);
  const token_s &consume(token_type_e type, const std::string &message);
  const token_s &consume_keyword(language::keywords_e keyword,
                                 const std::string &message);
  const token_s &consume_identifier(const std::string &message);

  std::vector<language::nodes::base_ptr> parse_program();
  language::nodes::base_ptr parse_declaration();
  language::nodes::base_ptr parse_fn_decl();
  language::nodes::base_ptr parse_struct_decl();
  language::nodes::base_ptr parse_var_decl();
  language::nodes::base_ptr parse_const_decl();

  language::nodes::type_ptr parse_type_internal();
  language::nodes::type_ptr parse_type_annotation();
  language::nodes::type_ptr parse_primitive_type(language::keywords_e keyword);
  language::nodes::type_ptr parse_array_type();
  language::nodes::type_ptr parse_pointer_type();
  language::nodes::type_ptr parse_function_type();

  language::nodes::base_ptr parse_statement();
  language::nodes::base_ptr parse_block();
  language::nodes::base_ptr parse_if_stmt();
  language::nodes::base_ptr parse_while_stmt();
  language::nodes::base_ptr parse_for_stmt();
  language::nodes::base_ptr parse_return_stmt();
  language::nodes::base_ptr parse_break_stmt();
  language::nodes::base_ptr parse_continue_stmt();
  language::nodes::base_ptr parse_expression_stmt();

  language::nodes::base_ptr parse_expression();
  language::nodes::base_ptr parse_assignment();
  language::nodes::base_ptr parse_cast();
  language::nodes::base_ptr parse_logical_or();
  language::nodes::base_ptr parse_logical_and();
  language::nodes::base_ptr parse_equality();
  language::nodes::base_ptr parse_comparison();
  language::nodes::base_ptr parse_bitwise_or();
  language::nodes::base_ptr parse_bitwise_xor();
  language::nodes::base_ptr parse_bitwise_and();
  language::nodes::base_ptr parse_shift();
  language::nodes::base_ptr parse_additive();
  language::nodes::base_ptr parse_multiplicative();
  language::nodes::base_ptr parse_unary();
  language::nodes::base_ptr parse_postfix();
  language::nodes::base_ptr parse_primary();

  std::vector<language::nodes::parameter_s> parse_param_list();
  language::nodes::parameter_s parse_param();
  std::vector<language::nodes::struct_field_s> parse_field_list();
  language::nodes::struct_field_s parse_field();
  std::vector<language::nodes::base_ptr> parse_argument_list();
  language::nodes::base_ptr parse_array_literal();
  language::nodes::base_ptr parse_struct_literal();
};

} // namespace truk::ingestion
