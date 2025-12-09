#include "../scanner.h"
#include "test.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void test_scanner_new_valid_position(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  ASSERT(buffer != NULL);

  uint8_t data[] = "hello world";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  ASSERT(scanner != NULL);
  ASSERT(scanner->buffer == buffer);
  ASSERT(scanner->position == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_scanner_new_mid_position(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  ASSERT(buffer != NULL);

  uint8_t data[] = "hello world";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 6);
  ASSERT(scanner != NULL);
  ASSERT(scanner->buffer == buffer);
  ASSERT(scanner->position == 6);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_scanner_new_end_position(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  ASSERT(buffer != NULL);

  uint8_t data[] = "hello world";
  size_t len = strlen((char *)data);
  slp_buffer_copy_to(buffer, data, len);

  slp_scanner_t *scanner = slp_scanner_new(buffer, len);
  ASSERT(scanner != NULL);
  ASSERT(scanner->buffer == buffer);
  ASSERT(scanner->position == len);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_scanner_new_invalid_position(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  ASSERT(buffer != NULL);

  uint8_t data[] = "hello world";
  size_t len = strlen((char *)data);
  slp_buffer_copy_to(buffer, data, len);

  slp_scanner_t *scanner = slp_scanner_new(buffer, len + 1);
  ASSERT(scanner == NULL);

  slp_buffer_free(buffer);
}

static void test_scanner_new_null_buffer(void) {
  slp_scanner_t *scanner = slp_scanner_new(NULL, 0);
  ASSERT(scanner == NULL);
}

static void test_scanner_new_empty_buffer(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  ASSERT(buffer != NULL);

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  ASSERT(scanner != NULL);
  ASSERT(scanner->buffer == buffer);
  ASSERT(scanner->position == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_scanner_free_null(void) { slp_scanner_free(NULL); }

static void test_scanner_does_not_own_buffer(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  ASSERT(buffer != NULL);

  uint8_t data[] = "test data";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  ASSERT(scanner != NULL);

  slp_scanner_free(scanner);

  ASSERT(buffer->data != NULL);
  ASSERT(buffer->count == strlen((char *)data));

  slp_buffer_free(buffer);
}

static void test_parse_simple_symbol(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "hello";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_SYMBOL);
  ASSERT(result.data.byte_length == 5);
  ASSERT(memcmp(result.data.data, "hello", 5) == 0);
  ASSERT(scanner->position == 5);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_simple_integer(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "42";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_INTEGER);
  ASSERT(result.data.byte_length == 2);
  ASSERT(memcmp(result.data.data, "42", 2) == 0);
  ASSERT(scanner->position == 2);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_simple_real(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "3.14";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_REAL);
  ASSERT(result.data.byte_length == 4);
  ASSERT(memcmp(result.data.data, "3.14", 4) == 0);
  ASSERT(scanner->position == 4);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_multiple_tokens(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "a +1 3.13";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);

  slp_scanner_static_type_result_t result1 =
      slp_scanner_read_static_base_type(scanner, NULL);
  ASSERT(result1.success);
  ASSERT(result1.data.base == SLP_STATIC_BASE_SYMBOL);
  ASSERT(result1.data.byte_length == 1);
  ASSERT(memcmp(result1.data.data, "a", 1) == 0);

  slp_scanner_static_type_result_t result2 =
      slp_scanner_read_static_base_type(scanner, NULL);
  ASSERT(result2.success);
  ASSERT(result2.data.base == SLP_STATIC_BASE_INTEGER);
  ASSERT(result2.data.byte_length == 2);
  ASSERT(memcmp(result2.data.data, "+1", 2) == 0);

  slp_scanner_static_type_result_t result3 =
      slp_scanner_read_static_base_type(scanner, NULL);
  ASSERT(result3.success);
  ASSERT(result3.data.base == SLP_STATIC_BASE_REAL);
  ASSERT(result3.data.byte_length == 4);
  ASSERT(memcmp(result3.data.data, "3.13", 4) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_positive_integer(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "+123";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_INTEGER);
  ASSERT(result.data.byte_length == 4);
  ASSERT(memcmp(result.data.data, "+123", 4) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_negative_integer(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "-42";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_INTEGER);
  ASSERT(result.data.byte_length == 3);
  ASSERT(memcmp(result.data.data, "-42", 3) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_negative_real(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "-2.5";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_REAL);
  ASSERT(result.data.byte_length == 4);
  ASSERT(memcmp(result.data.data, "-2.5", 4) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_sign_as_symbol(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "+a";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_SYMBOL);
  ASSERT(result.data.byte_length == 2);
  ASSERT(memcmp(result.data.data, "+a", 2) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_leading_whitespace(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "  \t\n42";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_INTEGER);
  ASSERT(result.data.byte_length == 2);
  ASSERT(memcmp(result.data.data, "42", 2) == 0);
  ASSERT(scanner->position == 6);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_whitespace_terminator(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "abc def";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_SYMBOL);
  ASSERT(result.data.byte_length == 3);
  ASSERT(memcmp(result.data.data, "abc", 3) == 0);
  ASSERT(scanner->position == 3);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_double_period_error(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "1.11.1";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(!result.success);
  ASSERT(scanner->position == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_invalid_integer(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "123x";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(!result.success);
  ASSERT(scanner->position == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_invalid_real(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "3.14x";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(!result.success);
  ASSERT(scanner->position == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_all_whitespace(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "   \t\n";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(!result.success);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_at_end_of_buffer(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "test";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 4);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(!result.success);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_null_scanner(void) {
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(NULL, NULL);

  ASSERT(!result.success);
}

static void test_parse_symbol_with_digits(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "var123";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_SYMBOL);
  ASSERT(result.data.byte_length == 6);
  ASSERT(memcmp(result.data.data, "var123", 6) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_lone_plus(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "+ ";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_SYMBOL);
  ASSERT(result.data.byte_length == 1);
  ASSERT(memcmp(result.data.data, "+", 1) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_lone_minus(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "-\t";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_SYMBOL);
  ASSERT(result.data.byte_length == 1);
  ASSERT(memcmp(result.data.data, "-", 1) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_real_with_trailing_digits(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "0.123456789";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_REAL);
  ASSERT(result.data.byte_length == 11);
  ASSERT(memcmp(result.data.data, "0.123456789", 11) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_zero(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "0";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_INTEGER);
  ASSERT(result.data.byte_length == 1);
  ASSERT(memcmp(result.data.data, "0", 1) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_special_chars_in_symbol(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "foo-bar_baz!";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_SYMBOL);
  ASSERT(result.data.byte_length == 12);
  ASSERT(memcmp(result.data.data, "foo-bar_baz!", 12) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_with_paren_stop_symbol(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "hello)world";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  uint8_t stop_chars[] = {')', '('};
  slp_scanner_stop_symbols_t stop_symbols = {.symbols = stop_chars, .count = 2};

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, &stop_symbols);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_SYMBOL);
  ASSERT(result.data.byte_length == 5);
  ASSERT(memcmp(result.data.data, "hello", 5) == 0);
  ASSERT(scanner->position == 5);
  ASSERT(buffer->data[scanner->position] == ')');

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_integer_with_paren_stop(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "42)";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  uint8_t stop_chars[] = {')'};
  slp_scanner_stop_symbols_t stop_symbols = {.symbols = stop_chars, .count = 1};

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, &stop_symbols);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_INTEGER);
  ASSERT(result.data.byte_length == 2);
  ASSERT(memcmp(result.data.data, "42", 2) == 0);
  ASSERT(scanner->position == 2);
  ASSERT(buffer->data[scanner->position] == ')');

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_real_with_paren_stop(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "3.14)";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  uint8_t stop_chars[] = {')'};
  slp_scanner_stop_symbols_t stop_symbols = {.symbols = stop_chars, .count = 1};

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, &stop_symbols);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_REAL);
  ASSERT(result.data.byte_length == 4);
  ASSERT(memcmp(result.data.data, "3.14", 4) == 0);
  ASSERT(scanner->position == 4);
  ASSERT(buffer->data[scanner->position] == ')');

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_multiple_tokens_with_stop_symbols(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "(add 42 3.14)";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  uint8_t stop_chars[] = {'(', ')'};
  slp_scanner_stop_symbols_t stop_symbols = {.symbols = stop_chars, .count = 2};

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);

  scanner->position = 1;

  slp_scanner_static_type_result_t result1 =
      slp_scanner_read_static_base_type(scanner, &stop_symbols);
  ASSERT(result1.success);
  ASSERT(result1.data.base == SLP_STATIC_BASE_SYMBOL);
  ASSERT(result1.data.byte_length == 3);
  ASSERT(memcmp(result1.data.data, "add", 3) == 0);

  slp_scanner_static_type_result_t result2 =
      slp_scanner_read_static_base_type(scanner, &stop_symbols);
  ASSERT(result2.success);
  ASSERT(result2.data.base == SLP_STATIC_BASE_INTEGER);
  ASSERT(result2.data.byte_length == 2);
  ASSERT(memcmp(result2.data.data, "42", 2) == 0);

  slp_scanner_static_type_result_t result3 =
      slp_scanner_read_static_base_type(scanner, &stop_symbols);
  ASSERT(result3.success);
  ASSERT(result3.data.base == SLP_STATIC_BASE_REAL);
  ASSERT(result3.data.byte_length == 4);
  ASSERT(memcmp(result3.data.data, "3.14", 4) == 0);
  ASSERT(scanner->position == 12);
  ASSERT(buffer->data[scanner->position] == ')');

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_stop_symbol_at_start(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = ")hello";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  uint8_t stop_chars[] = {')'};
  slp_scanner_stop_symbols_t stop_symbols = {.symbols = stop_chars, .count = 1};

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, &stop_symbols);

  ASSERT(!result.success);
  ASSERT(scanner->position == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

static void test_parse_null_stop_symbols_same_as_before(void) {
  slp_buffer_t *buffer = slp_buffer_new(64);
  uint8_t data[] = "test)data";
  slp_buffer_copy_to(buffer, data, strlen((char *)data));

  slp_scanner_t *scanner = slp_scanner_new(buffer, 0);
  slp_scanner_static_type_result_t result =
      slp_scanner_read_static_base_type(scanner, NULL);

  ASSERT(result.success);
  ASSERT(result.data.base == SLP_STATIC_BASE_SYMBOL);
  ASSERT(result.data.byte_length == 9);
  ASSERT(memcmp(result.data.data, "test)data", 9) == 0);

  slp_scanner_free(scanner);
  slp_buffer_free(buffer);
}

int main(void) {

  printf("Running scanner tests...\n");
  clock_t start = clock();

  int num_tests = 0;

#define TEST(test_name)                                                        \
  do {                                                                         \
    num_tests++;                                                               \
    test_name();                                                               \
  } while (0)

  TEST(test_scanner_new_valid_position);
  TEST(test_scanner_new_mid_position);
  TEST(test_scanner_new_end_position);
  TEST(test_scanner_new_invalid_position);
  TEST(test_scanner_new_null_buffer);
  TEST(test_scanner_new_empty_buffer);
  TEST(test_scanner_free_null);
  TEST(test_scanner_does_not_own_buffer);

  TEST(test_parse_simple_symbol);
  TEST(test_parse_simple_integer);
  TEST(test_parse_simple_real);
  TEST(test_parse_multiple_tokens);
  TEST(test_parse_positive_integer);
  TEST(test_parse_negative_integer);
  TEST(test_parse_negative_real);
  TEST(test_parse_sign_as_symbol);
  TEST(test_parse_leading_whitespace);
  TEST(test_parse_whitespace_terminator);
  TEST(test_parse_double_period_error);
  TEST(test_parse_invalid_integer);
  TEST(test_parse_invalid_real);
  TEST(test_parse_all_whitespace);
  TEST(test_parse_at_end_of_buffer);
  TEST(test_parse_null_scanner);
  TEST(test_parse_symbol_with_digits);
  TEST(test_parse_lone_plus);
  TEST(test_parse_lone_minus);
  TEST(test_parse_real_with_trailing_digits);
  TEST(test_parse_zero);
  TEST(test_parse_special_chars_in_symbol);

  TEST(test_parse_with_paren_stop_symbol);
  TEST(test_parse_integer_with_paren_stop);
  TEST(test_parse_real_with_paren_stop);
  TEST(test_parse_multiple_tokens_with_stop_symbols);
  TEST(test_parse_stop_symbol_at_start);
  TEST(test_parse_null_stop_symbols_same_as_before);

  clock_t end = clock();
  double elapsed_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

  printf("Scanner tests passed\n");
  printf("\tTotal tests run: %d\n", num_tests);
  printf("\tTotal test time: %.2f ms\n", elapsed_ms);

  return 0;
}
