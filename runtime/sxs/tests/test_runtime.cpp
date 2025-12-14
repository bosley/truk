#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <sxs/runtime.h>
#include <sxs/types.h>

TEST_GROUP(SxsTypes) {};

TEST(SxsTypes, TypeSizes) {
  CHECK_EQUAL(1, sizeof(i8));
  CHECK_EQUAL(2, sizeof(i16));
  CHECK_EQUAL(4, sizeof(i32));
  CHECK_EQUAL(8, sizeof(i64));
  CHECK_EQUAL(1, sizeof(u8));
  CHECK_EQUAL(2, sizeof(u16));
  CHECK_EQUAL(4, sizeof(u32));
  CHECK_EQUAL(8, sizeof(u64));
  CHECK_EQUAL(4, sizeof(f32));
  CHECK_EQUAL(8, sizeof(f64));
}

TEST(SxsTypes, BoolType) { CHECK_EQUAL(1, sizeof(bool)); }

TEST_GROUP(SxsBoundsCheck) {};

TEST(SxsBoundsCheck, ValidIndex) {
  sxs_bounds_check(0, 10);
  sxs_bounds_check(5, 10);
  sxs_bounds_check(9, 10);
}

TEST(SxsBoundsCheck, BoundaryCondition) { sxs_bounds_check(0, 1); }

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
