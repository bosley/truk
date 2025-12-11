// clang-format off
#include <truk/ingestion/ingestion.hpp>
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
// clang-format on

TEST_GROUP(IngestionTests){void setup() override{}

                           void teardown() override{}};

TEST(IngestionTests, CanConstruct) {
  const char *source = "fn main() {}";
  truk::ingestion::parser_c parser(source, 12);
  CHECK_TRUE(true);
}

TEST(IngestionTests, TokenizeSimpleFunction) {
  const char *source = "fn main() {}";
  truk::ingestion::parser_c parser(source, 12);
  auto tokens = parser.tokenize();

  CHECK_EQUAL(7, tokens.size());

  CHECK_TRUE(tokens[0].type == truk::ingestion::token_type_e::KEYWORD);
  STRCMP_EQUAL("fn", tokens[0].lexeme.c_str());
  CHECK_TRUE(tokens[0].keyword.has_value());
  CHECK_TRUE(tokens[0].keyword.value() == truk::language::keywords_e::FN);
  CHECK_EQUAL(0, tokens[0].source_index);
  CHECK_EQUAL(1, tokens[0].line);
  CHECK_EQUAL(1, tokens[0].column);

  CHECK_TRUE(tokens[1].type == truk::ingestion::token_type_e::IDENTIFIER);
  STRCMP_EQUAL("main", tokens[1].lexeme.c_str());
  CHECK_FALSE(tokens[1].keyword.has_value());
  CHECK_EQUAL(3, tokens[1].source_index);
  CHECK_EQUAL(1, tokens[1].line);
  CHECK_EQUAL(4, tokens[1].column);

  CHECK_TRUE(tokens[2].type == truk::ingestion::token_type_e::LEFT_PAREN);
  STRCMP_EQUAL("(", tokens[2].lexeme.c_str());
  CHECK_EQUAL(7, tokens[2].source_index);
  CHECK_EQUAL(1, tokens[2].line);
  CHECK_EQUAL(8, tokens[2].column);

  CHECK_TRUE(tokens[3].type == truk::ingestion::token_type_e::RIGHT_PAREN);
  STRCMP_EQUAL(")", tokens[3].lexeme.c_str());
  CHECK_EQUAL(8, tokens[3].source_index);
  CHECK_EQUAL(1, tokens[3].line);
  CHECK_EQUAL(9, tokens[3].column);

  CHECK_TRUE(tokens[4].type == truk::ingestion::token_type_e::LEFT_BRACE);
  STRCMP_EQUAL("{", tokens[4].lexeme.c_str());
  CHECK_EQUAL(10, tokens[4].source_index);
  CHECK_EQUAL(1, tokens[4].line);
  CHECK_EQUAL(11, tokens[4].column);

  CHECK_TRUE(tokens[5].type == truk::ingestion::token_type_e::RIGHT_BRACE);
  STRCMP_EQUAL("}", tokens[5].lexeme.c_str());
  CHECK_EQUAL(11, tokens[5].source_index);
  CHECK_EQUAL(1, tokens[5].line);
  CHECK_EQUAL(12, tokens[5].column);

  CHECK_TRUE(tokens[6].type == truk::ingestion::token_type_e::END_OF_FILE);
  STRCMP_EQUAL("", tokens[6].lexeme.c_str());
  CHECK_EQUAL(12, tokens[6].source_index);
  CHECK_EQUAL(1, tokens[6].line);
  CHECK_EQUAL(13, tokens[6].column);
}

TEST(IngestionTests, TokenizeNumbers) {
  const char *source = "123 0x1A 0b101 0o77 3.14 2.5e10";
  truk::ingestion::parser_c parser(source, 31);
  auto tokens = parser.tokenize();

  CHECK_EQUAL(7, tokens.size());

  CHECK_TRUE(tokens[0].type == truk::ingestion::token_type_e::INTEGER_LITERAL);
  STRCMP_EQUAL("123", tokens[0].lexeme.c_str());
  CHECK_EQUAL(0, tokens[0].source_index);
  CHECK_EQUAL(1, tokens[0].line);
  CHECK_EQUAL(1, tokens[0].column);

  CHECK_TRUE(tokens[1].type == truk::ingestion::token_type_e::INTEGER_LITERAL);
  STRCMP_EQUAL("0x1A", tokens[1].lexeme.c_str());
  CHECK_EQUAL(4, tokens[1].source_index);
  CHECK_EQUAL(1, tokens[1].line);
  CHECK_EQUAL(5, tokens[1].column);

  CHECK_TRUE(tokens[2].type == truk::ingestion::token_type_e::INTEGER_LITERAL);
  STRCMP_EQUAL("0b101", tokens[2].lexeme.c_str());
  CHECK_EQUAL(9, tokens[2].source_index);
  CHECK_EQUAL(1, tokens[2].line);
  CHECK_EQUAL(10, tokens[2].column);

  CHECK_TRUE(tokens[3].type == truk::ingestion::token_type_e::INTEGER_LITERAL);
  STRCMP_EQUAL("0o77", tokens[3].lexeme.c_str());
  CHECK_EQUAL(15, tokens[3].source_index);
  CHECK_EQUAL(1, tokens[3].line);
  CHECK_EQUAL(16, tokens[3].column);

  CHECK_TRUE(tokens[4].type == truk::ingestion::token_type_e::FLOAT_LITERAL);
  STRCMP_EQUAL("3.14", tokens[4].lexeme.c_str());
  CHECK_EQUAL(20, tokens[4].source_index);
  CHECK_EQUAL(1, tokens[4].line);
  CHECK_EQUAL(21, tokens[4].column);

  CHECK_TRUE(tokens[5].type == truk::ingestion::token_type_e::FLOAT_LITERAL);
  STRCMP_EQUAL("2.5e10", tokens[5].lexeme.c_str());
  CHECK_EQUAL(25, tokens[5].source_index);
  CHECK_EQUAL(1, tokens[5].line);
  CHECK_EQUAL(26, tokens[5].column);

  CHECK_TRUE(tokens[6].type == truk::ingestion::token_type_e::END_OF_FILE);
  CHECK_EQUAL(31, tokens[6].source_index);
  CHECK_EQUAL(1, tokens[6].line);
  CHECK_EQUAL(32, tokens[6].column);
}

TEST(IngestionTests, TokenizeStrings) {
  const char *source = R"("hello world" "escaped \"quote\"")";
  truk::ingestion::parser_c parser(source, 33);
  auto tokens = parser.tokenize();

  CHECK_EQUAL(3, tokens.size());

  CHECK_TRUE(tokens[0].type == truk::ingestion::token_type_e::STRING_LITERAL);
  STRCMP_EQUAL("\"hello world\"", tokens[0].lexeme.c_str());
  CHECK_EQUAL(0, tokens[0].source_index);
  CHECK_EQUAL(1, tokens[0].line);
  CHECK_EQUAL(1, tokens[0].column);

  CHECK_TRUE(tokens[1].type == truk::ingestion::token_type_e::STRING_LITERAL);
  STRCMP_EQUAL("\"escaped \\\"quote\\\"\"", tokens[1].lexeme.c_str());
  CHECK_EQUAL(14, tokens[1].source_index);
  CHECK_EQUAL(1, tokens[1].line);
  CHECK_EQUAL(15, tokens[1].column);

  CHECK_TRUE(tokens[2].type == truk::ingestion::token_type_e::END_OF_FILE);
  CHECK_EQUAL(33, tokens[2].source_index);
  CHECK_EQUAL(1, tokens[2].line);
  CHECK_EQUAL(34, tokens[2].column);
}

TEST(IngestionTests, TokenizeOperators) {
  const char *source = "+ - * / % == != < <= > >= && || ! & | ^ ~ << >>";
  truk::ingestion::parser_c parser(source, 47);
  auto tokens = parser.tokenize();

  CHECK_EQUAL(21, tokens.size());

  CHECK_TRUE(tokens[0].type == truk::ingestion::token_type_e::PLUS);
  STRCMP_EQUAL("+", tokens[0].lexeme.c_str());
  CHECK_EQUAL(0, tokens[0].source_index);
  CHECK_EQUAL(1, tokens[0].line);
  CHECK_EQUAL(1, tokens[0].column);

  CHECK_TRUE(tokens[1].type == truk::ingestion::token_type_e::MINUS);
  STRCMP_EQUAL("-", tokens[1].lexeme.c_str());
  CHECK_EQUAL(2, tokens[1].source_index);
  CHECK_EQUAL(1, tokens[1].line);
  CHECK_EQUAL(3, tokens[1].column);

  CHECK_TRUE(tokens[2].type == truk::ingestion::token_type_e::STAR);
  STRCMP_EQUAL("*", tokens[2].lexeme.c_str());
  CHECK_EQUAL(4, tokens[2].source_index);
  CHECK_EQUAL(1, tokens[2].line);
  CHECK_EQUAL(5, tokens[2].column);

  CHECK_TRUE(tokens[3].type == truk::ingestion::token_type_e::SLASH);
  STRCMP_EQUAL("/", tokens[3].lexeme.c_str());
  CHECK_EQUAL(6, tokens[3].source_index);
  CHECK_EQUAL(1, tokens[3].line);
  CHECK_EQUAL(7, tokens[3].column);

  CHECK_TRUE(tokens[4].type == truk::ingestion::token_type_e::PERCENT);
  STRCMP_EQUAL("%", tokens[4].lexeme.c_str());
  CHECK_EQUAL(8, tokens[4].source_index);
  CHECK_EQUAL(1, tokens[4].line);
  CHECK_EQUAL(9, tokens[4].column);

  CHECK_TRUE(tokens[5].type == truk::ingestion::token_type_e::EQUAL_EQUAL);
  STRCMP_EQUAL("==", tokens[5].lexeme.c_str());
  CHECK_EQUAL(10, tokens[5].source_index);
  CHECK_EQUAL(1, tokens[5].line);
  CHECK_EQUAL(11, tokens[5].column);

  CHECK_TRUE(tokens[6].type == truk::ingestion::token_type_e::BANG_EQUAL);
  STRCMP_EQUAL("!=", tokens[6].lexeme.c_str());
  CHECK_EQUAL(13, tokens[6].source_index);
  CHECK_EQUAL(1, tokens[6].line);
  CHECK_EQUAL(14, tokens[6].column);

  CHECK_TRUE(tokens[7].type == truk::ingestion::token_type_e::LESS);
  STRCMP_EQUAL("<", tokens[7].lexeme.c_str());
  CHECK_EQUAL(16, tokens[7].source_index);
  CHECK_EQUAL(1, tokens[7].line);
  CHECK_EQUAL(17, tokens[7].column);

  CHECK_TRUE(tokens[8].type == truk::ingestion::token_type_e::LESS_EQUAL);
  STRCMP_EQUAL("<=", tokens[8].lexeme.c_str());
  CHECK_EQUAL(18, tokens[8].source_index);
  CHECK_EQUAL(1, tokens[8].line);
  CHECK_EQUAL(19, tokens[8].column);

  CHECK_TRUE(tokens[9].type == truk::ingestion::token_type_e::GREATER);
  STRCMP_EQUAL(">", tokens[9].lexeme.c_str());
  CHECK_EQUAL(21, tokens[9].source_index);
  CHECK_EQUAL(1, tokens[9].line);
  CHECK_EQUAL(22, tokens[9].column);

  CHECK_TRUE(tokens[10].type == truk::ingestion::token_type_e::GREATER_EQUAL);
  STRCMP_EQUAL(">=", tokens[10].lexeme.c_str());
  CHECK_EQUAL(23, tokens[10].source_index);
  CHECK_EQUAL(1, tokens[10].line);
  CHECK_EQUAL(24, tokens[10].column);

  CHECK_TRUE(tokens[11].type == truk::ingestion::token_type_e::AMP_AMP);
  STRCMP_EQUAL("&&", tokens[11].lexeme.c_str());
  CHECK_EQUAL(26, tokens[11].source_index);
  CHECK_EQUAL(1, tokens[11].line);
  CHECK_EQUAL(27, tokens[11].column);

  CHECK_TRUE(tokens[12].type == truk::ingestion::token_type_e::PIPE_PIPE);
  STRCMP_EQUAL("||", tokens[12].lexeme.c_str());
  CHECK_EQUAL(29, tokens[12].source_index);
  CHECK_EQUAL(1, tokens[12].line);
  CHECK_EQUAL(30, tokens[12].column);

  CHECK_TRUE(tokens[13].type == truk::ingestion::token_type_e::BANG);
  STRCMP_EQUAL("!", tokens[13].lexeme.c_str());
  CHECK_EQUAL(32, tokens[13].source_index);
  CHECK_EQUAL(1, tokens[13].line);
  CHECK_EQUAL(33, tokens[13].column);

  CHECK_TRUE(tokens[14].type == truk::ingestion::token_type_e::AMP);
  STRCMP_EQUAL("&", tokens[14].lexeme.c_str());
  CHECK_EQUAL(34, tokens[14].source_index);
  CHECK_EQUAL(1, tokens[14].line);
  CHECK_EQUAL(35, tokens[14].column);

  CHECK_TRUE(tokens[15].type == truk::ingestion::token_type_e::PIPE);
  STRCMP_EQUAL("|", tokens[15].lexeme.c_str());
  CHECK_EQUAL(36, tokens[15].source_index);
  CHECK_EQUAL(1, tokens[15].line);
  CHECK_EQUAL(37, tokens[15].column);

  CHECK_TRUE(tokens[16].type == truk::ingestion::token_type_e::CARET);
  STRCMP_EQUAL("^", tokens[16].lexeme.c_str());
  CHECK_EQUAL(38, tokens[16].source_index);
  CHECK_EQUAL(1, tokens[16].line);
  CHECK_EQUAL(39, tokens[16].column);

  CHECK_TRUE(tokens[17].type == truk::ingestion::token_type_e::TILDE);
  STRCMP_EQUAL("~", tokens[17].lexeme.c_str());
  CHECK_EQUAL(40, tokens[17].source_index);
  CHECK_EQUAL(1, tokens[17].line);
  CHECK_EQUAL(41, tokens[17].column);

  CHECK_TRUE(tokens[18].type == truk::ingestion::token_type_e::LESS_LESS);
  STRCMP_EQUAL("<<", tokens[18].lexeme.c_str());
  CHECK_EQUAL(42, tokens[18].source_index);
  CHECK_EQUAL(1, tokens[18].line);
  CHECK_EQUAL(43, tokens[18].column);

  CHECK_TRUE(tokens[19].type == truk::ingestion::token_type_e::GREATER_GREATER);
  STRCMP_EQUAL(">>", tokens[19].lexeme.c_str());
  CHECK_EQUAL(45, tokens[19].source_index);
  CHECK_EQUAL(1, tokens[19].line);
  CHECK_EQUAL(46, tokens[19].column);

  CHECK_TRUE(tokens[20].type == truk::ingestion::token_type_e::END_OF_FILE);
  CHECK_EQUAL(47, tokens[20].source_index);
  CHECK_EQUAL(1, tokens[20].line);
  CHECK_EQUAL(48, tokens[20].column);
}

TEST(IngestionTests, TokenizeComments) {
  const char *source = "var x // line comment\nvar y /* block comment */ var z";
  truk::ingestion::parser_c parser(source, 53);
  auto tokens = parser.tokenize();

  CHECK_EQUAL(7, tokens.size());

  CHECK_TRUE(tokens[0].type == truk::ingestion::token_type_e::KEYWORD);
  STRCMP_EQUAL("var", tokens[0].lexeme.c_str());
  CHECK_TRUE(tokens[0].keyword.has_value());
  CHECK_TRUE(tokens[0].keyword.value() == truk::language::keywords_e::VAR);
  CHECK_EQUAL(0, tokens[0].source_index);
  CHECK_EQUAL(1, tokens[0].line);
  CHECK_EQUAL(1, tokens[0].column);

  CHECK_TRUE(tokens[1].type == truk::ingestion::token_type_e::IDENTIFIER);
  STRCMP_EQUAL("x", tokens[1].lexeme.c_str());
  CHECK_FALSE(tokens[1].keyword.has_value());
  CHECK_EQUAL(4, tokens[1].source_index);
  CHECK_EQUAL(1, tokens[1].line);
  CHECK_EQUAL(5, tokens[1].column);

  CHECK_TRUE(tokens[2].type == truk::ingestion::token_type_e::KEYWORD);
  STRCMP_EQUAL("var", tokens[2].lexeme.c_str());
  CHECK_TRUE(tokens[2].keyword.has_value());
  CHECK_TRUE(tokens[2].keyword.value() == truk::language::keywords_e::VAR);
  CHECK_EQUAL(22, tokens[2].source_index);
  CHECK_EQUAL(2, tokens[2].line);
  CHECK_EQUAL(1, tokens[2].column);

  CHECK_TRUE(tokens[3].type == truk::ingestion::token_type_e::IDENTIFIER);
  STRCMP_EQUAL("y", tokens[3].lexeme.c_str());
  CHECK_FALSE(tokens[3].keyword.has_value());
  CHECK_EQUAL(26, tokens[3].source_index);
  CHECK_EQUAL(2, tokens[3].line);
  CHECK_EQUAL(5, tokens[3].column);

  CHECK_TRUE(tokens[4].type == truk::ingestion::token_type_e::KEYWORD);
  STRCMP_EQUAL("var", tokens[4].lexeme.c_str());
  CHECK_TRUE(tokens[4].keyword.has_value());
  CHECK_TRUE(tokens[4].keyword.value() == truk::language::keywords_e::VAR);
  CHECK_EQUAL(48, tokens[4].source_index);
  CHECK_EQUAL(2, tokens[4].line);
  CHECK_EQUAL(27, tokens[4].column);

  CHECK_TRUE(tokens[5].type == truk::ingestion::token_type_e::IDENTIFIER);
  STRCMP_EQUAL("z", tokens[5].lexeme.c_str());
  CHECK_FALSE(tokens[5].keyword.has_value());
  CHECK_EQUAL(52, tokens[5].source_index);
  CHECK_EQUAL(2, tokens[5].line);
  CHECK_EQUAL(31, tokens[5].column);

  CHECK_TRUE(tokens[6].type == truk::ingestion::token_type_e::END_OF_FILE);
  CHECK_EQUAL(53, tokens[6].source_index);
  CHECK_EQUAL(2, tokens[6].line);
  CHECK_EQUAL(32, tokens[6].column);
}

TEST(IngestionTests, MultilineTracking) {
  const char *source = "fn test() {\n  var x: i32 = 42;\n  return x;\n}";
  truk::ingestion::parser_c parser(source, 44);
  auto tokens = parser.tokenize();

  CHECK_EQUAL(17, tokens.size());

  CHECK_TRUE(tokens[0].type == truk::ingestion::token_type_e::KEYWORD);
  STRCMP_EQUAL("fn", tokens[0].lexeme.c_str());
  CHECK_EQUAL(0, tokens[0].source_index);
  CHECK_EQUAL(1, tokens[0].line);
  CHECK_EQUAL(1, tokens[0].column);

  CHECK_TRUE(tokens[1].type == truk::ingestion::token_type_e::IDENTIFIER);
  STRCMP_EQUAL("test", tokens[1].lexeme.c_str());
  CHECK_EQUAL(3, tokens[1].source_index);
  CHECK_EQUAL(1, tokens[1].line);
  CHECK_EQUAL(4, tokens[1].column);

  CHECK_TRUE(tokens[2].type == truk::ingestion::token_type_e::LEFT_PAREN);
  CHECK_EQUAL(7, tokens[2].source_index);
  CHECK_EQUAL(1, tokens[2].line);
  CHECK_EQUAL(8, tokens[2].column);

  CHECK_TRUE(tokens[3].type == truk::ingestion::token_type_e::RIGHT_PAREN);
  CHECK_EQUAL(8, tokens[3].source_index);
  CHECK_EQUAL(1, tokens[3].line);
  CHECK_EQUAL(9, tokens[3].column);

  CHECK_TRUE(tokens[4].type == truk::ingestion::token_type_e::LEFT_BRACE);
  CHECK_EQUAL(10, tokens[4].source_index);
  CHECK_EQUAL(1, tokens[4].line);
  CHECK_EQUAL(11, tokens[4].column);

  CHECK_TRUE(tokens[5].type == truk::ingestion::token_type_e::KEYWORD);
  STRCMP_EQUAL("var", tokens[5].lexeme.c_str());
  CHECK_EQUAL(14, tokens[5].source_index);
  CHECK_EQUAL(2, tokens[5].line);
  CHECK_EQUAL(3, tokens[5].column);

  CHECK_TRUE(tokens[6].type == truk::ingestion::token_type_e::IDENTIFIER);
  STRCMP_EQUAL("x", tokens[6].lexeme.c_str());
  CHECK_EQUAL(18, tokens[6].source_index);
  CHECK_EQUAL(2, tokens[6].line);
  CHECK_EQUAL(7, tokens[6].column);

  CHECK_TRUE(tokens[7].type == truk::ingestion::token_type_e::COLON);
  CHECK_EQUAL(19, tokens[7].source_index);
  CHECK_EQUAL(2, tokens[7].line);
  CHECK_EQUAL(8, tokens[7].column);

  CHECK_TRUE(tokens[8].type == truk::ingestion::token_type_e::KEYWORD);
  STRCMP_EQUAL("i32", tokens[8].lexeme.c_str());
  CHECK_EQUAL(21, tokens[8].source_index);
  CHECK_EQUAL(2, tokens[8].line);
  CHECK_EQUAL(10, tokens[8].column);

  CHECK_TRUE(tokens[9].type == truk::ingestion::token_type_e::EQUAL);
  CHECK_EQUAL(25, tokens[9].source_index);
  CHECK_EQUAL(2, tokens[9].line);
  CHECK_EQUAL(14, tokens[9].column);

  CHECK_TRUE(tokens[10].type == truk::ingestion::token_type_e::INTEGER_LITERAL);
  STRCMP_EQUAL("42", tokens[10].lexeme.c_str());
  CHECK_EQUAL(27, tokens[10].source_index);
  CHECK_EQUAL(2, tokens[10].line);
  CHECK_EQUAL(16, tokens[10].column);

  CHECK_TRUE(tokens[11].type == truk::ingestion::token_type_e::SEMICOLON);
  CHECK_EQUAL(29, tokens[11].source_index);
  CHECK_EQUAL(2, tokens[11].line);
  CHECK_EQUAL(18, tokens[11].column);

  CHECK_TRUE(tokens[12].type == truk::ingestion::token_type_e::KEYWORD);
  STRCMP_EQUAL("return", tokens[12].lexeme.c_str());
  CHECK_EQUAL(33, tokens[12].source_index);
  CHECK_EQUAL(3, tokens[12].line);
  CHECK_EQUAL(3, tokens[12].column);

  CHECK_TRUE(tokens[13].type == truk::ingestion::token_type_e::IDENTIFIER);
  STRCMP_EQUAL("x", tokens[13].lexeme.c_str());
  CHECK_EQUAL(40, tokens[13].source_index);
  CHECK_EQUAL(3, tokens[13].line);
  CHECK_EQUAL(10, tokens[13].column);

  CHECK_TRUE(tokens[14].type == truk::ingestion::token_type_e::SEMICOLON);
  CHECK_EQUAL(41, tokens[14].source_index);
  CHECK_EQUAL(3, tokens[14].line);
  CHECK_EQUAL(11, tokens[14].column);

  CHECK_TRUE(tokens[15].type == truk::ingestion::token_type_e::RIGHT_BRACE);
  CHECK_EQUAL(43, tokens[15].source_index);
  CHECK_EQUAL(4, tokens[15].line);
  CHECK_EQUAL(1, tokens[15].column);

  CHECK_TRUE(tokens[16].type == truk::ingestion::token_type_e::END_OF_FILE);
  CHECK_EQUAL(44, tokens[16].source_index);
  CHECK_EQUAL(4, tokens[16].line);
  CHECK_EQUAL(2, tokens[16].column);
}

TEST(IngestionTests, AllKeywordsHaveKeywordField) {
  const char *source =
      "fn struct var const if else while for in return break continue true "
      "false nil i8 i16 i32 i64 u8 u16 u32 u64 f32 f64 bool void";
  truk::ingestion::parser_c parser(source, 125);
  auto tokens = parser.tokenize();

  CHECK_EQUAL(28, tokens.size());

  for (size_t i = 0; i < tokens.size() - 1; i++) {
    CHECK_TRUE(tokens[i].type == truk::ingestion::token_type_e::KEYWORD);
    CHECK_TRUE(tokens[i].keyword.has_value());
  }

  CHECK_TRUE(tokens[0].keyword.value() == truk::language::keywords_e::FN);
  CHECK_TRUE(tokens[1].keyword.value() == truk::language::keywords_e::STRUCT);
  CHECK_TRUE(tokens[2].keyword.value() == truk::language::keywords_e::VAR);
  CHECK_TRUE(tokens[3].keyword.value() == truk::language::keywords_e::CONST);
  CHECK_TRUE(tokens[4].keyword.value() == truk::language::keywords_e::IF);
  CHECK_TRUE(tokens[5].keyword.value() == truk::language::keywords_e::ELSE);
  CHECK_TRUE(tokens[6].keyword.value() == truk::language::keywords_e::WHILE);
  CHECK_TRUE(tokens[7].keyword.value() == truk::language::keywords_e::FOR);
  CHECK_TRUE(tokens[8].keyword.value() == truk::language::keywords_e::IN);
  CHECK_TRUE(tokens[9].keyword.value() == truk::language::keywords_e::RETURN);
  CHECK_TRUE(tokens[10].keyword.value() == truk::language::keywords_e::BREAK);
  CHECK_TRUE(tokens[11].keyword.value() ==
             truk::language::keywords_e::CONTINUE);
  CHECK_TRUE(tokens[12].keyword.value() == truk::language::keywords_e::TRUE);
  CHECK_TRUE(tokens[13].keyword.value() == truk::language::keywords_e::FALSE);
  CHECK_TRUE(tokens[14].keyword.value() == truk::language::keywords_e::NIL);
  CHECK_TRUE(tokens[15].keyword.value() == truk::language::keywords_e::I8);
  CHECK_TRUE(tokens[16].keyword.value() == truk::language::keywords_e::I16);
  CHECK_TRUE(tokens[17].keyword.value() == truk::language::keywords_e::I32);
  CHECK_TRUE(tokens[18].keyword.value() == truk::language::keywords_e::I64);
  CHECK_TRUE(tokens[19].keyword.value() == truk::language::keywords_e::U8);
  CHECK_TRUE(tokens[20].keyword.value() == truk::language::keywords_e::U16);
  CHECK_TRUE(tokens[21].keyword.value() == truk::language::keywords_e::U32);
  CHECK_TRUE(tokens[22].keyword.value() == truk::language::keywords_e::U64);
  CHECK_TRUE(tokens[23].keyword.value() == truk::language::keywords_e::F32);
  CHECK_TRUE(tokens[24].keyword.value() == truk::language::keywords_e::F64);
  CHECK_TRUE(tokens[25].keyword.value() == truk::language::keywords_e::BOOL);
  CHECK_TRUE(tokens[26].keyword.value() == truk::language::keywords_e::VOID);

  CHECK_TRUE(tokens[27].type == truk::ingestion::token_type_e::END_OF_FILE);
}

TEST(IngestionTests, CompoundAssignmentOperators) {
  const char *source = "+= -= *= /= %=";
  truk::ingestion::parser_c parser(source, 14);
  auto tokens = parser.tokenize();

  CHECK_EQUAL(6, tokens.size());

  CHECK_TRUE(tokens[0].type == truk::ingestion::token_type_e::PLUS_EQUAL);
  STRCMP_EQUAL("+=", tokens[0].lexeme.c_str());
  CHECK_EQUAL(0, tokens[0].source_index);
  CHECK_EQUAL(1, tokens[0].line);
  CHECK_EQUAL(1, tokens[0].column);

  CHECK_TRUE(tokens[1].type == truk::ingestion::token_type_e::MINUS_EQUAL);
  STRCMP_EQUAL("-=", tokens[1].lexeme.c_str());
  CHECK_EQUAL(3, tokens[1].source_index);
  CHECK_EQUAL(1, tokens[1].line);
  CHECK_EQUAL(4, tokens[1].column);

  CHECK_TRUE(tokens[2].type == truk::ingestion::token_type_e::STAR_EQUAL);
  STRCMP_EQUAL("*=", tokens[2].lexeme.c_str());
  CHECK_EQUAL(6, tokens[2].source_index);
  CHECK_EQUAL(1, tokens[2].line);
  CHECK_EQUAL(7, tokens[2].column);

  CHECK_TRUE(tokens[3].type == truk::ingestion::token_type_e::SLASH_EQUAL);
  STRCMP_EQUAL("/=", tokens[3].lexeme.c_str());
  CHECK_EQUAL(9, tokens[3].source_index);
  CHECK_EQUAL(1, tokens[3].line);
  CHECK_EQUAL(10, tokens[3].column);

  CHECK_TRUE(tokens[4].type == truk::ingestion::token_type_e::PERCENT_EQUAL);
  STRCMP_EQUAL("%=", tokens[4].lexeme.c_str());
  CHECK_EQUAL(12, tokens[4].source_index);
  CHECK_EQUAL(1, tokens[4].line);
  CHECK_EQUAL(13, tokens[4].column);

  CHECK_TRUE(tokens[5].type == truk::ingestion::token_type_e::END_OF_FILE);
  CHECK_EQUAL(14, tokens[5].source_index);
  CHECK_EQUAL(1, tokens[5].line);
  CHECK_EQUAL(15, tokens[5].column);
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
