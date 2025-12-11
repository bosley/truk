#include <sstream>
#include <truk/ingestion/parser.hpp>

namespace truk::ingestion {

parser_c::parser_c(const char *data, std::size_t len) : _data(data), _len(len) {
  _tokens = tokenize();
}

parser_c::~parser_c() = default;

std::vector<token_s> parser_c::tokenize() {
  std::vector<token_s> tokens;
  tokenizer_c tokenizer(_data, _len);

  while (true) {
    auto token_opt = tokenizer.next_token();
    if (!token_opt.has_value()) {
      break;
    }

    auto token = token_opt.value();
    tokens.push_back(token);

    if (token.type == token_type_e::END_OF_FILE) {
      break;
    }
  }

  return tokens;
}

parse_result_s parser_c::parse() {
  parse_result_s result;
  try {
    result.declarations = parse_program();
    result.success = true;
  } catch (const parse_error &e) {
    result.success = false;
    result.error_message = e.what();
    result.error_line = e.line();
    result.error_column = e.column();
  } catch (const std::exception &e) {
    result.success = false;
    result.error_message = std::string("Unexpected error: ") + e.what();
    result.error_line = 0;
    result.error_column = 0;
  }
  return result;
}

language::nodes::type_ptr parser_c::parse_type() {
  try {
    return parse_type_internal();
  } catch (...) {
    return nullptr;
  }
}

const token_s &parser_c::peek() const {
  if (_current >= _tokens.size()) {
    return _tokens.back();
  }
  return _tokens[_current];
}

const token_s &parser_c::previous() const {
  if (_current == 0) {
    return _tokens[0];
  }
  return _tokens[_current - 1];
}

const token_s &parser_c::advance() {
  if (!is_at_end()) {
    _current++;
  }
  return previous();
}

bool parser_c::is_at_end() const {
  return peek().type == token_type_e::END_OF_FILE;
}

bool parser_c::check(token_type_e type) const {
  if (is_at_end()) {
    return false;
  }
  return peek().type == type;
}

bool parser_c::check_keyword(language::keywords_e keyword) const {
  if (is_at_end()) {
    return false;
  }
  const auto &token = peek();
  return token.type == token_type_e::KEYWORD && token.keyword.has_value() &&
         token.keyword.value() == keyword;
}

bool parser_c::match(token_type_e type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

bool parser_c::match_keyword(language::keywords_e keyword) {
  if (check_keyword(keyword)) {
    advance();
    return true;
  }
  return false;
}

const token_s &parser_c::consume(token_type_e type,
                                 const std::string &message) {
  if (check(type)) {
    return advance();
  }
  const auto &token = peek();
  throw parse_error(message, token.line, token.column);
}

const token_s &parser_c::consume_keyword(language::keywords_e keyword,
                                         const std::string &message) {
  if (check_keyword(keyword)) {
    return advance();
  }
  const auto &token = peek();
  throw parse_error(message, token.line, token.column);
}

const token_s &parser_c::consume_identifier(const std::string &message) {
  if (check(token_type_e::IDENTIFIER)) {
    return advance();
  }
  const auto &token = peek();
  throw parse_error(message, token.line, token.column);
}

std::vector<language::nodes::base_ptr> parser_c::parse_program() {
  std::vector<language::nodes::base_ptr> declarations;
  while (!is_at_end()) {
    declarations.push_back(parse_declaration());
  }
  return declarations;
}

language::nodes::base_ptr parser_c::parse_declaration() {
  if (check_keyword(language::keywords_e::FN)) {
    return parse_fn_decl();
  }
  if (check_keyword(language::keywords_e::STRUCT)) {
    return parse_struct_decl();
  }
  if (check_keyword(language::keywords_e::VAR)) {
    return parse_var_decl();
  }
  if (check_keyword(language::keywords_e::CONST)) {
    return parse_const_decl();
  }
  const auto &token = peek();
  throw parse_error("Expected declaration (fn, struct, var, or const)",
                    token.line, token.column);
}

language::nodes::base_ptr parser_c::parse_fn_decl() {
  const auto &fn_token =
      consume_keyword(language::keywords_e::FN, "Expected 'fn' keyword");
  const auto &name_token = consume_identifier("Expected function name");
  language::nodes::identifier_s name(name_token.lexeme,
                                     name_token.source_index);

  consume(token_type_e::LEFT_PAREN, "Expected '(' after function name");

  std::vector<language::nodes::parameter_s> params;
  if (!check(token_type_e::RIGHT_PAREN)) {
    params = parse_param_list();
  }

  consume(token_type_e::RIGHT_PAREN, "Expected ')' after parameters");

  language::nodes::type_ptr return_type;
  if (check(token_type_e::COLON)) {
    return_type = parse_type_annotation();
  } else {
    return_type = std::make_unique<language::nodes::primitive_type_c>(
        language::keywords_e::VOID, fn_token.source_index);
  }

  auto body = parse_block();

  return std::make_unique<language::nodes::fn_c>(
      fn_token.source_index, std::move(name), std::move(params),
      std::move(return_type), std::move(body));
}

language::nodes::base_ptr parser_c::parse_struct_decl() {
  const auto &struct_token = consume_keyword(language::keywords_e::STRUCT,
                                             "Expected 'struct' keyword");
  const auto &name_token = consume_identifier("Expected struct name");
  language::nodes::identifier_s name(name_token.lexeme,
                                     name_token.source_index);

  consume(token_type_e::LEFT_BRACE, "Expected '{' after struct name");

  std::vector<language::nodes::struct_field_s> fields;
  if (!check(token_type_e::RIGHT_BRACE)) {
    fields = parse_field_list();
  }

  consume(token_type_e::RIGHT_BRACE, "Expected '}' after struct fields");

  return std::make_unique<language::nodes::struct_c>(
      struct_token.source_index, std::move(name), std::move(fields));
}

language::nodes::base_ptr parser_c::parse_var_decl() {
  const auto &var_token =
      consume_keyword(language::keywords_e::VAR, "Expected 'var' keyword");
  const auto &name_token = consume_identifier("Expected variable name");
  language::nodes::identifier_s name(name_token.lexeme,
                                     name_token.source_index);

  auto type = parse_type_annotation();

  std::optional<language::nodes::base_ptr> initializer = std::nullopt;
  if (match(token_type_e::EQUAL)) {
    initializer = parse_expression();
  }

  consume(token_type_e::SEMICOLON, "Expected ';' after variable declaration");

  return std::make_unique<language::nodes::var_c>(
      var_token.source_index, std::move(name), std::move(type),
      std::move(initializer));
}

language::nodes::base_ptr parser_c::parse_const_decl() {
  const auto &const_token =
      consume_keyword(language::keywords_e::CONST, "Expected 'const' keyword");
  const auto &name_token = consume_identifier("Expected constant name");
  language::nodes::identifier_s name(name_token.lexeme,
                                     name_token.source_index);

  auto type = parse_type_annotation();

  consume(token_type_e::EQUAL, "Expected '=' in constant declaration");

  auto value = parse_expression();

  consume(token_type_e::SEMICOLON, "Expected ';' after constant declaration");

  return std::make_unique<language::nodes::const_c>(
      const_token.source_index, std::move(name), std::move(type),
      std::move(value));
}

language::nodes::type_ptr parser_c::parse_type_annotation() {
  consume(token_type_e::COLON, "Expected ':' in type annotation");
  return parse_type_internal();
}

language::nodes::type_ptr parser_c::parse_type_internal() {
  if (check(token_type_e::STAR)) {
    return parse_pointer_type();
  }
  if (check(token_type_e::LEFT_BRACKET)) {
    return parse_array_type();
  }
  if (check(token_type_e::KEYWORD)) {
    const auto &token = peek();
    if (token.keyword.has_value()) {
      auto keyword = token.keyword.value();
      switch (keyword) {
      case language::keywords_e::FN:
        return parse_function_type();
      case language::keywords_e::I8:
      case language::keywords_e::I16:
      case language::keywords_e::I32:
      case language::keywords_e::I64:
      case language::keywords_e::U8:
      case language::keywords_e::U16:
      case language::keywords_e::U32:
      case language::keywords_e::U64:
      case language::keywords_e::F32:
      case language::keywords_e::F64:
      case language::keywords_e::BOOL:
      case language::keywords_e::VOID:
        return parse_primitive_type(keyword);
      default:
        break;
      }
    }
  }
  if (check(token_type_e::IDENTIFIER)) {
    const auto &token = advance();
    language::nodes::identifier_s name(token.lexeme, token.source_index);
    return std::make_unique<language::nodes::named_type_c>(token.source_index,
                                                           std::move(name));
  }
  const auto &token = peek();
  throw parse_error("Expected type", token.line, token.column);
}

language::nodes::type_ptr
parser_c::parse_primitive_type(language::keywords_e keyword) {
  const auto &token = advance();
  return std::make_unique<language::nodes::primitive_type_c>(
      keyword, token.source_index);
}

language::nodes::type_ptr parser_c::parse_array_type() {
  const auto &bracket_token =
      consume(token_type_e::LEFT_BRACKET, "Expected '['");

  std::optional<std::size_t> size = std::nullopt;
  if (!check(token_type_e::RIGHT_BRACKET)) {
    auto size_expr = parse_expression();

    auto *literal = dynamic_cast<language::nodes::literal_c *>(size_expr.get());
    if (!literal) {
      throw parse_error("Array size must be an integer literal",
                        size_expr->source_index(), bracket_token.column);
    }

    if (literal->type() != language::nodes::literal_type_e::INTEGER) {
      throw parse_error("Array size must be an integer literal",
                        literal->source_index(), bracket_token.column);
    }

    try {
      const std::string &value = literal->value();
      int base = 0;
      size_t start_pos = 0;

      if (value.size() >= 2 && value[0] == '0') {
        if (value[1] == 'b' || value[1] == 'B') {
          base = 2;
          start_pos = 2;
        } else if (value[1] == 'o' || value[1] == 'O') {
          base = 8;
          start_pos = 2;
        }
      }

      size = std::stoull(value.substr(start_pos), nullptr, base);
    } catch (const std::invalid_argument &) {
      throw parse_error("Invalid array size literal", literal->source_index(),
                        bracket_token.column);
    } catch (const std::out_of_range &) {
      throw parse_error("Array size literal out of range",
                        literal->source_index(), bracket_token.column);
    }
  }

  consume(token_type_e::RIGHT_BRACKET, "Expected ']' after array size");

  auto element_type = parse_type_internal();

  return std::make_unique<language::nodes::array_type_c>(
      bracket_token.source_index, std::move(element_type), size);
}

language::nodes::type_ptr parser_c::parse_pointer_type() {
  const auto &star_token = consume(token_type_e::STAR, "Expected '*'");
  auto pointee_type = parse_type_internal();
  return std::make_unique<language::nodes::pointer_type_c>(
      star_token.source_index, std::move(pointee_type));
}

language::nodes::type_ptr parser_c::parse_function_type() {
  const auto &fn_token =
      consume_keyword(language::keywords_e::FN, "Expected 'fn'");

  consume(token_type_e::LEFT_PAREN, "Expected '(' after 'fn'");

  std::vector<language::nodes::type_ptr> param_types;
  if (!check(token_type_e::RIGHT_PAREN)) {
    do {
      param_types.push_back(parse_type());
    } while (match(token_type_e::COMMA));
  }

  consume(token_type_e::RIGHT_PAREN, "Expected ')' after parameter types");

  language::nodes::type_ptr return_type;
  if (match(token_type_e::COLON)) {
    return_type = parse_type();
  } else {
    return_type = std::make_unique<language::nodes::primitive_type_c>(
        language::keywords_e::VOID, fn_token.source_index);
  }

  return std::make_unique<language::nodes::function_type_c>(
      fn_token.source_index, std::move(param_types), std::move(return_type));
}

language::nodes::base_ptr parser_c::parse_statement() {
  if (check_keyword(language::keywords_e::VAR)) {
    return parse_var_decl();
  }
  if (check_keyword(language::keywords_e::CONST)) {
    return parse_const_decl();
  }
  if (check_keyword(language::keywords_e::IF)) {
    return parse_if_stmt();
  }
  if (check_keyword(language::keywords_e::WHILE)) {
    return parse_while_stmt();
  }
  if (check_keyword(language::keywords_e::FOR)) {
    return parse_for_stmt();
  }
  if (check_keyword(language::keywords_e::RETURN)) {
    return parse_return_stmt();
  }
  if (check_keyword(language::keywords_e::BREAK)) {
    return parse_break_stmt();
  }
  if (check_keyword(language::keywords_e::CONTINUE)) {
    return parse_continue_stmt();
  }
  if (check(token_type_e::LEFT_BRACE)) {
    return parse_block();
  }
  return parse_expression_stmt();
}

language::nodes::base_ptr parser_c::parse_block() {
  const auto &brace_token = consume(token_type_e::LEFT_BRACE, "Expected '{'");

  std::vector<language::nodes::base_ptr> statements;
  while (!check(token_type_e::RIGHT_BRACE) && !is_at_end()) {
    statements.push_back(parse_statement());
  }

  consume(token_type_e::RIGHT_BRACE, "Expected '}' after block");

  return std::make_unique<language::nodes::block_c>(brace_token.source_index,
                                                    std::move(statements));
}

language::nodes::base_ptr parser_c::parse_if_stmt() {
  const auto &if_token =
      consume_keyword(language::keywords_e::IF, "Expected 'if' keyword");

  auto condition = parse_expression();
  auto then_block = parse_block();

  std::optional<language::nodes::base_ptr> else_block = std::nullopt;
  if (match_keyword(language::keywords_e::ELSE)) {
    if (check_keyword(language::keywords_e::IF)) {
      else_block = parse_if_stmt();
    } else {
      else_block = parse_block();
    }
  }

  return std::make_unique<language::nodes::if_c>(
      if_token.source_index, std::move(condition), std::move(then_block),
      std::move(else_block));
}

language::nodes::base_ptr parser_c::parse_while_stmt() {
  const auto &while_token =
      consume_keyword(language::keywords_e::WHILE, "Expected 'while' keyword");

  auto condition = parse_expression();
  auto body = parse_block();

  return std::make_unique<language::nodes::while_c>(
      while_token.source_index, std::move(condition), std::move(body));
}

language::nodes::base_ptr parser_c::parse_for_stmt() {
  const auto &for_token =
      consume_keyword(language::keywords_e::FOR, "Expected 'for' keyword");

  std::optional<language::nodes::base_ptr> init = std::nullopt;
  std::optional<language::nodes::base_ptr> condition = std::nullopt;
  std::optional<language::nodes::base_ptr> post = std::nullopt;

  if (!check(token_type_e::SEMICOLON)) {
    if (check_keyword(language::keywords_e::VAR)) {
      const auto &var_token =
          consume_keyword(language::keywords_e::VAR, "Expected 'var' keyword");
      const auto &name_token = consume_identifier("Expected variable name");
      language::nodes::identifier_s name(name_token.lexeme,
                                         name_token.source_index);

      auto type = parse_type_annotation();

      consume(token_type_e::EQUAL, "Expected '=' in variable declaration");

      auto initializer = parse_expression();

      init = std::make_unique<language::nodes::var_c>(
          var_token.source_index, std::move(name), std::move(type),
          std::move(initializer));
    } else {
      init = parse_expression();
    }
  }
  consume(token_type_e::SEMICOLON, "Expected ';' after for loop initializer");

  if (!check(token_type_e::SEMICOLON)) {
    condition = parse_expression();
  }
  consume(token_type_e::SEMICOLON, "Expected ';' after for loop condition");

  if (!check(token_type_e::LEFT_BRACE)) {
    post = parse_expression();
  }

  auto body = parse_block();

  return std::make_unique<language::nodes::for_c>(
      for_token.source_index, std::move(init), std::move(condition),
      std::move(post), std::move(body));
}

language::nodes::base_ptr parser_c::parse_return_stmt() {
  const auto &return_token = consume_keyword(language::keywords_e::RETURN,
                                             "Expected 'return' keyword");

  std::optional<language::nodes::base_ptr> expression = std::nullopt;
  if (!check(token_type_e::SEMICOLON)) {
    expression = parse_expression();
  }

  consume(token_type_e::SEMICOLON, "Expected ';' after return statement");

  return std::make_unique<language::nodes::return_c>(return_token.source_index,
                                                     std::move(expression));
}

language::nodes::base_ptr parser_c::parse_break_stmt() {
  const auto &break_token =
      consume_keyword(language::keywords_e::BREAK, "Expected 'break' keyword");
  consume(token_type_e::SEMICOLON, "Expected ';' after break statement");
  return std::make_unique<language::nodes::break_c>(break_token.source_index);
}

language::nodes::base_ptr parser_c::parse_continue_stmt() {
  const auto &continue_token = consume_keyword(language::keywords_e::CONTINUE,
                                               "Expected 'continue' keyword");
  consume(token_type_e::SEMICOLON, "Expected ';' after continue statement");
  return std::make_unique<language::nodes::continue_c>(
      continue_token.source_index);
}

language::nodes::base_ptr parser_c::parse_expression_stmt() {
  auto expr = parse_expression();
  consume(token_type_e::SEMICOLON, "Expected ';' after expression");
  return expr;
}

language::nodes::base_ptr parser_c::parse_expression() {
  return parse_assignment();
}

language::nodes::base_ptr
clone_expr_for_compound_assignment(const language::nodes::base_c *expr) {
  if (auto *id = dynamic_cast<const language::nodes::identifier_c *>(expr)) {
    return std::make_unique<language::nodes::identifier_c>(
        id->source_index(),
        language::nodes::identifier_s(id->id().name, id->id().source_index));
  }

  if (auto *index = dynamic_cast<const language::nodes::index_c *>(expr)) {
    auto cloned_object = clone_expr_for_compound_assignment(index->object());
    auto cloned_index = clone_expr_for_compound_assignment(index->index());
    return std::make_unique<language::nodes::index_c>(index->source_index(),
                                                      std::move(cloned_object),
                                                      std::move(cloned_index));
  }

  if (auto *member =
          dynamic_cast<const language::nodes::member_access_c *>(expr)) {
    auto cloned_object = clone_expr_for_compound_assignment(member->object());
    return std::make_unique<language::nodes::member_access_c>(
        member->source_index(), std::move(cloned_object),
        language::nodes::identifier_s(member->field().name,
                                      member->field().source_index));
  }

  if (auto *unary = dynamic_cast<const language::nodes::unary_op_c *>(expr)) {
    if (unary->op() == language::nodes::unary_op_e::DEREF) {
      auto cloned_operand =
          clone_expr_for_compound_assignment(unary->operand());
      return std::make_unique<language::nodes::unary_op_c>(
          unary->source_index(), language::nodes::unary_op_e::DEREF,
          std::move(cloned_operand));
    }
  }

  return nullptr;
}

language::nodes::base_ptr parser_c::parse_assignment() {
  auto expr = parse_logical_or();

  if (match(token_type_e::EQUAL) || match(token_type_e::PLUS_EQUAL) ||
      match(token_type_e::MINUS_EQUAL) || match(token_type_e::STAR_EQUAL) ||
      match(token_type_e::SLASH_EQUAL) || match(token_type_e::PERCENT_EQUAL)) {
    const auto &op_token = previous();
    auto value = parse_assignment();

    if (op_token.type != token_type_e::EQUAL) {
      language::nodes::binary_op_e bin_op;
      switch (op_token.type) {
      case token_type_e::PLUS_EQUAL:
        bin_op = language::nodes::binary_op_e::ADD;
        break;
      case token_type_e::MINUS_EQUAL:
        bin_op = language::nodes::binary_op_e::SUB;
        break;
      case token_type_e::STAR_EQUAL:
        bin_op = language::nodes::binary_op_e::MUL;
        break;
      case token_type_e::SLASH_EQUAL:
        bin_op = language::nodes::binary_op_e::DIV;
        break;
      case token_type_e::PERCENT_EQUAL:
        bin_op = language::nodes::binary_op_e::MOD;
        break;
      default:
        bin_op = language::nodes::binary_op_e::ADD;
        break;
      }

      auto left_copy = clone_expr_for_compound_assignment(expr.get());
      if (!left_copy) {
        throw parse_error("Invalid left-hand side for compound assignment",
                          op_token.line, op_token.column);
      }

      value = std::make_unique<language::nodes::binary_op_c>(
          op_token.source_index, bin_op, std::move(left_copy),
          std::move(value));
    }

    return std::make_unique<language::nodes::assignment_c>(
        op_token.source_index, std::move(expr), std::move(value));
  }

  return expr;
}

language::nodes::base_ptr parser_c::parse_cast() {
  auto expr = parse_logical_or();

  while (check(token_type_e::KEYWORD)) {
    const auto &tok = peek();
    if (tok.keyword && tok.keyword.value() == language::keywords_e::AS) {
      advance();
      auto target_type = parse_type();
      expr = std::make_unique<language::nodes::cast_c>(
          tok.source_index, std::move(expr), std::move(target_type));
    } else {
      break;
    }
  }

  return expr;
}

language::nodes::base_ptr parser_c::parse_logical_or() {
  auto expr = parse_logical_and();

  while (match(token_type_e::PIPE_PIPE)) {
    const auto &op_token = previous();
    auto right = parse_logical_and();
    expr = std::make_unique<language::nodes::binary_op_c>(
        op_token.source_index, language::nodes::binary_op_e::OR,
        std::move(expr), std::move(right));
  }

  return expr;
}

language::nodes::base_ptr parser_c::parse_logical_and() {
  auto expr = parse_equality();

  while (match(token_type_e::AMP_AMP)) {
    const auto &op_token = previous();
    auto right = parse_equality();
    expr = std::make_unique<language::nodes::binary_op_c>(
        op_token.source_index, language::nodes::binary_op_e::AND,
        std::move(expr), std::move(right));
  }

  return expr;
}

language::nodes::base_ptr parser_c::parse_equality() {
  auto expr = parse_comparison();

  while (match(token_type_e::EQUAL_EQUAL) || match(token_type_e::BANG_EQUAL)) {
    const auto &op_token = previous();
    auto right = parse_comparison();

    language::nodes::binary_op_e op = op_token.type == token_type_e::EQUAL_EQUAL
                                          ? language::nodes::binary_op_e::EQ
                                          : language::nodes::binary_op_e::NE;

    expr = std::make_unique<language::nodes::binary_op_c>(
        op_token.source_index, op, std::move(expr), std::move(right));
  }

  return expr;
}

language::nodes::base_ptr parser_c::parse_comparison() {
  auto expr = parse_bitwise_or();

  while (match(token_type_e::LESS) || match(token_type_e::LESS_EQUAL) ||
         match(token_type_e::GREATER) || match(token_type_e::GREATER_EQUAL)) {
    const auto &op_token = previous();
    auto right = parse_bitwise_or();

    language::nodes::binary_op_e op;
    switch (op_token.type) {
    case token_type_e::LESS:
      op = language::nodes::binary_op_e::LT;
      break;
    case token_type_e::LESS_EQUAL:
      op = language::nodes::binary_op_e::LE;
      break;
    case token_type_e::GREATER:
      op = language::nodes::binary_op_e::GT;
      break;
    case token_type_e::GREATER_EQUAL:
      op = language::nodes::binary_op_e::GE;
      break;
    default:
      op = language::nodes::binary_op_e::LT;
      break;
    }

    expr = std::make_unique<language::nodes::binary_op_c>(
        op_token.source_index, op, std::move(expr), std::move(right));
  }

  return expr;
}

language::nodes::base_ptr parser_c::parse_bitwise_or() {
  auto expr = parse_bitwise_xor();

  while (match(token_type_e::PIPE)) {
    const auto &op_token = previous();
    auto right = parse_bitwise_xor();
    expr = std::make_unique<language::nodes::binary_op_c>(
        op_token.source_index, language::nodes::binary_op_e::BITWISE_OR,
        std::move(expr), std::move(right));
  }

  return expr;
}

language::nodes::base_ptr parser_c::parse_bitwise_xor() {
  auto expr = parse_bitwise_and();

  while (match(token_type_e::CARET)) {
    const auto &op_token = previous();
    auto right = parse_bitwise_and();
    expr = std::make_unique<language::nodes::binary_op_c>(
        op_token.source_index, language::nodes::binary_op_e::BITWISE_XOR,
        std::move(expr), std::move(right));
  }

  return expr;
}

language::nodes::base_ptr parser_c::parse_bitwise_and() {
  auto expr = parse_shift();

  while (match(token_type_e::AMP)) {
    const auto &op_token = previous();
    auto right = parse_shift();
    expr = std::make_unique<language::nodes::binary_op_c>(
        op_token.source_index, language::nodes::binary_op_e::BITWISE_AND,
        std::move(expr), std::move(right));
  }

  return expr;
}

language::nodes::base_ptr parser_c::parse_shift() {
  auto expr = parse_additive();

  while (match(token_type_e::LESS_LESS) ||
         match(token_type_e::GREATER_GREATER)) {
    const auto &op_token = previous();
    auto right = parse_additive();

    language::nodes::binary_op_e op =
        op_token.type == token_type_e::LESS_LESS
            ? language::nodes::binary_op_e::LEFT_SHIFT
            : language::nodes::binary_op_e::RIGHT_SHIFT;

    expr = std::make_unique<language::nodes::binary_op_c>(
        op_token.source_index, op, std::move(expr), std::move(right));
  }

  return expr;
}

language::nodes::base_ptr parser_c::parse_additive() {
  auto expr = parse_multiplicative();

  while (match(token_type_e::PLUS) || match(token_type_e::MINUS)) {
    const auto &op_token = previous();
    auto right = parse_multiplicative();

    language::nodes::binary_op_e op = op_token.type == token_type_e::PLUS
                                          ? language::nodes::binary_op_e::ADD
                                          : language::nodes::binary_op_e::SUB;

    expr = std::make_unique<language::nodes::binary_op_c>(
        op_token.source_index, op, std::move(expr), std::move(right));
  }

  return expr;
}

language::nodes::base_ptr parser_c::parse_multiplicative() {
  auto expr = parse_unary();

  while (match(token_type_e::STAR) || match(token_type_e::SLASH) ||
         match(token_type_e::PERCENT)) {
    const auto &op_token = previous();
    auto right = parse_unary();

    language::nodes::binary_op_e op;
    switch (op_token.type) {
    case token_type_e::STAR:
      op = language::nodes::binary_op_e::MUL;
      break;
    case token_type_e::SLASH:
      op = language::nodes::binary_op_e::DIV;
      break;
    case token_type_e::PERCENT:
      op = language::nodes::binary_op_e::MOD;
      break;
    default:
      op = language::nodes::binary_op_e::MUL;
      break;
    }

    expr = std::make_unique<language::nodes::binary_op_c>(
        op_token.source_index, op, std::move(expr), std::move(right));
  }

  return expr;
}

language::nodes::base_ptr parser_c::parse_unary() {
  if (match(token_type_e::BANG) || match(token_type_e::MINUS) ||
      match(token_type_e::TILDE) || match(token_type_e::AMP) ||
      match(token_type_e::STAR)) {
    const auto &op_token = previous();
    auto operand = parse_unary();

    language::nodes::unary_op_e op;
    switch (op_token.type) {
    case token_type_e::BANG:
      op = language::nodes::unary_op_e::NOT;
      break;
    case token_type_e::MINUS:
      op = language::nodes::unary_op_e::NEG;
      break;
    case token_type_e::TILDE:
      op = language::nodes::unary_op_e::BITWISE_NOT;
      break;
    case token_type_e::AMP:
      op = language::nodes::unary_op_e::ADDRESS_OF;
      break;
    case token_type_e::STAR:
      op = language::nodes::unary_op_e::DEREF;
      break;
    default:
      op = language::nodes::unary_op_e::NOT;
      break;
    }

    return std::make_unique<language::nodes::unary_op_c>(
        op_token.source_index, op, std::move(operand));
  }

  return parse_postfix();
}

language::nodes::base_ptr parser_c::parse_postfix() {
  auto expr = parse_primary();

  while (true) {
    if (match(token_type_e::LEFT_PAREN)) {
      const auto &paren_token = previous();
      std::vector<language::nodes::base_ptr> arguments;
      if (!check(token_type_e::RIGHT_PAREN)) {
        arguments = parse_argument_list();
      }
      consume(token_type_e::RIGHT_PAREN, "Expected ')' after arguments");
      expr = std::make_unique<language::nodes::call_c>(
          paren_token.source_index, std::move(expr), std::move(arguments));
    } else if (match(token_type_e::LEFT_BRACKET)) {
      const auto &bracket_token = previous();
      auto index = parse_expression();
      consume(token_type_e::RIGHT_BRACKET, "Expected ']' after index");
      expr = std::make_unique<language::nodes::index_c>(
          bracket_token.source_index, std::move(expr), std::move(index));
    } else if (match(token_type_e::DOT)) {
      const auto &dot_token = previous();
      const auto &field_token =
          consume_identifier("Expected field name after '.'");
      language::nodes::identifier_s field(field_token.lexeme,
                                          field_token.source_index);
      expr = std::make_unique<language::nodes::member_access_c>(
          dot_token.source_index, std::move(expr), std::move(field));
    } else if (check(token_type_e::KEYWORD)) {
      const auto &tok = peek();
      if (tok.keyword && tok.keyword.value() == language::keywords_e::AS) {
        advance();
        auto target_type = parse_type();
        expr = std::make_unique<language::nodes::cast_c>(
            tok.source_index, std::move(expr), std::move(target_type));
      } else {
        break;
      }
    } else {
      break;
    }
  }

  return expr;
}

language::nodes::base_ptr parser_c::parse_primary() {
  if (match(token_type_e::INTEGER_LITERAL)) {
    const auto &token = previous();
    return std::make_unique<language::nodes::literal_c>(
        token.source_index, language::nodes::literal_type_e::INTEGER,
        token.lexeme);
  }

  if (match(token_type_e::FLOAT_LITERAL)) {
    const auto &token = previous();
    return std::make_unique<language::nodes::literal_c>(
        token.source_index, language::nodes::literal_type_e::FLOAT,
        token.lexeme);
  }

  if (match(token_type_e::STRING_LITERAL)) {
    const auto &token = previous();
    return std::make_unique<language::nodes::literal_c>(
        token.source_index, language::nodes::literal_type_e::STRING,
        token.lexeme);
  }

  if (match_keyword(language::keywords_e::TRUE)) {
    const auto &token = previous();
    return std::make_unique<language::nodes::literal_c>(
        token.source_index, language::nodes::literal_type_e::BOOL, "true");
  }

  if (match_keyword(language::keywords_e::FALSE)) {
    const auto &token = previous();
    return std::make_unique<language::nodes::literal_c>(
        token.source_index, language::nodes::literal_type_e::BOOL, "false");
  }

  if (match_keyword(language::keywords_e::NIL)) {
    const auto &token = previous();
    return std::make_unique<language::nodes::literal_c>(
        token.source_index, language::nodes::literal_type_e::NIL, "nil");
  }

  if (check(token_type_e::IDENTIFIER)) {
    const auto &token = peek();
    std::size_t saved_pos = _current;
    advance();

    if (check(token_type_e::LEFT_BRACE)) {
      advance();
      bool is_struct_literal = false;
      if (check(token_type_e::RIGHT_BRACE)) {
        advance();
        if (check(token_type_e::SEMICOLON) || check(token_type_e::COMMA) ||
            check(token_type_e::RIGHT_PAREN) ||
            check(token_type_e::RIGHT_BRACKET) ||
            check(token_type_e::RIGHT_BRACE) || is_at_end()) {
          is_struct_literal = true;
        }
        _current = saved_pos + 1;
      } else if (check(token_type_e::IDENTIFIER)) {
        advance();
        if (check(token_type_e::COLON) || check(token_type_e::RIGHT_BRACE) ||
            check(token_type_e::COMMA)) {
          is_struct_literal = true;
        }
      }

      _current = saved_pos;
      if (is_struct_literal) {
        return parse_struct_literal();
      }
    } else {
      _current = saved_pos;
    }

    const auto &id_token = advance();
    language::nodes::identifier_s id(id_token.lexeme, id_token.source_index);
    return std::make_unique<language::nodes::identifier_c>(
        id_token.source_index, std::move(id));
  }

  if (match(token_type_e::LEFT_PAREN)) {
    auto expr = parse_expression();
    consume(token_type_e::RIGHT_PAREN, "Expected ')' after expression");
    return expr;
  }

  if (check(token_type_e::LEFT_BRACKET)) {
    return parse_array_literal();
  }

  const auto &token = peek();
  throw parse_error("Expected expression", token.line, token.column);
}

std::vector<language::nodes::parameter_s> parser_c::parse_param_list() {
  std::vector<language::nodes::parameter_s> params;
  params.push_back(parse_param());

  if (params[0].is_variadic) {
    const auto &token = peek();
    throw parse_error("Variadic parameter cannot be the only parameter",
                      token.line, token.column);
  }

  while (match(token_type_e::COMMA)) {
    if (params.back().is_variadic) {
      const auto &token = peek();
      throw parse_error("Variadic parameter must be the last parameter",
                        token.line, token.column);
    }
    params.push_back(parse_param());
  }

  return params;
}

language::nodes::parameter_s parser_c::parse_param() {
  bool is_variadic = false;
  if (match(token_type_e::DOT_DOT_DOT)) {
    is_variadic = true;
  }

  const auto &name_token = consume_identifier("Expected parameter name");
  language::nodes::identifier_s name(name_token.lexeme,
                                     name_token.source_index);

  language::nodes::type_ptr type;
  if (is_variadic) {
    auto void_type = std::make_unique<language::nodes::primitive_type_c>(
        language::keywords_e::VOID, name_token.source_index);
    type = std::make_unique<language::nodes::array_type_c>(
        name_token.source_index, std::move(void_type), std::nullopt);
  } else {
    type = parse_type_annotation();
  }

  return language::nodes::parameter_s(std::move(name), std::move(type),
                                      is_variadic);
}

std::vector<language::nodes::struct_field_s> parser_c::parse_field_list() {
  std::vector<language::nodes::struct_field_s> fields;
  fields.push_back(parse_field());

  while (match(token_type_e::COMMA)) {
    if (check(token_type_e::RIGHT_BRACE)) {
      break;
    }
    fields.push_back(parse_field());
  }

  return fields;
}

language::nodes::struct_field_s parser_c::parse_field() {
  const auto &name_token = consume_identifier("Expected field name");
  language::nodes::identifier_s name(name_token.lexeme,
                                     name_token.source_index);

  auto type = parse_type_annotation();

  return language::nodes::struct_field_s(std::move(name), std::move(type));
}

std::vector<language::nodes::base_ptr> parser_c::parse_argument_list() {
  std::vector<language::nodes::base_ptr> arguments;

  if (match(token_type_e::AT)) {
    const auto &at_token = previous();
    auto type = parse_type_internal();
    arguments.push_back(std::make_unique<language::nodes::type_param_c>(
        at_token.source_index, std::move(type)));

    while (match(token_type_e::COMMA)) {
      if (match(token_type_e::AT)) {
        const auto &next_at_token = previous();
        auto next_type = parse_type_internal();
        arguments.push_back(std::make_unique<language::nodes::type_param_c>(
            next_at_token.source_index, std::move(next_type)));
      } else {
        arguments.push_back(parse_expression());
      }
    }
  } else {
    arguments.push_back(parse_expression());

    while (match(token_type_e::COMMA)) {
      if (match(token_type_e::AT)) {
        const auto &at_token = previous();
        auto type = parse_type_internal();
        arguments.push_back(std::make_unique<language::nodes::type_param_c>(
            at_token.source_index, std::move(type)));
      } else {
        arguments.push_back(parse_expression());
      }
    }
  }

  return arguments;
}

language::nodes::base_ptr parser_c::parse_array_literal() {
  const auto &bracket_token =
      consume(token_type_e::LEFT_BRACKET, "Expected '['");

  std::vector<language::nodes::base_ptr> elements;
  if (!check(token_type_e::RIGHT_BRACKET)) {
    elements.push_back(parse_expression());

    while (match(token_type_e::COMMA)) {
      if (check(token_type_e::RIGHT_BRACKET)) {
        break;
      }
      elements.push_back(parse_expression());
    }
  }

  consume(token_type_e::RIGHT_BRACKET, "Expected ']' after array elements");

  return std::make_unique<language::nodes::array_literal_c>(
      bracket_token.source_index, std::move(elements));
}

language::nodes::base_ptr parser_c::parse_struct_literal() {
  const auto &name_token = consume_identifier("Expected struct name");
  language::nodes::identifier_s struct_name(name_token.lexeme,
                                            name_token.source_index);

  consume(token_type_e::LEFT_BRACE,
          "Expected '{' after struct name in literal");

  std::vector<language::nodes::field_initializer_s> field_inits;
  if (!check(token_type_e::RIGHT_BRACE)) {
    const auto &field_name_token = consume_identifier("Expected field name");
    consume(token_type_e::COLON, "Expected ':' after field name");
    auto value = parse_expression();

    language::nodes::identifier_s field_name(field_name_token.lexeme,
                                             field_name_token.source_index);
    field_inits.push_back(language::nodes::field_initializer_s(
        std::move(field_name), std::move(value)));

    while (match(token_type_e::COMMA)) {
      if (check(token_type_e::RIGHT_BRACE)) {
        break;
      }
      const auto &fn_token = consume_identifier("Expected field name");
      consume(token_type_e::COLON, "Expected ':' after field name");
      auto val = parse_expression();

      language::nodes::identifier_s fn(fn_token.lexeme, fn_token.source_index);
      field_inits.push_back(
          language::nodes::field_initializer_s(std::move(fn), std::move(val)));
    }
  }

  consume(token_type_e::RIGHT_BRACE,
          "Expected '}' after struct literal fields");

  return std::make_unique<language::nodes::struct_literal_c>(
      name_token.source_index, std::move(struct_name), std::move(field_inits));
}

} // namespace truk::ingestion
