#include <truk/ingestion/tokenize.hpp>

namespace truk::ingestion {

tokenizer_c::tokenizer_c(const char *data, std::size_t len)
    : _data(data), _len(len) {}

tokenizer_c::~tokenizer_c() = default;

char tokenizer_c::current_char() const {
  if (is_at_end()) {
    return '\0';
  }
  return _data[_pos];
}

char tokenizer_c::peek_char(std::size_t offset) const {
  if (_pos + offset >= _len) {
    return '\0';
  }
  return _data[_pos + offset];
}

void tokenizer_c::advance() {
  if (is_at_end()) {
    return;
  }
  if (_data[_pos] == '\n') {
    _line++;
    _column = 1;
  } else {
    _column++;
  }
  _pos++;
}

void tokenizer_c::skip_whitespace() {
  while (!is_at_end()) {
    char c = current_char();
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
      advance();
    } else {
      break;
    }
  }
}

void tokenizer_c::skip_line_comment() {
  while (!is_at_end() && current_char() != '\n') {
    advance();
  }
}

void tokenizer_c::skip_block_comment() {
  advance();
  advance();
  while (!is_at_end()) {
    if (current_char() == '*' && peek_char() == '/') {
      advance();
      advance();
      break;
    }
    advance();
  }
}

bool tokenizer_c::is_at_end() const { return _pos >= _len; }

bool tokenizer_c::is_digit(char c) const { return c >= '0' && c <= '9'; }

bool tokenizer_c::is_alpha(char c) const {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool tokenizer_c::is_alphanumeric(char c) const {
  return is_alpha(c) || is_digit(c);
}

token_s tokenizer_c::make_token(token_type_e type, std::size_t start_pos,
                                std::size_t start_line,
                                std::size_t start_column) {
  std::string lexeme(_data + start_pos, _pos - start_pos);
  return token_s(type, lexeme, start_line, start_column, start_pos);
}

token_s tokenizer_c::tokenize_number(std::size_t start_line,
                                     std::size_t start_column) {
  std::size_t start_pos = _pos;
  bool is_float = false;

  if (current_char() == '0' && (peek_char() == 'x' || peek_char() == 'X')) {
    advance();
    advance();
    while (!is_at_end() && ((current_char() >= '0' && current_char() <= '9') ||
                            (current_char() >= 'a' && current_char() <= 'f') ||
                            (current_char() >= 'A' && current_char() <= 'F'))) {
      advance();
    }
  } else if (current_char() == '0' &&
             (peek_char() == 'b' || peek_char() == 'B')) {
    advance();
    advance();
    while (!is_at_end() && (current_char() == '0' || current_char() == '1')) {
      advance();
    }
  } else if (current_char() == '0' &&
             (peek_char() == 'o' || peek_char() == 'O')) {
    advance();
    advance();
    while (!is_at_end() && current_char() >= '0' && current_char() <= '7') {
      advance();
    }
  } else {
    while (!is_at_end() && is_digit(current_char())) {
      advance();
    }

    if (!is_at_end() && current_char() == '.' && is_digit(peek_char())) {
      is_float = true;
      advance();
      while (!is_at_end() && is_digit(current_char())) {
        advance();
      }
    }

    if (!is_at_end() && (current_char() == 'e' || current_char() == 'E')) {
      is_float = true;
      advance();
      if (!is_at_end() && (current_char() == '+' || current_char() == '-')) {
        advance();
      }
      while (!is_at_end() && is_digit(current_char())) {
        advance();
      }
    }
  }

  return make_token(is_float ? token_type_e::FLOAT_LITERAL
                             : token_type_e::INTEGER_LITERAL,
                    start_pos, start_line, start_column);
}

token_s tokenizer_c::tokenize_string(std::size_t start_line,
                                     std::size_t start_column) {
  std::size_t start_pos = _pos;
  advance();

  while (!is_at_end() && current_char() != '"') {
    if (current_char() == '\\') {
      advance();
      if (!is_at_end()) {
        advance();
      }
    } else {
      advance();
    }
  }

  if (!is_at_end()) {
    advance();
  }

  return make_token(token_type_e::STRING_LITERAL, start_pos, start_line,
                    start_column);
}

token_s tokenizer_c::tokenize_identifier(std::size_t start_line,
                                         std::size_t start_column) {
  std::size_t start_pos = _pos;

  while (!is_at_end() && is_alphanumeric(current_char())) {
    advance();
  }

  std::string lexeme(_data + start_pos, _pos - start_pos);
  auto keyword_opt = language::keywords_c::from_string(lexeme);

  if (keyword_opt.has_value()) {
    return token_s(token_type_e::KEYWORD, lexeme, start_line, start_column,
                   start_pos, keyword_opt.value());
  }

  return token_s(token_type_e::IDENTIFIER, lexeme, start_line, start_column,
                 start_pos);
}

std::optional<token_s> tokenizer_c::next_token() {
  if (_peeked_token.has_value()) {
    auto token = _peeked_token.value();
    _peeked_token = std::nullopt;
    return token;
  }

  skip_whitespace();

  if (is_at_end()) {
    return token_s(token_type_e::END_OF_FILE, "", _line, _column, _pos);
  }

  while (!is_at_end() && current_char() == '/') {
    if (peek_char() == '/') {
      skip_line_comment();
      skip_whitespace();
      if (is_at_end()) {
        return token_s(token_type_e::END_OF_FILE, "", _line, _column, _pos);
      }
    } else if (peek_char() == '*') {
      skip_block_comment();
      skip_whitespace();
      if (is_at_end()) {
        return token_s(token_type_e::END_OF_FILE, "", _line, _column, _pos);
      }
    } else {
      break;
    }
  }

  std::size_t start_line = _line;
  std::size_t start_column = _column;
  std::size_t start_pos = _pos;
  char c = current_char();

  if (is_digit(c)) {
    return tokenize_number(start_line, start_column);
  }

  if (is_alpha(c)) {
    return tokenize_identifier(start_line, start_column);
  }

  if (c == '"') {
    return tokenize_string(start_line, start_column);
  }

  switch (c) {
  case '+':
    advance();
    if (!is_at_end() && current_char() == '=') {
      advance();
      return make_token(token_type_e::PLUS_EQUAL, start_pos, start_line,
                        start_column);
    }
    return make_token(token_type_e::PLUS, start_pos, start_line, start_column);

  case '-':
    advance();
    if (!is_at_end() && current_char() == '>') {
      advance();
      return make_token(token_type_e::ARROW, start_pos, start_line,
                        start_column);
    }
    if (!is_at_end() && current_char() == '=') {
      advance();
      return make_token(token_type_e::MINUS_EQUAL, start_pos, start_line,
                        start_column);
    }
    return make_token(token_type_e::MINUS, start_pos, start_line, start_column);

  case '*':
    advance();
    if (!is_at_end() && current_char() == '=') {
      advance();
      return make_token(token_type_e::STAR_EQUAL, start_pos, start_line,
                        start_column);
    }
    return make_token(token_type_e::STAR, start_pos, start_line, start_column);

  case '/':
    advance();
    if (!is_at_end() && current_char() == '=') {
      advance();
      return make_token(token_type_e::SLASH_EQUAL, start_pos, start_line,
                        start_column);
    }
    return make_token(token_type_e::SLASH, start_pos, start_line, start_column);

  case '%':
    advance();
    if (!is_at_end() && current_char() == '=') {
      advance();
      return make_token(token_type_e::PERCENT_EQUAL, start_pos, start_line,
                        start_column);
    }
    return make_token(token_type_e::PERCENT, start_pos, start_line,
                      start_column);

  case '=':
    advance();
    if (!is_at_end() && current_char() == '=') {
      advance();
      return make_token(token_type_e::EQUAL_EQUAL, start_pos, start_line,
                        start_column);
    }
    if (!is_at_end() && current_char() == '>') {
      advance();
      return make_token(token_type_e::FAT_ARROW, start_pos, start_line,
                        start_column);
    }
    return make_token(token_type_e::EQUAL, start_pos, start_line, start_column);

  case '!':
    advance();
    if (!is_at_end() && current_char() == '=') {
      advance();
      return make_token(token_type_e::BANG_EQUAL, start_pos, start_line,
                        start_column);
    }
    return make_token(token_type_e::BANG, start_pos, start_line, start_column);

  case '<':
    advance();
    if (!is_at_end() && current_char() == '=') {
      advance();
      return make_token(token_type_e::LESS_EQUAL, start_pos, start_line,
                        start_column);
    }
    if (!is_at_end() && current_char() == '<') {
      advance();
      return make_token(token_type_e::LESS_LESS, start_pos, start_line,
                        start_column);
    }
    return make_token(token_type_e::LESS, start_pos, start_line, start_column);

  case '>':
    advance();
    if (!is_at_end() && current_char() == '=') {
      advance();
      return make_token(token_type_e::GREATER_EQUAL, start_pos, start_line,
                        start_column);
    }
    if (!is_at_end() && current_char() == '>') {
      advance();
      return make_token(token_type_e::GREATER_GREATER, start_pos, start_line,
                        start_column);
    }
    return make_token(token_type_e::GREATER, start_pos, start_line,
                      start_column);

  case '&':
    advance();
    if (!is_at_end() && current_char() == '&') {
      advance();
      return make_token(token_type_e::AMP_AMP, start_pos, start_line,
                        start_column);
    }
    return make_token(token_type_e::AMP, start_pos, start_line, start_column);

  case '|':
    advance();
    if (!is_at_end() && current_char() == '|') {
      advance();
      return make_token(token_type_e::PIPE_PIPE, start_pos, start_line,
                        start_column);
    }
    return make_token(token_type_e::PIPE, start_pos, start_line, start_column);

  case '^':
    advance();
    return make_token(token_type_e::CARET, start_pos, start_line, start_column);

  case '~':
    advance();
    return make_token(token_type_e::TILDE, start_pos, start_line, start_column);

  case '(':
    advance();
    return make_token(token_type_e::LEFT_PAREN, start_pos, start_line,
                      start_column);

  case ')':
    advance();
    return make_token(token_type_e::RIGHT_PAREN, start_pos, start_line,
                      start_column);

  case '{':
    advance();
    return make_token(token_type_e::LEFT_BRACE, start_pos, start_line,
                      start_column);

  case '}':
    advance();
    return make_token(token_type_e::RIGHT_BRACE, start_pos, start_line,
                      start_column);

  case '[':
    advance();
    return make_token(token_type_e::LEFT_BRACKET, start_pos, start_line,
                      start_column);

  case ']':
    advance();
    return make_token(token_type_e::RIGHT_BRACKET, start_pos, start_line,
                      start_column);

  case ',':
    advance();
    return make_token(token_type_e::COMMA, start_pos, start_line, start_column);

  case ';':
    advance();
    return make_token(token_type_e::SEMICOLON, start_pos, start_line,
                      start_column);

  case ':':
    advance();
    return make_token(token_type_e::COLON, start_pos, start_line, start_column);

  case '.':
    advance();
    if (current_char() == '.' && peek_char() == '.') {
      advance();
      advance();
      return make_token(token_type_e::DOT_DOT_DOT, start_pos, start_line,
                        start_column);
    }
    return make_token(token_type_e::DOT, start_pos, start_line, start_column);

  case '@':
    advance();
    return make_token(token_type_e::AT, start_pos, start_line, start_column);

  default:
    advance();
    return make_token(token_type_e::UNKNOWN, start_pos, start_line,
                      start_column);
  }
}

std::optional<token_s> tokenizer_c::peek_token() {
  if (!_peeked_token.has_value()) {
    _peeked_token = next_token();
  }
  return _peeked_token;
}

} // namespace truk::ingestion
