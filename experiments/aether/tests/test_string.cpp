#include "truk/aether/forms.hpp"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

TEST_GROUP(StringTests){void setup() override{} void teardown() override{}};

TEST(StringTests, CanConstructEmptyString) {
  truk::aether::string_c str;
  CHECK_EQUAL(0, str.string_length());
  CHECK_EQUAL(1, str.byte_size());
  CHECK(str.c_str() != nullptr);
  CHECK_EQUAL('\0', str.c_str()[0]);
}

TEST(StringTests, CanConstructFromCString) {
  truk::aether::string_c str("hello");
  CHECK_EQUAL(5, str.string_length());
  CHECK_EQUAL(6, str.byte_size());
  STRCMP_EQUAL("hello", str.c_str());
}

TEST(StringTests, CanConstructFromStdString) {
  std::string input = "world";
  truk::aether::string_c str(input);
  CHECK_EQUAL(5, str.string_length());
  STRCMP_EQUAL("world", str.c_str());
}

TEST(StringTests, CanConstructFromNullptr) {
  truk::aether::string_c str(nullptr);
  CHECK_EQUAL(0, str.string_length());
  CHECK_EQUAL('\0', str.c_str()[0]);
}

TEST(StringTests, ToStringMethod) {
  truk::aether::string_c str("test");
  std::string result = str.to_string();
  CHECK_EQUAL(std::string("test"), result);
  CHECK_EQUAL(4, result.size());
}

TEST(StringTests, EmptyStringToString) {
  truk::aether::string_c str;
  std::string result = str.to_string();
  CHECK_EQUAL(std::string(""), result);
  CHECK_EQUAL(0, result.size());
}

TEST(StringTests, AppendCString) {
  truk::aether::string_c str("hello");
  str.append(" world");
  STRCMP_EQUAL("hello world", str.c_str());
  CHECK_EQUAL(11, str.string_length());
}

TEST(StringTests, AppendStdString) {
  truk::aether::string_c str("foo");
  str.append(std::string("bar"));
  STRCMP_EQUAL("foobar", str.c_str());
  CHECK_EQUAL(6, str.string_length());
}

TEST(StringTests, AppendToEmptyString) {
  truk::aether::string_c str;
  str.append("first");
  STRCMP_EQUAL("first", str.c_str());
  CHECK_EQUAL(5, str.string_length());
}

TEST(StringTests, AppendNull) {
  truk::aether::string_c str("test");
  str.append(nullptr);
  STRCMP_EQUAL("test", str.c_str());
  CHECK_EQUAL(4, str.string_length());
}

TEST(StringTests, MultipleAppends) {
  truk::aether::string_c str("a");
  str.append("b");
  str.append("c");
  str.append("d");
  STRCMP_EQUAL("abcd", str.c_str());
  CHECK_EQUAL(4, str.string_length());
}

TEST(StringTests, ClearString) {
  truk::aether::string_c str("hello world");
  CHECK_EQUAL(11, str.string_length());

  str.clear();
  CHECK_EQUAL(0, str.string_length());
  CHECK_EQUAL('\0', str.c_str()[0]);
}

TEST(StringTests, ClearEmptyString) {
  truk::aether::string_c str;
  str.clear();
  CHECK_EQUAL(0, str.string_length());
}

TEST(StringTests, AlwaysNullTerminated) {
  truk::aether::string_c str("test");
  const auto &bytes = str.get_bytes();
  CHECK_EQUAL(0, bytes.back());
}

TEST(StringTests, NullTerminatedAfterAppend) {
  truk::aether::string_c str("hello");
  str.append(" world");
  const auto &bytes = str.get_bytes();
  CHECK_EQUAL(0, bytes.back());
  CHECK_EQUAL(12, bytes.size());
}

TEST(StringTests, NullTerminatedAfterClear) {
  truk::aether::string_c str("something");
  str.clear();
  const auto &bytes = str.get_bytes();
  CHECK_EQUAL(1, bytes.size());
  CHECK_EQUAL(0, bytes[0]);
}

TEST(StringTests, LongString) {
  std::string long_str(1000, 'x');
  truk::aether::string_c str(long_str);
  CHECK_EQUAL(1000, str.string_length());
  CHECK_EQUAL(1001, str.byte_size());
  std::string result = str.to_string();
  CHECK_EQUAL(long_str, result);
}

TEST(StringTests, StringWithSpecialCharacters) {
  truk::aether::string_c str("hello\nworld\ttab");
  STRCMP_EQUAL("hello\nworld\ttab", str.c_str());
  CHECK_EQUAL(15, str.string_length());
}

TEST(StringTests, EmptyAppend) {
  truk::aether::string_c str("test");
  str.append("");
  STRCMP_EQUAL("test", str.c_str());
  CHECK_EQUAL(4, str.string_length());
}

TEST(StringTests, AppendEmptyStdString) {
  truk::aether::string_c str("test");
  str.append(std::string());
  STRCMP_EQUAL("test", str.c_str());
  CHECK_EQUAL(4, str.string_length());
}

TEST(StringTests, ConstructFromBytes) {
  std::vector<std::uint8_t> bytes = {'h', 'e', 'l', 'l', 'o'};
  truk::aether::string_c str(bytes);
  STRCMP_EQUAL("hello", str.c_str());
  CHECK_EQUAL(5, str.string_length());
}

TEST(StringTests, ByteSizeIncludesNullTerminator) {
  truk::aether::string_c str("abc");
  CHECK_EQUAL(3, str.string_length());
  CHECK_EQUAL(4, str.byte_size());
}

TEST_GROUP(StringArrayTests){void setup() override{} void teardown()
                                 override{}};

TEST(StringArrayTests, CanConstructArrayOfStrings) {
  truk::aether::array_c<truk::aether::string_c> arr(5);
  CHECK_EQUAL(5, arr.length());
}

TEST(StringArrayTests, ArrayOfStringsDefaultInitialized) {
  truk::aether::array_c<truk::aether::string_c> arr(3);

  CHECK_EQUAL(0, arr[0].string_length());
  CHECK_EQUAL(0, arr[1].string_length());
  CHECK_EQUAL(0, arr[2].string_length());
}

TEST(StringArrayTests, CanModifyStringsInArray) {
  truk::aether::array_c<truk::aether::string_c> arr(3);

  arr[0] = truk::aether::string_c("hello");
  arr[1] = truk::aether::string_c("world");
  arr[2] = truk::aether::string_c("!");

  STRCMP_EQUAL("hello", arr[0].c_str());
  STRCMP_EQUAL("world", arr[1].c_str());
  STRCMP_EQUAL("!", arr[2].c_str());
}

TEST(StringArrayTests, CanAppendToStringsInArray) {
  truk::aether::array_c<truk::aether::string_c> arr(2);

  arr[0] = truk::aether::string_c("foo");
  arr[0].append("bar");

  arr[1] = truk::aether::string_c("hello");
  arr[1].append(" world");

  STRCMP_EQUAL("foobar", arr[0].c_str());
  STRCMP_EQUAL("hello world", arr[1].c_str());
}

TEST(StringArrayTests, StringArrayIndependence) {
  truk::aether::array_c<truk::aether::string_c> arr(2);

  arr[0] = truk::aether::string_c("first");
  arr[1] = truk::aether::string_c("second");

  arr[0].append(" modified");

  STRCMP_EQUAL("first modified", arr[0].c_str());
  STRCMP_EQUAL("second", arr[1].c_str());
}

TEST(StringArrayTests, StringArrayBoundsChecking) {
  truk::aether::array_c<truk::aether::string_c> arr(3);

  arr.at(0) = truk::aether::string_c("safe");
  STRCMP_EQUAL("safe", arr.at(0).c_str());

  try {
    arr.at(3) = truk::aether::string_c("oob");
    FAIL("Expected aether_bounds_exception_c");
  } catch (const truk::aether::aether_bounds_exception_c &e) {
    CHECK(e.what() != nullptr);
  }
}

TEST(StringArrayTests, LargeStringArray) {
  truk::aether::array_c<truk::aether::string_c> arr(100);
  CHECK_EQUAL(100, arr.length());

  arr[0] = truk::aether::string_c("first");
  arr[99] = truk::aether::string_c("last");

  STRCMP_EQUAL("first", arr[0].c_str());
  STRCMP_EQUAL("last", arr[99].c_str());
}

TEST(StringArrayTests, IterateAndModifyStringArray) {
  truk::aether::array_c<truk::aether::string_c> arr(5);

  for (std::size_t i = 0; i < arr.length(); ++i) {
    arr[i] = truk::aether::string_c("item");
    arr[i].append(std::to_string(i));
  }

  STRCMP_EQUAL("item0", arr[0].c_str());
  STRCMP_EQUAL("item1", arr[1].c_str());
  STRCMP_EQUAL("item2", arr[2].c_str());
  STRCMP_EQUAL("item3", arr[3].c_str());
  STRCMP_EQUAL("item4", arr[4].c_str());
}

TEST(StringArrayTests, EmptyStringsInArray) {
  truk::aether::array_c<truk::aether::string_c> arr(3);

  arr[0] = truk::aether::string_c("");
  arr[1] = truk::aether::string_c();
  arr[2] = truk::aether::string_c(nullptr);

  CHECK_EQUAL(0, arr[0].string_length());
  CHECK_EQUAL(0, arr[1].string_length());
  CHECK_EQUAL(0, arr[2].string_length());
}

TEST(StringArrayTests, LongStringsInArray) {
  truk::aether::array_c<truk::aether::string_c> arr(2);

  std::string long_str(500, 'x');
  arr[0] = truk::aether::string_c(long_str);
  arr[1] = truk::aether::string_c("short");

  CHECK_EQUAL(500, arr[0].string_length());
  CHECK_EQUAL(5, arr[1].string_length());
  CHECK_EQUAL(long_str, arr[0].to_string());
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
