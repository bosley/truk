#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

extern "C" {
#include <stdio.h>
#include <string.h>
#include <sxs/ds/map.h>
}

#define MAP_GET_INT(m, k) ((m)->ref = (int *)map_get_(&(m)->base, k), (m)->ref)
#define MAP_GET_STR(m, k)                                                      \
  ((m)->ref = (char **)map_get_(&(m)->base, k), (m)->ref)
#define MAP_GET_VOID(m, k)                                                     \
  ((m)->ref = (void **)map_get_(&(m)->base, k), (m)->ref)
#define MAP_GET_FLOAT(m, k)                                                    \
  ((m)->ref = (float *)map_get_(&(m)->base, k), (m)->ref)
#define MAP_GET_DOUBLE(m, k)                                                   \
  ((m)->ref = (double *)map_get_(&(m)->base, k), (m)->ref)
#define MAP_GET_CHAR(m, k)                                                     \
  ((m)->ref = (char *)map_get_(&(m)->base, k), (m)->ref)

TEST_GROUP(MapBasic){};

TEST(MapBasic, InitDeinit) {
  map_int_t map;
  map_init(&map);

  CHECK_EQUAL(0, map.base.nbuckets);
  CHECK_EQUAL(0, map.base.nnodes);
  POINTERS_EQUAL(NULL, map.base.buckets);

  map_deinit(&map);
}

TEST(MapBasic, SetGetInt) {
  map_int_t map;
  map_init(&map);

  int result = map_set(&map, "key1", 42);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(1, map.base.nnodes);

  int *value = MAP_GET_INT(&map, "key1");
  CHECK(value != NULL);
  CHECK_EQUAL(42, *value);

  map_deinit(&map);
}

TEST(MapBasic, SetGetMultiple) {
  map_int_t map;
  map_init(&map);

  map_set(&map, "one", 1);
  map_set(&map, "two", 2);
  map_set(&map, "three", 3);
  map_set(&map, "four", 4);
  map_set(&map, "five", 5);

  CHECK_EQUAL(5, map.base.nnodes);

  int *v1 = MAP_GET_INT(&map, "one");
  int *v2 = MAP_GET_INT(&map, "two");
  int *v3 = MAP_GET_INT(&map, "three");
  int *v4 = MAP_GET_INT(&map, "four");
  int *v5 = MAP_GET_INT(&map, "five");

  CHECK_EQUAL(1, *v1);
  CHECK_EQUAL(2, *v2);
  CHECK_EQUAL(3, *v3);
  CHECK_EQUAL(4, *v4);
  CHECK_EQUAL(5, *v5);

  map_deinit(&map);
}

TEST(MapBasic, UpdateExisting) {
  map_int_t map;
  map_init(&map);

  map_set(&map, "key", 100);
  int *v1 = MAP_GET_INT(&map, "key");
  CHECK_EQUAL(100, *v1);

  map_set(&map, "key", 200);
  int *v2 = MAP_GET_INT(&map, "key");
  CHECK_EQUAL(200, *v2);
  CHECK_EQUAL(1, map.base.nnodes);

  map_deinit(&map);
}

TEST(MapBasic, GetNonexistent) {
  map_int_t map;
  map_init(&map);

  int *value = MAP_GET_INT(&map, "nonexistent");
  POINTERS_EQUAL(NULL, value);

  map_deinit(&map);
}

TEST(MapBasic, Remove) {
  map_int_t map;
  map_init(&map);

  map_set(&map, "key1", 10);
  map_set(&map, "key2", 20);
  map_set(&map, "key3", 30);
  CHECK_EQUAL(3, map.base.nnodes);

  map_remove(&map, "key2");
  CHECK_EQUAL(2, map.base.nnodes);

  int *v1 = MAP_GET_INT(&map, "key1");
  int *v2 = MAP_GET_INT(&map, "key2");
  int *v3 = MAP_GET_INT(&map, "key3");

  CHECK_EQUAL(10, *v1);
  POINTERS_EQUAL(NULL, v2);
  CHECK_EQUAL(30, *v3);

  map_deinit(&map);
}

TEST(MapBasic, RemoveNonexistent) {
  map_int_t map;
  map_init(&map);

  map_set(&map, "key", 42);
  CHECK_EQUAL(1, map.base.nnodes);

  map_remove(&map, "nonexistent");
  CHECK_EQUAL(1, map.base.nnodes);

  map_deinit(&map);
}

TEST(MapBasic, EmptyKey) {
  map_int_t map;
  map_init(&map);

  map_set(&map, "", 999);

  int *value = MAP_GET_INT(&map, "");
  CHECK(value != NULL);
  CHECK_EQUAL(999, *value);

  map_deinit(&map);
}

TEST(MapBasic, RemoveAll) {
  map_int_t map;
  map_init(&map);

  map_set(&map, "k1", 1);
  map_set(&map, "k2", 2);
  map_set(&map, "k3", 3);

  map_remove(&map, "k1");
  map_remove(&map, "k2");
  map_remove(&map, "k3");

  CHECK_EQUAL(0, map.base.nnodes);
  POINTERS_EQUAL(NULL, MAP_GET_INT(&map, "k1"));
  POINTERS_EQUAL(NULL, MAP_GET_INT(&map, "k2"));
  POINTERS_EQUAL(NULL, MAP_GET_INT(&map, "k3"));

  map_deinit(&map);
}

TEST(MapBasic, ReuseAfterClear) {
  map_int_t map;
  map_init(&map);

  map_set(&map, "first", 1);
  map_set(&map, "second", 2);

  map_deinit(&map);

  map_init(&map);

  map_set(&map, "third", 3);
  map_set(&map, "fourth", 4);

  CHECK_EQUAL(2, map.base.nnodes);
  int *v3 = MAP_GET_INT(&map, "third");
  int *v4 = MAP_GET_INT(&map, "fourth");
  CHECK_EQUAL(3, *v3);
  CHECK_EQUAL(4, *v4);

  map_deinit(&map);
}

TEST_GROUP(MapTypes){};

TEST(MapTypes, StringValues) {
  map_str_t map;
  map_init(&map);

  char *str1 = (char *)"hello";
  char *str2 = (char *)"world";
  char *str3 = (char *)"test";

  map_set(&map, "greeting", str1);
  map_set(&map, "noun", str2);
  map_set(&map, "action", str3);

  char **value1 = MAP_GET_STR(&map, "greeting");
  char **value2 = MAP_GET_STR(&map, "noun");
  char **value3 = MAP_GET_STR(&map, "action");

  CHECK(value1 != NULL);
  CHECK(value2 != NULL);
  CHECK(value3 != NULL);

  STRCMP_EQUAL("hello", *value1);
  STRCMP_EQUAL("world", *value2);
  STRCMP_EQUAL("test", *value3);

  map_deinit(&map);
}

TEST(MapTypes, PointerValues) {
  map_void_t map;
  map_init(&map);

  int data1 = 100;
  int data2 = 200;
  int data3 = 300;

  map_set(&map, "ptr1", &data1);
  map_set(&map, "ptr2", &data2);
  map_set(&map, "ptr3", &data3);

  void **ptr1 = MAP_GET_VOID(&map, "ptr1");
  void **ptr2 = MAP_GET_VOID(&map, "ptr2");
  void **ptr3 = MAP_GET_VOID(&map, "ptr3");

  CHECK(ptr1 != NULL);
  CHECK(ptr2 != NULL);
  CHECK(ptr3 != NULL);

  CHECK_EQUAL(100, *(int *)*ptr1);
  CHECK_EQUAL(200, *(int *)*ptr2);
  CHECK_EQUAL(300, *(int *)*ptr3);

  map_deinit(&map);
}

TEST(MapTypes, FloatValues) {
  map_float_t map;
  map_init(&map);

  map_set(&map, "pi", 3.14159f);
  map_set(&map, "e", 2.71828f);
  map_set(&map, "phi", 1.61803f);

  float *pi = MAP_GET_FLOAT(&map, "pi");
  float *e = MAP_GET_FLOAT(&map, "e");
  float *phi = MAP_GET_FLOAT(&map, "phi");

  CHECK(pi != NULL);
  CHECK(e != NULL);
  CHECK(phi != NULL);

  CHECK(*pi > 3.14f && *pi < 3.15f);
  CHECK(*e > 2.71f && *e < 2.72f);
  CHECK(*phi > 1.61f && *phi < 1.62f);

  map_deinit(&map);
}

TEST(MapTypes, DoubleValues) {
  map_double_t map;
  map_init(&map);

  map_set(&map, "large", 1234567890.123456);
  map_set(&map, "small", 0.000000123456);

  double *large = MAP_GET_DOUBLE(&map, "large");
  double *small = MAP_GET_DOUBLE(&map, "small");

  CHECK(large != NULL);
  CHECK(small != NULL);

  CHECK(*large > 1234567890.0);
  CHECK(*small < 0.001);

  map_deinit(&map);
}

TEST(MapTypes, CharValues) {
  map_char_t map;
  map_init(&map);

  map_set(&map, "letter_a", 'A');
  map_set(&map, "letter_b", 'B');
  map_set(&map, "letter_z", 'Z');

  char *a = MAP_GET_CHAR(&map, "letter_a");
  char *b = MAP_GET_CHAR(&map, "letter_b");
  char *z = MAP_GET_CHAR(&map, "letter_z");

  CHECK(a != NULL);
  CHECK(b != NULL);
  CHECK(z != NULL);

  CHECK_EQUAL('A', *a);
  CHECK_EQUAL('B', *b);
  CHECK_EQUAL('Z', *z);

  map_deinit(&map);
}

TEST_GROUP(MapIterator){};

TEST(MapIterator, Empty) {
  map_int_t map;
  map_init(&map);

  map_iter_t iter = map_iter(&map);
  const char *key = map_next(&map, &iter);

  POINTERS_EQUAL(NULL, key);

  map_deinit(&map);
}

TEST(MapIterator, Single) {
  map_int_t map;
  map_init(&map);

  map_set(&map, "only", 42);

  map_iter_t iter = map_iter(&map);
  const char *key = map_next(&map, &iter);

  CHECK(key != NULL);
  STRCMP_EQUAL("only", key);
  int *v = MAP_GET_INT(&map, key);
  CHECK_EQUAL(42, *v);

  key = map_next(&map, &iter);
  POINTERS_EQUAL(NULL, key);

  map_deinit(&map);
}

TEST(MapIterator, Multiple) {
  map_int_t map;
  map_init(&map);

  map_set(&map, "alpha", 1);
  map_set(&map, "beta", 2);
  map_set(&map, "gamma", 3);
  map_set(&map, "delta", 4);

  int count = 0;
  int sum = 0;

  map_iter_t iter = map_iter(&map);
  const char *key;
  while ((key = map_next(&map, &iter))) {
    int *value = MAP_GET_INT(&map, key);
    CHECK(value != NULL);
    sum += *value;
    count++;
  }

  CHECK_EQUAL(4, count);
  CHECK_EQUAL(10, sum);

  map_deinit(&map);
}

TEST(MapIterator, AfterRemove) {
  map_int_t map;
  map_init(&map);

  map_set(&map, "keep1", 1);
  map_set(&map, "remove", 2);
  map_set(&map, "keep2", 3);

  map_remove(&map, "remove");

  int count = 0;
  map_iter_t iter = map_iter(&map);
  const char *key;
  while ((key = map_next(&map, &iter))) {
    CHECK(strcmp(key, "remove") != 0);
    count++;
  }

  CHECK_EQUAL(2, count);

  map_deinit(&map);
}

TEST_GROUP(MapStress){};

TEST(MapStress, ResizeBehavior) {
  map_int_t map;
  map_init(&map);

  for (int i = 0; i < 100; i++) {
    char key[32];
    snprintf(key, sizeof(key), "key%d", i);
    map_set(&map, key, i);
  }

  CHECK_EQUAL(100, map.base.nnodes);
  CHECK(map.base.nbuckets >= 100);

  for (int i = 0; i < 100; i++) {
    char key[32];
    snprintf(key, sizeof(key), "key%d", i);
    int *value = MAP_GET_INT(&map, key);
    CHECK(value != NULL);
    CHECK_EQUAL(i, *value);
  }

  map_deinit(&map);
}

TEST(MapStress, CollisionHandling) {
  map_int_t map;
  map_init(&map);

  map_set(&map, "a", 1);
  map_set(&map, "aa", 2);
  map_set(&map, "aaa", 3);
  map_set(&map, "aaaa", 4);
  map_set(&map, "aaaaa", 5);

  int *v1 = MAP_GET_INT(&map, "a");
  int *v2 = MAP_GET_INT(&map, "aa");
  int *v3 = MAP_GET_INT(&map, "aaa");
  int *v4 = MAP_GET_INT(&map, "aaaa");
  int *v5 = MAP_GET_INT(&map, "aaaaa");

  CHECK_EQUAL(1, *v1);
  CHECK_EQUAL(2, *v2);
  CHECK_EQUAL(3, *v3);
  CHECK_EQUAL(4, *v4);
  CHECK_EQUAL(5, *v5);

  map_deinit(&map);
}

TEST(MapStress, LongKeys) {
  map_int_t map;
  map_init(&map);

  char long_key1[256];
  char long_key2[256];
  memset(long_key1, 'x', 255);
  memset(long_key2, 'y', 255);
  long_key1[255] = '\0';
  long_key2[255] = '\0';

  map_set(&map, long_key1, 111);
  map_set(&map, long_key2, 222);

  int *v1 = MAP_GET_INT(&map, long_key1);
  int *v2 = MAP_GET_INT(&map, long_key2);
  CHECK_EQUAL(111, *v1);
  CHECK_EQUAL(222, *v2);

  map_deinit(&map);
}

TEST(MapStress, StressTest) {
  map_int_t map;
  map_init(&map);

  for (int i = 0; i < 1000; i++) {
    char key[32];
    snprintf(key, sizeof(key), "stress_key_%d", i);
    map_set(&map, key, i * 2);
  }

  CHECK_EQUAL(1000, map.base.nnodes);

  for (int i = 0; i < 1000; i++) {
    char key[32];
    snprintf(key, sizeof(key), "stress_key_%d", i);
    int *value = MAP_GET_INT(&map, key);
    CHECK(value != NULL);
    CHECK_EQUAL(i * 2, *value);
  }

  for (int i = 0; i < 500; i++) {
    char key[32];
    snprintf(key, sizeof(key), "stress_key_%d", i * 2);
    map_remove(&map, key);
  }

  CHECK_EQUAL(500, map.base.nnodes);

  map_deinit(&map);
}

TEST(MapStress, MemoryLeakCheck) {
  for (int round = 0; round < 10; round++) {
    map_int_t map;
    map_init(&map);

    for (int i = 0; i < 100; i++) {
      char key[32];
      snprintf(key, sizeof(key), "key_%d", i);
      map_set(&map, key, i);
    }

    for (int i = 0; i < 50; i++) {
      char key[32];
      snprintf(key, sizeof(key), "key_%d", i);
      map_remove(&map, key);
    }

    map_deinit(&map);
  }
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
