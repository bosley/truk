#include "truk/aether/forms.hpp"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

TEST_GROUP(ArrayTests){void setup() override{} void teardown() override{}};

TEST(ArrayTests, CanConstructArrayWithSize) {
  truk::aether::array_c<truk::aether::i32_c> arr(10);
  CHECK_EQUAL(10, arr.length());
}

TEST(ArrayTests, CanAccessArrayElements) {
  truk::aether::array_c<truk::aether::i32_c> arr(5);
  auto &elem = arr[0];
  CHECK_EQUAL(0, elem.value());
}

TEST(ArrayTests, CanModifyArrayElements) {
  truk::aether::array_c<truk::aether::i32_c> arr(5);
  arr[0] = truk::aether::i32_c(42);
  arr[1] = truk::aether::i32_c(-100);

  CHECK_EQUAL(42, arr[0].value());
  CHECK_EQUAL(-100, arr[1].value());
}

TEST(ArrayTests, ArrayOfI8) {
  truk::aether::array_c<truk::aether::i8_c> arr(3);
  CHECK_EQUAL(3, arr.length());

  arr[0] = truk::aether::i8_c(-10);
  arr[1] = truk::aether::i8_c(20);
  arr[2] = truk::aether::i8_c(-30);

  CHECK_EQUAL(-10, arr[0].value());
  CHECK_EQUAL(20, arr[1].value());
  CHECK_EQUAL(-30, arr[2].value());
}

TEST(ArrayTests, ArrayOfI16) {
  truk::aether::array_c<truk::aether::i16_c> arr(3);
  CHECK_EQUAL(3, arr.length());

  arr[0] = truk::aether::i16_c(-1000);
  arr[1] = truk::aether::i16_c(2000);
  arr[2] = truk::aether::i16_c(-3000);

  CHECK_EQUAL(-1000, arr[0].value());
  CHECK_EQUAL(2000, arr[1].value());
  CHECK_EQUAL(-3000, arr[2].value());
}

TEST(ArrayTests, ArrayOfI32) {
  truk::aether::array_c<truk::aether::i32_c> arr(3);
  CHECK_EQUAL(3, arr.length());

  arr[0] = truk::aether::i32_c(-100000);
  arr[1] = truk::aether::i32_c(200000);
  arr[2] = truk::aether::i32_c(-300000);

  CHECK_EQUAL(-100000, arr[0].value());
  CHECK_EQUAL(200000, arr[1].value());
  CHECK_EQUAL(-300000, arr[2].value());
}

TEST(ArrayTests, ArrayOfI64) {
  truk::aether::array_c<truk::aether::i64_c> arr(3);
  CHECK_EQUAL(3, arr.length());

  arr[0] = truk::aether::i64_c(-10000000000LL);
  arr[1] = truk::aether::i64_c(20000000000LL);
  arr[2] = truk::aether::i64_c(-30000000000LL);

  CHECK_EQUAL(-10000000000LL, arr[0].value());
  CHECK_EQUAL(20000000000LL, arr[1].value());
  CHECK_EQUAL(-30000000000LL, arr[2].value());
}

TEST(ArrayTests, ArrayOfU8) {
  truk::aether::array_c<truk::aether::u8_c> arr(3);
  CHECK_EQUAL(3, arr.length());

  arr[0] = truk::aether::u8_c(10);
  arr[1] = truk::aether::u8_c(200);
  arr[2] = truk::aether::u8_c(255);

  CHECK_EQUAL(10, arr[0].value());
  CHECK_EQUAL(200, arr[1].value());
  CHECK_EQUAL(255, arr[2].value());
}

TEST(ArrayTests, ArrayOfU16) {
  truk::aether::array_c<truk::aether::u16_c> arr(3);
  CHECK_EQUAL(3, arr.length());

  arr[0] = truk::aether::u16_c(1000);
  arr[1] = truk::aether::u16_c(50000);
  arr[2] = truk::aether::u16_c(65535);

  CHECK_EQUAL(1000, arr[0].value());
  CHECK_EQUAL(50000, arr[1].value());
  CHECK_EQUAL(65535, arr[2].value());
}

TEST(ArrayTests, ArrayOfU32) {
  truk::aether::array_c<truk::aether::u32_c> arr(3);
  CHECK_EQUAL(3, arr.length());

  arr[0] = truk::aether::u32_c(100000);
  arr[1] = truk::aether::u32_c(3000000000U);
  arr[2] = truk::aether::u32_c(4000000000U);

  CHECK_EQUAL(100000, arr[0].value());
  CHECK_EQUAL(3000000000U, arr[1].value());
  CHECK_EQUAL(4000000000U, arr[2].value());
}

TEST(ArrayTests, ArrayOfU64) {
  truk::aether::array_c<truk::aether::u64_c> arr(3);
  CHECK_EQUAL(3, arr.length());

  arr[0] = truk::aether::u64_c(10000000000ULL);
  arr[1] = truk::aether::u64_c(20000000000ULL);
  arr[2] = truk::aether::u64_c(18000000000000000000ULL);

  CHECK_EQUAL(10000000000ULL, arr[0].value());
  CHECK_EQUAL(20000000000ULL, arr[1].value());
  CHECK_EQUAL(18000000000000000000ULL, arr[2].value());
}

TEST(ArrayTests, ArrayOfR32) {
  truk::aether::array_c<truk::aether::r32_c> arr(3);
  CHECK_EQUAL(3, arr.length());

  arr[0] = truk::aether::r32_c(3.14f);
  arr[1] = truk::aether::r32_c(-2.71f);
  arr[2] = truk::aether::r32_c(1.41f);

  DOUBLES_EQUAL(3.14f, arr[0].value(), 0.001);
  DOUBLES_EQUAL(-2.71f, arr[1].value(), 0.001);
  DOUBLES_EQUAL(1.41f, arr[2].value(), 0.001);
}

TEST(ArrayTests, ArrayOfR64) {
  truk::aether::array_c<truk::aether::r64_c> arr(3);
  CHECK_EQUAL(3, arr.length());

  arr[0] = truk::aether::r64_c(3.14159265359);
  arr[1] = truk::aether::r64_c(-2.71828182846);
  arr[2] = truk::aether::r64_c(1.41421356237);

  DOUBLES_EQUAL(3.14159265359, arr[0].value(), 0.0000001);
  DOUBLES_EQUAL(-2.71828182846, arr[1].value(), 0.0000001);
  DOUBLES_EQUAL(1.41421356237, arr[2].value(), 0.0000001);
}

TEST(ArrayTests, ArrayOfBool) {
  truk::aether::array_c<truk::aether::bool_c> arr(4);
  CHECK_EQUAL(4, arr.length());

  CHECK_EQUAL(false, arr[0].value());
  CHECK_EQUAL(false, arr[1].value());

  arr[0] = truk::aether::bool_c(true);
  arr[1] = truk::aether::bool_c(false);
  arr[2] = truk::aether::bool_c(true);
  arr[3] = truk::aether::bool_c(true);

  CHECK_EQUAL(true, arr[0].value());
  CHECK_EQUAL(false, arr[1].value());
  CHECK_EQUAL(true, arr[2].value());
  CHECK_EQUAL(true, arr[3].value());
}

TEST(ArrayTests, ArrayDefaultInitialization) {
  truk::aether::array_c<truk::aether::i32_c> arr(5);

  for (std::size_t i = 0; i < arr.length(); ++i) {
    CHECK_EQUAL(0, arr[i].value());
  }
}

TEST(ArrayTests, ArrayArithmetic) {
  truk::aether::array_c<truk::aether::i32_c> arr(3);

  arr[0] = truk::aether::i32_c(10);
  arr[1] = truk::aether::i32_c(-20);
  arr[2] = truk::aether::i32_c(30);

  auto sum = arr[0] + arr[1];
  CHECK_EQUAL(-10, sum.value());

  auto diff = arr[2] - arr[1];
  CHECK_EQUAL(50, diff.value());
}

TEST(ArrayTests, LargeArray) {
  truk::aether::array_c<truk::aether::u8_c> arr(1000);
  CHECK_EQUAL(1000, arr.length());

  arr[0] = truk::aether::u8_c(1);
  arr[999] = truk::aether::u8_c(255);

  CHECK_EQUAL(1, arr[0].value());
  CHECK_EQUAL(255, arr[999].value());
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
