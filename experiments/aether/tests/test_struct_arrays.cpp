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

struct test_polygon_t {
  std::int32_t vertex_count;
  std::int32_t vertices[10][2];

  test_polygon_t() : vertex_count(0) {
    for (int i = 0; i < 10; ++i) {
      vertices[i][0] = 0;
      vertices[i][1] = 0;
    }
  }
};

class polygon_c : public truk::aether::struct_c<test_polygon_t> {
public:
  polygon_c() : truk::aether::struct_c<test_polygon_t>() {}
};

TEST_GROUP(StructArrayTests){void setup() override{} void teardown()
                                 override{}};

TEST(StructArrayTests, CanConstructArrayOfStructs) {
  truk::aether::array_c<point_c> arr(5);
  CHECK_EQUAL(5, arr.length());
}

TEST(StructArrayTests, ArrayOfStructsDefaultInitialized) {
  truk::aether::array_c<point_c> arr(3);

  CHECK_EQUAL(0, arr[0].get().x);
  CHECK_EQUAL(0, arr[0].get().y);
  CHECK_EQUAL(0, arr[1].get().x);
  CHECK_EQUAL(0, arr[1].get().y);
  CHECK_EQUAL(0, arr[2].get().x);
  CHECK_EQUAL(0, arr[2].get().y);
}

TEST(StructArrayTests, CanAccessArrayOfStructsElements) {
  truk::aether::array_c<point_c> arr(3);

  auto &p0 = arr[0];
  auto &p1 = arr[1];
  auto &p2 = arr[2];

  CHECK_EQUAL(0, p0.get().x);
  CHECK_EQUAL(0, p1.get().x);
  CHECK_EQUAL(0, p2.get().x);
}

TEST(StructArrayTests, CanModifyArrayOfStructsElements) {
  truk::aether::array_c<point_c> arr(3);

  arr[0].get().x = 10;
  arr[0].get().y = 20;
  arr[1].get().x = 30;
  arr[1].get().y = 40;
  arr[2].get().x = 50;
  arr[2].get().y = 60;

  CHECK_EQUAL(10, arr[0].get().x);
  CHECK_EQUAL(20, arr[0].get().y);
  CHECK_EQUAL(30, arr[1].get().x);
  CHECK_EQUAL(40, arr[1].get().y);
  CHECK_EQUAL(50, arr[2].get().x);
  CHECK_EQUAL(60, arr[2].get().y);
}

TEST(StructArrayTests, ArrayOfStructsIndependence) {
  truk::aether::array_c<point_c> arr(2);

  arr[0].get().x = 100;
  arr[1].get().x = 200;

  CHECK_EQUAL(100, arr[0].get().x);
  CHECK_EQUAL(200, arr[1].get().x);

  arr[0].get().x = 999;
  CHECK_EQUAL(999, arr[0].get().x);
  CHECK_EQUAL(200, arr[1].get().x);
}

TEST(StructArrayTests, ArrayOfNestedStructs) {
  truk::aether::array_c<rect_c> arr(2);
  CHECK_EQUAL(2, arr.length());

  CHECK_EQUAL(0, arr[0].get().top_left.x);
  CHECK_EQUAL(0, arr[0].get().top_left.y);
  CHECK_EQUAL(0, arr[0].get().bottom_right.x);
  CHECK_EQUAL(0, arr[0].get().bottom_right.y);
}

TEST(StructArrayTests, ModifyArrayOfNestedStructs) {
  truk::aether::array_c<rect_c> arr(2);

  arr[0].get().top_left.x = 0;
  arr[0].get().top_left.y = 0;
  arr[0].get().bottom_right.x = 100;
  arr[0].get().bottom_right.y = 100;

  arr[1].get().top_left.x = 50;
  arr[1].get().top_left.y = 50;
  arr[1].get().bottom_right.x = 150;
  arr[1].get().bottom_right.y = 150;

  CHECK_EQUAL(0, arr[0].get().top_left.x);
  CHECK_EQUAL(0, arr[0].get().top_left.y);
  CHECK_EQUAL(100, arr[0].get().bottom_right.x);
  CHECK_EQUAL(100, arr[0].get().bottom_right.y);

  CHECK_EQUAL(50, arr[1].get().top_left.x);
  CHECK_EQUAL(50, arr[1].get().top_left.y);
  CHECK_EQUAL(150, arr[1].get().bottom_right.x);
  CHECK_EQUAL(150, arr[1].get().bottom_right.y);
}

TEST(StructArrayTests, LargeArrayOfStructs) {
  truk::aether::array_c<point_c> arr(100);
  CHECK_EQUAL(100, arr.length());

  arr[0].get().x = 1;
  arr[99].get().x = 99;

  CHECK_EQUAL(1, arr[0].get().x);
  CHECK_EQUAL(99, arr[99].get().x);
}

TEST(StructArrayTests, ArrayOfStructsIteration) {
  truk::aether::array_c<point_c> arr(5);

  for (std::size_t i = 0; i < arr.length(); ++i) {
    arr[i].get().x = static_cast<std::int32_t>(i * 10);
    arr[i].get().y = static_cast<std::int32_t>(i * 20);
  }

  for (std::size_t i = 0; i < arr.length(); ++i) {
    CHECK_EQUAL(static_cast<std::int32_t>(i * 10), arr[i].get().x);
    CHECK_EQUAL(static_cast<std::int32_t>(i * 20), arr[i].get().y);
  }
}

TEST(StructArrayTests, StructInterfaceMethods) {
  truk::aether::array_c<point_c> arr(1);

  CHECK_EQUAL(sizeof(test_point_t), arr[0].size_bytes());
  CHECK_EQUAL(alignof(test_point_t), arr[0].alignment());
  CHECK(arr[0].data_ptr() != nullptr);
}

TEST(StructArrayTests, StructContainingArrays) {
  polygon_c poly;

  CHECK_EQUAL(0, poly.get().vertex_count);
  CHECK_EQUAL(0, poly.get().vertices[0][0]);
  CHECK_EQUAL(0, poly.get().vertices[0][1]);
}

TEST(StructArrayTests, ModifyStructContainingArrays) {
  polygon_c poly;

  poly.get().vertex_count = 3;
  poly.get().vertices[0][0] = 0;
  poly.get().vertices[0][1] = 0;
  poly.get().vertices[1][0] = 100;
  poly.get().vertices[1][1] = 0;
  poly.get().vertices[2][0] = 50;
  poly.get().vertices[2][1] = 100;

  CHECK_EQUAL(3, poly.get().vertex_count);
  CHECK_EQUAL(0, poly.get().vertices[0][0]);
  CHECK_EQUAL(0, poly.get().vertices[0][1]);
  CHECK_EQUAL(100, poly.get().vertices[1][0]);
  CHECK_EQUAL(0, poly.get().vertices[1][1]);
  CHECK_EQUAL(50, poly.get().vertices[2][0]);
  CHECK_EQUAL(100, poly.get().vertices[2][1]);
}

TEST(StructArrayTests, ArrayOfStructsContainingArrays) {
  truk::aether::array_c<polygon_c> polygons(2);
  CHECK_EQUAL(2, polygons.length());

  polygons[0].get().vertex_count = 3;
  polygons[0].get().vertices[0][0] = 10;
  polygons[0].get().vertices[0][1] = 20;
  polygons[0].get().vertices[1][0] = 30;
  polygons[0].get().vertices[1][1] = 40;
  polygons[0].get().vertices[2][0] = 50;
  polygons[0].get().vertices[2][1] = 60;

  polygons[1].get().vertex_count = 4;
  polygons[1].get().vertices[0][0] = 100;
  polygons[1].get().vertices[0][1] = 200;

  CHECK_EQUAL(3, polygons[0].get().vertex_count);
  CHECK_EQUAL(10, polygons[0].get().vertices[0][0]);
  CHECK_EQUAL(20, polygons[0].get().vertices[0][1]);
  CHECK_EQUAL(30, polygons[0].get().vertices[1][0]);
  CHECK_EQUAL(40, polygons[0].get().vertices[1][1]);

  CHECK_EQUAL(4, polygons[1].get().vertex_count);
  CHECK_EQUAL(100, polygons[1].get().vertices[0][0]);
  CHECK_EQUAL(200, polygons[1].get().vertices[0][1]);
}

TEST(StructArrayTests, IterateOverStructArraysInStruct) {
  polygon_c poly;

  poly.get().vertex_count = 5;
  for (int i = 0; i < 5; ++i) {
    poly.get().vertices[i][0] = i * 10;
    poly.get().vertices[i][1] = i * 20;
  }

  for (int i = 0; i < 5; ++i) {
    CHECK_EQUAL(i * 10, poly.get().vertices[i][0]);
    CHECK_EQUAL(i * 20, poly.get().vertices[i][1]);
  }
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
