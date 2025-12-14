#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <sxs/runtime.h>
#include <sxs/types.h>

TEST_GROUP(SxsTypes){};

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

TEST_GROUP(SxsBoundsCheck){};

TEST(SxsBoundsCheck, ValidIndex) {
  sxs_bounds_check(0, 10);
  sxs_bounds_check(5, 10);
  sxs_bounds_check(9, 10);
}

TEST(SxsBoundsCheck, BoundaryCondition) { sxs_bounds_check(0, 1); }

TEST_GROUP(SxsAllocation){};

TEST(SxsAllocation, AllocAndFree) {
  void *ptr = sxs_alloc(100);
  CHECK(ptr != nullptr);
  sxs_free(ptr);
}

TEST(SxsAllocation, AllocArrayAndFree) {
  void *ptr = sxs_alloc_array(sizeof(i32), 10);
  CHECK(ptr != nullptr);
  sxs_free_array(ptr);
}

TEST(SxsAllocation, AllocZeroSize) {
  void *ptr = sxs_alloc(0);
  sxs_free(ptr);
}

TEST(SxsAllocation, AllocArrayZeroCount) {
  void *ptr = sxs_alloc_array(sizeof(i32), 0);
  sxs_free_array(ptr);
}

TEST_GROUP(SxsSizeof){};

TEST(SxsSizeof, BasicTypes) {
  CHECK_EQUAL(sizeof(i32), sxs_sizeof_type(sizeof(i32)));
  CHECK_EQUAL(sizeof(u64), sxs_sizeof_type(sizeof(u64)));
  CHECK_EQUAL(sizeof(f32), sxs_sizeof_type(sizeof(f32)));
}

TEST(SxsSizeof, PointerType) {
  CHECK_EQUAL(sizeof(void *), sxs_sizeof_type(sizeof(void *)));
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
