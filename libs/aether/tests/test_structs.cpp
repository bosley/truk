#include "truk/aether/forms.hpp"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

struct test_point_t {
  std::int32_t x;
  std::int32_t y;

  test_point_t() : x(0), y(0) {}
  test_point_t(std::int32_t x_val, std::int32_t y_val) : x(x_val), y(y_val) {}
};

class point_c : public truk::aether::struct_c<test_point_t> {
public:
  point_c() : truk::aether::struct_c<test_point_t>() {}
  point_c(std::int32_t x, std::int32_t y)
      : truk::aether::struct_c<test_point_t>(x, y) {}
};

struct test_rect_t {
  test_point_t top_left;
  test_point_t bottom_right;

  test_rect_t() : top_left(), bottom_right() {}
  test_rect_t(test_point_t tl, test_point_t br)
      : top_left(tl), bottom_right(br) {}
};

class rect_c : public truk::aether::struct_c<test_rect_t> {
public:
  rect_c() : truk::aether::struct_c<test_rect_t>() {}
  rect_c(test_point_t tl, test_point_t br)
      : truk::aether::struct_c<test_rect_t>(tl, br) {}
};

TEST_GROUP(StructTests){void setup() override{} void teardown() override{}};

TEST(StructTests, CanConstructStruct) {
  point_c p;
  CHECK_EQUAL(0, p.get().x);
  CHECK_EQUAL(0, p.get().y);
}

TEST(StructTests, CanConstructStructWithArgs) {
  point_c p(10, 20);
  CHECK_EQUAL(10, p.get().x);
  CHECK_EQUAL(20, p.get().y);
}

TEST(StructTests, CanAccessStructMembers) {
  point_c p(5, 15);
  const auto &point = p.get();
  CHECK_EQUAL(5, point.x);
  CHECK_EQUAL(15, point.y);
}

TEST(StructTests, CanModifyStructMembers) {
  point_c p;
  p.get().x = 42;
  p.get().y = 84;

  CHECK_EQUAL(42, p.get().x);
  CHECK_EQUAL(84, p.get().y);
}

TEST(StructTests, StructSizeAndAlignment) {
  point_c p;
  CHECK_EQUAL(sizeof(test_point_t), p.size_bytes());
  CHECK_EQUAL(alignof(test_point_t), p.alignment());
}

TEST(StructTests, StructDataPtr) {
  point_c p(100, 200);
  void *ptr = p.data_ptr();
  CHECK(ptr != nullptr);

  const point_c &cp = p;
  const void *cptr = cp.data_ptr();
  CHECK(cptr != nullptr);
}

TEST(StructTests, NestedStruct) {
  test_point_t tl(0, 0);
  test_point_t br(100, 100);
  rect_c r(tl, br);

  CHECK_EQUAL(0, r.get().top_left.x);
  CHECK_EQUAL(0, r.get().top_left.y);
  CHECK_EQUAL(100, r.get().bottom_right.x);
  CHECK_EQUAL(100, r.get().bottom_right.y);
}

TEST(StructTests, ModifyNestedStruct) {
  rect_c r;
  r.get().top_left.x = 10;
  r.get().top_left.y = 20;
  r.get().bottom_right.x = 110;
  r.get().bottom_right.y = 120;

  CHECK_EQUAL(10, r.get().top_left.x);
  CHECK_EQUAL(20, r.get().top_left.y);
  CHECK_EQUAL(110, r.get().bottom_right.x);
  CHECK_EQUAL(120, r.get().bottom_right.y);
}

TEST(StructTests, NestedStructSize) {
  rect_c r;
  CHECK_EQUAL(sizeof(test_rect_t), r.size_bytes());
  CHECK_EQUAL(alignof(test_rect_t), r.alignment());
}

TEST(StructTests, MultipleStructInstances) {
  point_c p1(1, 2);
  point_c p2(3, 4);
  point_c p3(5, 6);

  CHECK_EQUAL(1, p1.get().x);
  CHECK_EQUAL(2, p1.get().y);
  CHECK_EQUAL(3, p2.get().x);
  CHECK_EQUAL(4, p2.get().y);
  CHECK_EQUAL(5, p3.get().x);
  CHECK_EQUAL(6, p3.get().y);
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
