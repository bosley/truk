#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

extern "C" {
#include <stdlib.h>
#include <string.h>
#include <sxs/ds/scanner.h>
}

TEST_GROUP(ScannerBasic){};

TEST(ScannerBasic, NewValidPosition) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  CHECK(buffer != NULL);

  uint8_t data[] = "hello world";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  CHECK(scanner != NULL);
  POINTERS_EQUAL(buffer, scanner->buffer);
  CHECK_EQUAL(0, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerBasic, NewMidPosition) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  CHECK(buffer != NULL);

  uint8_t data[] = "hello world";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 6);
  CHECK(scanner != NULL);
  POINTERS_EQUAL(buffer, scanner->buffer);
  CHECK_EQUAL(6, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerBasic, NewEndPosition) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  CHECK(buffer != NULL);

  uint8_t data[] = "hello world";
  size_t len = strlen((char *)data);
  slp_buffer_copy_to(buffer, data, len);

  slp_scanner_t *scanner = slp_scanner_new(buffer, len);
  CHECK(scanner != NULL);
  POINTERS_EQUAL(buffer, scanner->buffer);
  CHECK_EQUAL(len, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerBasic, NewInvalidPosition) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  CHECK(buffer != NULL);

  uint8_t data[] = "hello world";
  size_t len = strlen((char *)data);
  slp_buffer_copy_to(buffer, data, len);

  slp_scanner_t *scanner = slp_scanner_new(buffer, len + 1);
  POINTERS_EQUAL(NULL, scanner);

  slp_buffer_free(buffer);
}

TEST(ScannerBasic, NewNullBuffer) {
  slp_scanner_t *scanner = slp_scanner_new(NULL, 0);
  POINTERS_EQUAL(NULL, scanner);
}

TEST(ScannerBasic, NewEmptyBuffer) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  CHECK(buffer != NULL);

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  CHECK(scanner != NULL);
  POINTERS_EQUAL(buffer, scanner->buffer);
  CHECK_EQUAL(0, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerBasic, FreeNull) { slp_scanner_free(NULL); }

TEST(ScannerBasic, DoesNotOwnBuffer) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  CHECK(buffer != NULL);

  uint8_t data[] = "test data";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  CHECK(scanner != NULL);

  slp_scanner_free(scanner);

  CHECK(buffer->data != NULL);
  CHECK_EQUAL(strlen((char *)data), buffer->count);

  slp_buffer_free(buffer);
}

TEST_GROUP(ScannerParse){};

TEST(ScannerParse, SimpleSymbol) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "hello";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_SYMBOL, result.data.base);
  CHECK_EQUAL(5, result.data.byte_length);
  CHECK(memcmp(result.data.data, "hello", 5) == 0);
  CHECK_EQUAL(5, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, SimpleInteger) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "42";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_INTEGER, result.data.base);
  CHECK_EQUAL(2, result.data.byte_length);
  CHECK(memcmp(result.data.data, "42", 2) == 0);
  CHECK_EQUAL(2, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, SimpleReal) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "3.14";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_REAL, result.data.base);
  CHECK_EQUAL(4, result.data.byte_length);
  CHECK(memcmp(result.data.data, "3.14", 4) == 0);
  CHECK_EQUAL(4, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, MultipleTokens) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "a +1 3.13";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);

  slp_scanner_static_type_result_t result1 =
      slp_scanner_read_static_base_type(scanner, NULL);
  CHECK(result1.success);
  CHECK_EQUAL(SLP_STATIC_BASE_SYMBOL, result1.data.base);
  CHECK_EQUAL(1, result1.data.byte_length);
  CHECK(memcmp(result1.data.data, "a", 1) == 0);

  slp_scanner_static_type_result_t result2 =
      slp_scanner_read_static_base_type(scanner, NULL);
  CHECK(result2.success);
  CHECK_EQUAL(SLP_STATIC_BASE_INTEGER, result2.data.base);
  CHECK_EQUAL(2, result2.data.byte_length);
  CHECK(memcmp(result2.data.data, "+1", 2) == 0);

  slp_scanner_static_type_result_t result3 =
      slp_scanner_read_static_base_type(scanner, NULL);
  CHECK(result3.success);
  CHECK_EQUAL(SLP_STATIC_BASE_REAL, result3.data.base);
  CHECK_EQUAL(4, result3.data.byte_length);
  CHECK(memcmp(result3.data.data, "3.13", 4) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, PositiveInteger) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "+123";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_INTEGER, result.data.base);
  CHECK_EQUAL(4, result.data.byte_length);
  CHECK(memcmp(result.data.data, "+123", 4) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, NegativeInteger) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "-42";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_INTEGER, result.data.base);
  CHECK_EQUAL(3, result.data.byte_length);
  CHECK(memcmp(result.data.data, "-42", 3) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, NegativeReal) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "-2.5";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_REAL, result.data.base);
  CHECK_EQUAL(4, result.data.byte_length);
  CHECK(memcmp(result.data.data, "-2.5", 4) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, SignAsSymbol) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "+a";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_SYMBOL, result.data.base);
  CHECK_EQUAL(2, result.data.byte_length);
  CHECK(memcmp(result.data.data, "+a", 2) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, LeadingWhitespace) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "  \t\n42";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_INTEGER, result.data.base);
  CHECK_EQUAL(2, result.data.byte_length);
  CHECK(memcmp(result.data.data, "42", 2) == 0);
  CHECK_EQUAL(6, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, WhitespaceTerminator) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "abc def";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_SYMBOL, result.data.base);
  CHECK_EQUAL(3, result.data.byte_length);
  CHECK(memcmp(result.data.data, "abc", 3) == 0);
  CHECK_EQUAL(3, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, DoublePeriodError) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "1.11.1";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK_FALSE(result.success);
  CHECK_EQUAL(0, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, InvalidInteger) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "123x";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK_FALSE(result.success);
  CHECK_EQUAL(0, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, InvalidReal) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "3.14x";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK_FALSE(result.success);
  CHECK_EQUAL(0, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, AllWhitespace) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "   \t\n";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK_FALSE(result.success);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, AtEndOfBuffer) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "test";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 4);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK_FALSE(result.success);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, NullScanner) {
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(NULL, NULL);

  CHECK_FALSE(result.success);
}

TEST(ScannerParse, SymbolWithDigits) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "var123";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_SYMBOL, result.data.base);
  CHECK_EQUAL(6, result.data.byte_length);
  CHECK(memcmp(result.data.data, "var123", 6) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, LonePlus) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "+ ";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_SYMBOL, result.data.base);
  CHECK_EQUAL(1, result.data.byte_length);
  CHECK(memcmp(result.data.data, "+", 1) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, LoneMinus) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "-\t";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_SYMBOL, result.data.base);
  CHECK_EQUAL(1, result.data.byte_length);
  CHECK(memcmp(result.data.data, "-", 1) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerParse, Zero) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "0";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_INTEGER, result.data.base);
  CHECK_EQUAL(1, result.data.byte_length);
  CHECK(memcmp(result.data.data, "0", 1) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST_GROUP(ScannerStopSymbols){};

TEST(ScannerStopSymbols, WithParenStop) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "hello)world";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  uint8_t stop_chars[] = {')', '('};
  slp_scanner_stop_symbols_t stop_symbols = {.symbols = stop_chars, .count = 2};

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, &stop_symbols);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_SYMBOL, result.data.base);
  CHECK_EQUAL(5, result.data.byte_length);
  CHECK(memcmp(result.data.data, "hello", 5) == 0);
  CHECK_EQUAL(5, scanner->position);
  CHECK_EQUAL(')', buffer->data[scanner->position]);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerStopSymbols, IntegerWithParenStop) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "42)";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  uint8_t stop_chars[] = {')'};
  slp_scanner_stop_symbols_t stop_symbols = {.symbols = stop_chars, .count = 1};

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, &stop_symbols);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_INTEGER, result.data.base);
  CHECK_EQUAL(2, result.data.byte_length);
  CHECK(memcmp(result.data.data, "42", 2) == 0);
  CHECK_EQUAL(2, scanner->position);
  CHECK_EQUAL(')', buffer->data[scanner->position]);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerStopSymbols, RealWithParenStop) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "3.14)";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  uint8_t stop_chars[] = {')'};
  slp_scanner_stop_symbols_t stop_symbols = {.symbols = stop_chars, .count = 1};

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, &stop_symbols);

  CHECK(result.success);
  CHECK_EQUAL(SLP_STATIC_BASE_REAL, result.data.base);
  CHECK_EQUAL(4, result.data.byte_length);
  CHECK(memcmp(result.data.data, "3.14", 4) == 0);
  CHECK_EQUAL(4, scanner->position);
  CHECK_EQUAL(')', buffer->data[scanner->position]);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerStopSymbols, StopSymbolAtStart) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = ")hello";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  uint8_t stop_chars[] = {')'};
  slp_scanner_stop_symbols_t stop_symbols = {.symbols = stop_chars, .count = 1};

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, &stop_symbols);

  CHECK_FALSE(result.success);
  CHECK_EQUAL(0, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST_GROUP(ScannerFindGroup){};

TEST(ScannerFindGroup, SimpleParens) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "(hello)";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_find_group_result_t result =
      slp_scanner_find_group(scanner, '(', ')', NULL, false);

  CHECK(result.success);
  CHECK_EQUAL(0, result.index_of_start_symbol);
  CHECK_EQUAL(6, result.index_of_closing_symbol);
  CHECK_EQUAL(6, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerFindGroup, SimpleBrackets) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "[data]";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_find_group_result_t result =
      slp_scanner_find_group(scanner, '[', ']', NULL, false);

  CHECK(result.success);
  CHECK_EQUAL(0, result.index_of_start_symbol);
  CHECK_EQUAL(5, result.index_of_closing_symbol);
  CHECK_EQUAL(5, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerFindGroup, SimpleBraces) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "{content}";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_find_group_result_t result =
      slp_scanner_find_group(scanner, '{', '}', NULL, false);

  CHECK(result.success);
  CHECK_EQUAL(0, result.index_of_start_symbol);
  CHECK_EQUAL(8, result.index_of_closing_symbol);
  CHECK_EQUAL(8, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerFindGroup, CustomDelimiters) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "!a b +1 2$";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_find_group_result_t result =
      slp_scanner_find_group(scanner, '!', '$', NULL, false);

  CHECK(result.success);
  CHECK_EQUAL(0, result.index_of_start_symbol);
  CHECK_EQUAL(9, result.index_of_closing_symbol);
  CHECK_EQUAL(9, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerFindGroup, EscapedQuotes) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "\"hello \\\"world\\\"!\"";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  uint8_t escape = '\\';
  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_find_group_result_t result =
      slp_scanner_find_group(scanner, '"', '"', &escape, false);

  CHECK(result.success);
  CHECK_EQUAL(0, result.index_of_start_symbol);
  CHECK_EQUAL(17, result.index_of_closing_symbol);
  CHECK_EQUAL(17, scanner->position);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerFindGroup, MultipleEscapedEndSymbols) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "(a\\)b\\)c)";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  uint8_t escape = '\\';
  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_find_group_result_t result =
      slp_scanner_find_group(scanner, '(', ')', &escape, false);

  CHECK(result.success);
  CHECK_EQUAL(0, result.index_of_start_symbol);
  CHECK_EQUAL(8, result.index_of_closing_symbol);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerFindGroup, WithLeadingWhitespace) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "  \t\n(data)";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_find_group_result_t result =
      slp_scanner_find_group(scanner, '(', ')', NULL, true);

  CHECK(result.success);
  CHECK_EQUAL(4, result.index_of_start_symbol);
  CHECK_EQUAL(9, result.index_of_closing_symbol);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerFindGroup, NoLeadingWhitespaceConsumption) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "  (data)";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_find_group_result_t result =
      slp_scanner_find_group(scanner, '(', ')', NULL, false);

  CHECK_FALSE(result.success);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerFindGroup, MissingStartSymbol) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "hello)";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_find_group_result_t result =
      slp_scanner_find_group(scanner, '(', ')', NULL, false);

  CHECK_FALSE(result.success);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerFindGroup, MissingEndSymbol) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "(hello";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_find_group_result_t result =
      slp_scanner_find_group(scanner, '(', ')', NULL, false);

  CHECK_FALSE(result.success);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerFindGroup, EmptyGroup) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "()";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_find_group_result_t result =
      slp_scanner_find_group(scanner, '(', ')', NULL, false);

  CHECK(result.success);
  CHECK_EQUAL(0, result.index_of_start_symbol);
  CHECK_EQUAL(1, result.index_of_closing_symbol);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

TEST(ScannerFindGroup, NullScanner) {
  slp_scanner_find_group_result_t result =
      slp_scanner_find_group(NULL, '(', ')', NULL, false);

  CHECK_FALSE(result.success);
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
