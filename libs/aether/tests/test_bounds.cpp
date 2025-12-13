#include "truk/aether/forms.hpp"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

TEST_GROUP(BoundsCheckingTests){void setup() override{} void teardown()
                                    override{}};

TEST(BoundsCheckingTests, AtMethodWithinBounds) {
  truk::aether::array_c<truk::aether::i32_c> arr(10);
  arr.at(0) = truk::aether::i32_c(42);
  arr.at(9) = truk::aether::i32_c(99);

  CHECK_EQUAL(42, arr.at(0).value());
  CHECK_EQUAL(99, arr.at(9).value());
}

TEST(BoundsCheckingTests, AtMethodThrowsOnOutOfBounds) {
  truk::aether::array_c<truk::aether::i32_c> arr(10);

  try {
    arr.at(10) = truk::aether::i32_c(100);
    FAIL("Expected aether_bounds_exception_c");
  } catch (const truk::aether::aether_bounds_exception_c &e) {
    CHECK(e.what() != nullptr);
  }
}

TEST(BoundsCheckingTests, AtMethodThrowsOnLargeIndex) {
  truk::aether::array_c<truk::aether::u8_c> arr(5);

  try {
    auto val = arr.at(1000);
    (void)val;
    FAIL("Expected aether_bounds_exception_c");
  } catch (const truk::aether::aether_bounds_exception_c &e) {
    CHECK(e.what() != nullptr);
  }
}

TEST(BoundsCheckingTests, ConstAtMethodWithinBounds) {
  truk::aether::array_c<truk::aether::i32_c> arr(10);
  arr[5] = truk::aether::i32_c(55);

  const auto &const_arr = arr;
  CHECK_EQUAL(55, const_arr.at(5).value());
}

TEST(BoundsCheckingTests, ConstAtMethodThrowsOnOutOfBounds) {
  truk::aether::array_c<truk::aether::i32_c> arr(10);
  const auto &const_arr = arr;

  try {
    auto val = const_arr.at(10);
    (void)val;
    FAIL("Expected aether_bounds_exception_c");
  } catch (const truk::aether::aether_bounds_exception_c &e) {
    CHECK(e.what() != nullptr);
  }
}

TEST(BoundsCheckingTests, StructArrayBoundsChecking) {
  struct test_data_t {
    std::int32_t value;
    test_data_t() : value(0) {}
  };

  class data_c : public truk::aether::struct_c<test_data_t> {
  public:
    data_c() : truk::aether::struct_c<test_data_t>() {}
  };

  truk::aether::array_c<data_c> arr(5);

  arr.at(0).get().value = 100;
  CHECK_EQUAL(100, arr.at(0).get().value);

  try {
    arr.at(5).get().value = 200;
    FAIL("Expected aether_bounds_exception_c");
  } catch (const truk::aether::aether_bounds_exception_c &e) {
    CHECK(e.what() != nullptr);
  }
}

TEST(BoundsCheckingTests, ZeroLengthArrayThrows) {
  truk::aether::array_c<truk::aether::i32_c> arr(0);
  CHECK_EQUAL(0, arr.length());

  try {
    arr.at(0) = truk::aether::i32_c(1);
    FAIL("Expected aether_bounds_exception_c");
  } catch (const truk::aether::aether_bounds_exception_c &e) {
    CHECK(e.what() != nullptr);
  }
}

TEST(BoundsCheckingTests, BoolArrayBoundsChecking) {
  truk::aether::array_c<truk::aether::bool_c> arr(3);

  arr.at(0) = truk::aether::bool_c(true);
  arr.at(2) = truk::aether::bool_c(false);

  CHECK_EQUAL(true, arr.at(0).value());
  CHECK_EQUAL(false, arr.at(2).value());

  try {
    arr.at(3) = truk::aether::bool_c(true);
    FAIL("Expected aether_bounds_exception_c");
  } catch (const truk::aether::aether_bounds_exception_c &e) {
    CHECK(e.what() != nullptr);
  }
}

TEST(BoundsCheckingTests, FloatArrayBoundsChecking) {
  truk::aether::array_c<truk::aether::r32_c> arr(10);

  arr.at(5) = truk::aether::r32_c(3.14f);
  DOUBLES_EQUAL(3.14f, arr.at(5).value(), 0.001);

  try {
    arr.at(10) = truk::aether::r32_c(2.71f);
    FAIL("Expected aether_bounds_exception_c");
  } catch (const truk::aether::aether_bounds_exception_c &e) {
    CHECK(e.what() != nullptr);
  }
}

TEST(BoundsCheckingTests, ExceptionMessageContainsIndexAndLength) {
  truk::aether::array_c<truk::aether::i32_c> arr(10);

  try {
    arr.at(15) = truk::aether::i32_c(100);
    FAIL("Expected aether_bounds_exception_c");
  } catch (const truk::aether::aether_bounds_exception_c &e) {
    std::string msg(e.what());
    CHECK(msg.find("15") != std::string::npos);
    CHECK(msg.find("10") != std::string::npos);
  }
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
