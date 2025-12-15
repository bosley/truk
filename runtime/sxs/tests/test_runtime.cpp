#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>
#include <sxs/runtime.h>
#include <sxs/types.h>

TEST_GROUP(SxsTypes){};

TEST(SxsTypes, TypeSizes) {
  CHECK_EQUAL(1, sizeof(__truk_i8));
  CHECK_EQUAL(2, sizeof(__truk_i16));
  CHECK_EQUAL(4, sizeof(__truk_i32));
  CHECK_EQUAL(8, sizeof(__truk_i64));
  CHECK_EQUAL(1, sizeof(__truk_u8));
  CHECK_EQUAL(2, sizeof(__truk_u16));
  CHECK_EQUAL(4, sizeof(__truk_u32));
  CHECK_EQUAL(8, sizeof(__truk_u64));
  CHECK_EQUAL(4, sizeof(__truk_f32));
  CHECK_EQUAL(8, sizeof(__truk_f64));
}

TEST(SxsTypes, BoolType) { CHECK_EQUAL(1, sizeof(__truk_bool)); }

TEST_GROUP(SxsBoundsCheck){};

TEST(SxsBoundsCheck, ValidIndex) {
  __truk_runtime_sxs_bounds_check(0, 10);
  __truk_runtime_sxs_bounds_check(5, 10);
  __truk_runtime_sxs_bounds_check(9, 10);
}

TEST(SxsBoundsCheck, BoundaryCondition) {
  __truk_runtime_sxs_bounds_check(0, 1);
}

TEST_GROUP(SxsAllocation){};

TEST(SxsAllocation, AllocAndFree) {
  __truk_void *ptr = __truk_runtime_sxs_alloc(100);
  CHECK(ptr != nullptr);
  __truk_runtime_sxs_free(ptr);
}

TEST(SxsAllocation, AllocArrayAndFree) {
  __truk_void *ptr = __truk_runtime_sxs_alloc_array(sizeof(__truk_i32), 10);
  CHECK(ptr != nullptr);
  __truk_runtime_sxs_free_array(ptr);
}

TEST(SxsAllocation, AllocZeroSize) {
  __truk_void *ptr = __truk_runtime_sxs_alloc(0);
  __truk_runtime_sxs_free(ptr);
}

TEST(SxsAllocation, AllocArrayZeroCount) {
  __truk_void *ptr = __truk_runtime_sxs_alloc_array(sizeof(__truk_i32), 0);
  __truk_runtime_sxs_free_array(ptr);
}

TEST_GROUP(SxsSizeof){};

TEST(SxsSizeof, BasicTypes) {
  CHECK_EQUAL(sizeof(__truk_i32),
              __truk_runtime_sxs_sizeof_type(sizeof(__truk_i32)));
  CHECK_EQUAL(sizeof(__truk_u64),
              __truk_runtime_sxs_sizeof_type(sizeof(__truk_u64)));
  CHECK_EQUAL(sizeof(__truk_f32),
              __truk_runtime_sxs_sizeof_type(sizeof(__truk_f32)));
}

TEST(SxsSizeof, PointerType) {
  CHECK_EQUAL(sizeof(__truk_void *),
              __truk_runtime_sxs_sizeof_type(sizeof(__truk_void *)));
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
