#pragma once

#include <optional>
#include <string>

namespace truk::language {

enum class keywords_e {
  UNKNOWN_KEYWORD = 0, // not a keyword
  FN,
  STRUCT,
  VAR,
  CONST,
  IF,
  ELSE,
  WHILE,
  FOR,
  IN,
  RETURN,
  BREAK,
  CONTINUE,
  TRUE,
  FALSE,
  NIL,
  I8,
  I16,
  I32,
  I64,
  U8,
  U16,
  U32,
  U64,
  F32,
  F64,
  BOOL,
  VOID
};

class keywords_c {
public:
  static std::optional<keywords_e> from_string(const std::string &str);
  static std::string to_string(keywords_e keyword);
};

} // namespace truk::language
