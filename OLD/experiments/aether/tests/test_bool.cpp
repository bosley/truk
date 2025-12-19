#include "truk/aether/forms.hpp"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

TEST_GROUP(BoolTests){void setup() override{} void teardown() override{}};

TEST(BoolTests, CanConstructTrue) {
  truk::aether::bool_c val(true);
  CHECK_EQUAL(true, val.value());
}

TEST(BoolTests, CanConstructFalse) {
  truk::aether::bool_c val(false);
  CHECK_EQUAL(false, val.value());
}

TEST(BoolTests, LogicalAnd) {
  truk::aether::bool_c t(true);
  truk::aether::bool_c f(false);

  auto result1 = t && t;
  CHECK_EQUAL(true, result1.value());

  auto result2 = t && f;
  CHECK_EQUAL(false, result2.value());

  auto result3 = f && t;
  CHECK_EQUAL(false, result3.value());

  auto result4 = f && f;
  CHECK_EQUAL(false, result4.value());
}

TEST(BoolTests, LogicalOr) {
  truk::aether::bool_c t(true);
  truk::aether::bool_c f(false);

  auto result1 = t || t;
  CHECK_EQUAL(true, result1.value());

  auto result2 = t || f;
  CHECK_EQUAL(true, result2.value());

  auto result3 = f || t;
  CHECK_EQUAL(true, result3.value());

  auto result4 = f || f;
  CHECK_EQUAL(false, result4.value());
}

TEST(BoolTests, LogicalNot) {
  truk::aether::bool_c t(true);
  truk::aether::bool_c f(false);

  auto result1 = !t;
  CHECK_EQUAL(false, result1.value());

  auto result2 = !f;
  CHECK_EQUAL(true, result2.value());
}

TEST(BoolTests, Equality) {
  truk::aether::bool_c t1(true);
  truk::aether::bool_c t2(true);
  truk::aether::bool_c f1(false);
  truk::aether::bool_c f2(false);

  CHECK_EQUAL(true, t1 == t2);
  CHECK_EQUAL(true, f1 == f2);
  CHECK_EQUAL(false, t1 == f1);
  CHECK_EQUAL(false, f1 == t1);
}

TEST(BoolTests, ComplexExpression) {
  truk::aether::bool_c a(true);
  truk::aether::bool_c b(false);
  truk::aether::bool_c c(true);

  auto result = (a && c) || b;
  CHECK_EQUAL(true, result.value());

  auto result2 = (a && b) || (!c);
  CHECK_EQUAL(false, result2.value());
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
