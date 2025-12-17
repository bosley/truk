#pragma once

#include <language/keywords.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace truk::ingestion {

enum class token_type_e {
  KEYWORD,
  IDENTIFIER,
  INTEGER_LITERAL,
  FLOAT_LITERAL,
  STRING_LITERAL,
  PLUS,
  MINUS,
  STAR,
  SLASH,
  PERCENT,
  EQUAL,
  EQUAL_EQUAL,
  BANG_EQUAL,
  LESS,
  LESS_EQUAL,
  GREATER,
  GREATER_EQUAL,
  AMP_AMP,
  PIPE_PIPE,
  BANG,
  AMP,
  PIPE,
  CARET,
  TILDE,
  LESS_LESS,
  GREATER_GREATER,
  PLUS_EQUAL,
  MINUS_EQUAL,
  STAR_EQUAL,
  SLASH_EQUAL,
  PERCENT_EQUAL,
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  LEFT_BRACKET,
  RIGHT_BRACKET,
  COMMA,
  SEMICOLON,
  COLON,
  DOT,
  ARROW,
  DOT_DOT_DOT,
  AT,
  END_OF_FILE,
  UNKNOWN
};

struct token_s {
  token_type_e type;
  std::string lexeme;
  std::size_t line;
  std::size_t column;
  std::size_t source_index;
  std::optional<language::keywords_e> keyword;

  token_s() = delete;
  token_s(token_type_e t, std::string lex, std::size_t ln, std::size_t col,
          std::size_t idx,
          std::optional<language::keywords_e> kw = std::nullopt)
      : type(t), lexeme(std::move(lex)), line(ln), column(col),
        source_index(idx), keyword(kw) {}
};

class tokenizer_c {
public:
  tokenizer_c() = delete;
  tokenizer_c(const char *data, std::size_t len);
  ~tokenizer_c();

  std::optional<token_s> next_token();
  std::optional<token_s> peek_token();

private:
  const char *_data{nullptr};
  std::size_t _len{0};
  std::size_t _pos{0};
  std::size_t _line{1};
  std::size_t _column{1};
  std::optional<token_s> _peeked_token;

  char current_char() const;
  char peek_char(std::size_t offset = 1) const;
  void advance();
  void skip_whitespace();
  void skip_line_comment();
  void skip_block_comment();
  bool is_at_end() const;
  bool is_digit(char c) const;
  bool is_alpha(char c) const;
  bool is_alphanumeric(char c) const;

  token_s make_token(token_type_e type, std::size_t start_pos,
                     std::size_t start_line, std::size_t start_column);
  token_s tokenize_number(std::size_t start_line, std::size_t start_column);
  token_s tokenize_string(std::size_t start_line, std::size_t start_column);
  token_s tokenize_identifier(std::size_t start_line, std::size_t start_column);
};

} // namespace truk::ingestion
