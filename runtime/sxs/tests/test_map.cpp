#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

extern "C" {
#include <stdio.h>
#include <string.h>
#include <sxs/ds/map.h>
}

#define __TRUK_MAP_GET_INT(m, k)                                               \
  ((m)->ref = (int *)__truk_map_get_(&(m)->base, k), (m)->ref)
#define __TRUK_MAP_GET_STR(m, k)                                               \
  ((m)->ref = (char **)__truk_map_get_(&(m)->base, k), (m)->ref)
#define __TRUK_MAP_GET_VOID(m, k)                                              \
  ((m)->ref = (void **)__truk_map_get_(&(m)->base, k), (m)->ref)
#define __TRUK_MAP_GET_FLOAT(m, k)                                             \
  ((m)->ref = (float *)__truk_map_get_(&(m)->base, k), (m)->ref)
#define __TRUK_MAP_GET_DOUBLE(m, k)                                            \
  ((m)->ref = (double *)__truk_map_get_(&(m)->base, k), (m)->ref)
#define __TRUK_MAP_GET_CHAR(m, k)                                              \
  ((m)->ref = (char *)__truk_map_get_(&(m)->base, k), (m)->ref)

TEST_GROUP(MapBasic){};

TEST(MapBasic, InitDeinit) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  CHECK_EQUAL(0, map.base.nbuckets);
  CHECK_EQUAL(0, map.base.nnodes);
  POINTERS_EQUAL(NULL, map.base.buckets);

  __truk_map_deinit(&map);
}

TEST(MapBasic, SetGetInt) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *key1 = "key1";
  int result = __truk_map_set_generic(&map, &key1, 42);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(1, map.base.nnodes);

  int *value = __TRUK_MAP_GET_INT(&map, &key1);
  CHECK(value != NULL);
  CHECK_EQUAL(42, *value);

  __truk_map_deinit(&map);
}

TEST(MapBasic, SetGetMultiple) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *k1 = "one", *k2 = "two", *k3 = "three", *k4 = "four",
             *k5 = "five";
  __truk_map_set_generic(&map, &k1, 1);
  __truk_map_set_generic(&map, &k2, 2);
  __truk_map_set_generic(&map, &k3, 3);
  __truk_map_set_generic(&map, &k4, 4);
  __truk_map_set_generic(&map, &k5, 5);

  CHECK_EQUAL(5, map.base.nnodes);

  int *v1 = __TRUK_MAP_GET_INT(&map, &k1);
  int *v2 = __TRUK_MAP_GET_INT(&map, &k2);
  int *v3 = __TRUK_MAP_GET_INT(&map, &k3);
  int *v4 = __TRUK_MAP_GET_INT(&map, &k4);
  int *v5 = __TRUK_MAP_GET_INT(&map, &k5);

  CHECK_EQUAL(1, *v1);
  CHECK_EQUAL(2, *v2);
  CHECK_EQUAL(3, *v3);
  CHECK_EQUAL(4, *v4);
  CHECK_EQUAL(5, *v5);

  __truk_map_deinit(&map);
}

TEST(MapBasic, UpdateExisting) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *key = "key";
  __truk_map_set_generic(&map, &key, 100);
  int *v1 = __TRUK_MAP_GET_INT(&map, &key);
  CHECK_EQUAL(100, *v1);

  __truk_map_set_generic(&map, &key, 200);
  int *v2 = __TRUK_MAP_GET_INT(&map, &key);
  CHECK_EQUAL(200, *v2);
  CHECK_EQUAL(1, map.base.nnodes);

  __truk_map_deinit(&map);
}

TEST(MapBasic, GetNonexistent) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *key = "nonexistent";
  int *value = __TRUK_MAP_GET_INT(&map, &key);
  POINTERS_EQUAL(NULL, value);

  __truk_map_deinit(&map);
}

TEST(MapBasic, Remove) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *k1 = "key1", *k2 = "key2", *k3 = "key3";
  __truk_map_set_generic(&map, &k1, 10);
  __truk_map_set_generic(&map, &k2, 20);
  __truk_map_set_generic(&map, &k3, 30);
  CHECK_EQUAL(3, map.base.nnodes);

  __truk_map_remove_generic(&map, &k2);
  CHECK_EQUAL(2, map.base.nnodes);

  int *v1 = __TRUK_MAP_GET_INT(&map, &k1);
  int *v2 = __TRUK_MAP_GET_INT(&map, &k2);
  int *v3 = __TRUK_MAP_GET_INT(&map, &k3);

  CHECK_EQUAL(10, *v1);
  POINTERS_EQUAL(NULL, v2);
  CHECK_EQUAL(30, *v3);

  __truk_map_deinit(&map);
}

TEST(MapBasic, RemoveNonexistent) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *k1 = "key", *k2 = "nonexistent";
  __truk_map_set_generic(&map, &k1, 42);
  CHECK_EQUAL(1, map.base.nnodes);

  __truk_map_remove_generic(&map, &k2);
  CHECK_EQUAL(1, map.base.nnodes);

  __truk_map_deinit(&map);
}

TEST(MapBasic, EmptyKey) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *key = "";
  __truk_map_set_generic(&map, &key, 999);

  int *value = __TRUK_MAP_GET_INT(&map, &key);
  CHECK(value != NULL);
  CHECK_EQUAL(999, *value);

  __truk_map_deinit(&map);
}

TEST(MapBasic, RemoveAll) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *k1 = "k1", *k2 = "k2", *k3 = "k3";
  __truk_map_set_generic(&map, &k1, 1);
  __truk_map_set_generic(&map, &k2, 2);
  __truk_map_set_generic(&map, &k3, 3);

  __truk_map_remove_generic(&map, &k1);
  __truk_map_remove_generic(&map, &k2);
  __truk_map_remove_generic(&map, &k3);

  CHECK_EQUAL(0, map.base.nnodes);
  POINTERS_EQUAL(NULL, __TRUK_MAP_GET_INT(&map, &k1));
  POINTERS_EQUAL(NULL, __TRUK_MAP_GET_INT(&map, &k2));
  POINTERS_EQUAL(NULL, __TRUK_MAP_GET_INT(&map, &k3));

  __truk_map_deinit(&map);
}

TEST(MapBasic, ReuseAfterClear) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *k1 = "first", *k2 = "second";
  __truk_map_set_generic(&map, &k1, 1);
  __truk_map_set_generic(&map, &k2, 2);

  __truk_map_deinit(&map);

  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *k3 = "third", *k4 = "fourth";
  __truk_map_set_generic(&map, &k3, 3);
  __truk_map_set_generic(&map, &k4, 4);

  CHECK_EQUAL(2, map.base.nnodes);
  int *v3 = __TRUK_MAP_GET_INT(&map, &k3);
  int *v4 = __TRUK_MAP_GET_INT(&map, &k4);
  CHECK_EQUAL(3, *v3);
  CHECK_EQUAL(4, *v4);

  __truk_map_deinit(&map);
}

TEST_GROUP(MapTypes){};

TEST(MapTypes, StringValues) {
  __truk_map_str_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  char *str1 = (char *)"hello";
  char *str2 = (char *)"world";
  char *str3 = (char *)"test";

  const char *k1 = "greeting", *k2 = "noun", *k3 = "action";
  __truk_map_set_generic(&map, &k1, str1);
  __truk_map_set_generic(&map, &k2, str2);
  __truk_map_set_generic(&map, &k3, str3);

  char **value1 = __TRUK_MAP_GET_STR(&map, &k1);
  char **value2 = __TRUK_MAP_GET_STR(&map, &k2);
  char **value3 = __TRUK_MAP_GET_STR(&map, &k3);

  CHECK(value1 != NULL);
  CHECK(value2 != NULL);
  CHECK(value3 != NULL);

  STRCMP_EQUAL("hello", *value1);
  STRCMP_EQUAL("world", *value2);
  STRCMP_EQUAL("test", *value3);

  __truk_map_deinit(&map);
}

TEST(MapTypes, PointerValues) {
  __truk_map_void_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  int data1 = 100;
  int data2 = 200;
  int data3 = 300;

  const char *k1 = "ptr1", *k2 = "ptr2", *k3 = "ptr3";
  __truk_map_set_generic(&map, &k1, &data1);
  __truk_map_set_generic(&map, &k2, &data2);
  __truk_map_set_generic(&map, &k3, &data3);

  void **ptr1 = __TRUK_MAP_GET_VOID(&map, &k1);
  void **ptr2 = __TRUK_MAP_GET_VOID(&map, &k2);
  void **ptr3 = __TRUK_MAP_GET_VOID(&map, &k3);

  CHECK(ptr1 != NULL);
  CHECK(ptr2 != NULL);
  CHECK(ptr3 != NULL);

  CHECK_EQUAL(100, *(int *)*ptr1);
  CHECK_EQUAL(200, *(int *)*ptr2);
  CHECK_EQUAL(300, *(int *)*ptr3);

  __truk_map_deinit(&map);
}

TEST(MapTypes, FloatValues) {
  __truk_map_float_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *k1 = "pi", *k2 = "e", *k3 = "phi";
  __truk_map_set_generic(&map, &k1, 3.14159f);
  __truk_map_set_generic(&map, &k2, 2.71828f);
  __truk_map_set_generic(&map, &k3, 1.61803f);

  float *pi = __TRUK_MAP_GET_FLOAT(&map, &k1);
  float *e = __TRUK_MAP_GET_FLOAT(&map, &k2);
  float *phi = __TRUK_MAP_GET_FLOAT(&map, &k3);

  CHECK(pi != NULL);
  CHECK(e != NULL);
  CHECK(phi != NULL);

  CHECK(*pi > 3.14f && *pi < 3.15f);
  CHECK(*e > 2.71f && *e < 2.72f);
  CHECK(*phi > 1.61f && *phi < 1.62f);

  __truk_map_deinit(&map);
}

TEST(MapTypes, DoubleValues) {
  __truk_map_double_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *k1 = "large", *k2 = "small";
  __truk_map_set_generic(&map, &k1, 1234567890.123456);
  __truk_map_set_generic(&map, &k2, 0.000000123456);

  double *large = __TRUK_MAP_GET_DOUBLE(&map, &k1);
  double *small = __TRUK_MAP_GET_DOUBLE(&map, &k2);

  CHECK(large != NULL);
  CHECK(small != NULL);

  CHECK(*large > 1234567890.0);
  CHECK(*small < 0.001);

  __truk_map_deinit(&map);
}

TEST(MapTypes, CharValues) {
  __truk_map_char_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *k1 = "letter_a", *k2 = "letter_b", *k3 = "letter_z";
  __truk_map_set_generic(&map, &k1, 'A');
  __truk_map_set_generic(&map, &k2, 'B');
  __truk_map_set_generic(&map, &k3, 'Z');

  char *a = __TRUK_MAP_GET_CHAR(&map, &k1);
  char *b = __TRUK_MAP_GET_CHAR(&map, &k2);
  char *z = __TRUK_MAP_GET_CHAR(&map, &k3);

  CHECK(a != NULL);
  CHECK(b != NULL);
  CHECK(z != NULL);

  CHECK_EQUAL('A', *a);
  CHECK_EQUAL('B', *b);
  CHECK_EQUAL('Z', *z);

  __truk_map_deinit(&map);
}

TEST_GROUP(MapIterator){};

TEST(MapIterator, Empty) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  __truk_map_iter_t iter = __truk_map_iter();
  void *key = __truk_map_next_generic(&map, &iter);

  POINTERS_EQUAL(NULL, key);

  __truk_map_deinit(&map);
}

TEST(MapIterator, Single) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *only = "only";
  __truk_map_set_generic(&map, &only, 42);

  __truk_map_iter_t iter = __truk_map_iter();
  const char **key_ptr = (const char **)__truk_map_next_generic(&map, &iter);

  CHECK(key_ptr != NULL);
  STRCMP_EQUAL("only", *key_ptr);
  int *v = __TRUK_MAP_GET_INT(&map, key_ptr);
  CHECK_EQUAL(42, *v);

  key_ptr = (const char **)__truk_map_next_generic(&map, &iter);
  POINTERS_EQUAL(NULL, key_ptr);

  __truk_map_deinit(&map);
}

TEST(MapIterator, Multiple) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *k1 = "alpha", *k2 = "beta", *k3 = "gamma", *k4 = "delta";
  __truk_map_set_generic(&map, &k1, 1);
  __truk_map_set_generic(&map, &k2, 2);
  __truk_map_set_generic(&map, &k3, 3);
  __truk_map_set_generic(&map, &k4, 4);

  int count = 0;
  int sum = 0;

  __truk_map_iter_t iter = __truk_map_iter();
  const char **key_ptr;
  while ((key_ptr = (const char **)__truk_map_next_generic(&map, &iter))) {
    int *value = __TRUK_MAP_GET_INT(&map, key_ptr);
    CHECK(value != NULL);
    sum += *value;
    count++;
  }

  CHECK_EQUAL(4, count);
  CHECK_EQUAL(10, sum);

  __truk_map_deinit(&map);
}

TEST(MapIterator, AfterRemove) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *k1 = "keep1", *k2 = "remove", *k3 = "keep2";
  __truk_map_set_generic(&map, &k1, 1);
  __truk_map_set_generic(&map, &k2, 2);
  __truk_map_set_generic(&map, &k3, 3);

  __truk_map_remove_generic(&map, &k2);

  int count = 0;
  __truk_map_iter_t iter = __truk_map_iter();
  const char **key_ptr;
  while ((key_ptr = (const char **)__truk_map_next_generic(&map, &iter))) {
    CHECK(strcmp(*key_ptr, "remove") != 0);
    count++;
  }

  CHECK_EQUAL(2, count);

  __truk_map_deinit(&map);
}

TEST_GROUP(MapStress){};

TEST(MapStress, ResizeBehavior) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *keys[100];
  char key_bufs[100][32];
  for (int i = 0; i < 100; i++) {
    snprintf(key_bufs[i], sizeof(key_bufs[i]), "key%d", i);
    keys[i] = key_bufs[i];
    __truk_map_set_generic(&map, &keys[i], i);
  }

  CHECK_EQUAL(100, map.base.nnodes);
  CHECK(map.base.nbuckets >= 100);

  for (int i = 0; i < 100; i++) {
    int *value = __TRUK_MAP_GET_INT(&map, &keys[i]);
    CHECK(value != NULL);
    CHECK_EQUAL(i, *value);
  }

  __truk_map_deinit(&map);
}

TEST(MapStress, CollisionHandling) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *k1 = "a", *k2 = "aa", *k3 = "aaa", *k4 = "aaaa", *k5 = "aaaaa";
  __truk_map_set_generic(&map, &k1, 1);
  __truk_map_set_generic(&map, &k2, 2);
  __truk_map_set_generic(&map, &k3, 3);
  __truk_map_set_generic(&map, &k4, 4);
  __truk_map_set_generic(&map, &k5, 5);

  int *v1 = __TRUK_MAP_GET_INT(&map, &k1);
  int *v2 = __TRUK_MAP_GET_INT(&map, &k2);
  int *v3 = __TRUK_MAP_GET_INT(&map, &k3);
  int *v4 = __TRUK_MAP_GET_INT(&map, &k4);
  int *v5 = __TRUK_MAP_GET_INT(&map, &k5);

  CHECK_EQUAL(1, *v1);
  CHECK_EQUAL(2, *v2);
  CHECK_EQUAL(3, *v3);
  CHECK_EQUAL(4, *v4);
  CHECK_EQUAL(5, *v5);

  __truk_map_deinit(&map);
}

TEST(MapStress, LongKeys) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  char long_key1[256];
  char long_key2[256];
  memset(long_key1, 'x', 255);
  memset(long_key2, 'y', 255);
  long_key1[255] = '\0';
  long_key2[255] = '\0';

  const char *k1 = long_key1, *k2 = long_key2;
  __truk_map_set_generic(&map, &k1, 111);
  __truk_map_set_generic(&map, &k2, 222);

  int *v1 = __TRUK_MAP_GET_INT(&map, &k1);
  int *v2 = __TRUK_MAP_GET_INT(&map, &k2);
  CHECK_EQUAL(111, *v1);
  CHECK_EQUAL(222, *v2);

  __truk_map_deinit(&map);
}

TEST(MapStress, StressTest) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                          __truk_map_cmp_str);

  const char *keys[1000];
  char key_bufs[1000][32];
  for (int i = 0; i < 1000; i++) {
    snprintf(key_bufs[i], sizeof(key_bufs[i]), "stress_key_%d", i);
    keys[i] = key_bufs[i];
    __truk_map_set_generic(&map, &keys[i], i * 2);
  }

  CHECK_EQUAL(1000, map.base.nnodes);

  for (int i = 0; i < 1000; i++) {
    int *value = __TRUK_MAP_GET_INT(&map, &keys[i]);
    CHECK(value != NULL);
    CHECK_EQUAL(i * 2, *value);
  }

  for (int i = 0; i < 500; i++) {
    __truk_map_remove_generic(&map, &keys[i * 2]);
  }

  CHECK_EQUAL(500, map.base.nnodes);

  __truk_map_deinit(&map);
}

TEST(MapStress, MemoryLeakCheck) {
  for (int round = 0; round < 10; round++) {
    __truk_map_int_t map;
    __truk_map_init_generic(&map, sizeof(const char *), __truk_map_hash_str,
                            __truk_map_cmp_str);

    const char *keys[100];
    char key_bufs[100][32];
    for (int i = 0; i < 100; i++) {
      snprintf(key_bufs[i], sizeof(key_bufs[i]), "key_%d", i);
      keys[i] = key_bufs[i];
      __truk_map_set_generic(&map, &keys[i], i);
    }

    for (int i = 0; i < 50; i++) {
      __truk_map_remove_generic(&map, &keys[i]);
    }

    __truk_map_deinit(&map);
  }
}

TEST_GROUP(MapIntegerKeys){};

TEST(MapIntegerKeys, I32Keys) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(int), __truk_map_hash_i32,
                          __truk_map_cmp_mem);

  int k1 = 1, k2 = 42, k3 = 100;
  __truk_map_set_generic(&map, &k1, 10);
  __truk_map_set_generic(&map, &k2, 20);
  __truk_map_set_generic(&map, &k3, 30);

  int *v1 = (int *)__truk_map_get_(&map.base, &k1);
  int *v2 = (int *)__truk_map_get_(&map.base, &k2);
  int *v3 = (int *)__truk_map_get_(&map.base, &k3);

  CHECK(v1 != NULL);
  CHECK(v2 != NULL);
  CHECK(v3 != NULL);

  CHECK_EQUAL(10, *v1);
  CHECK_EQUAL(20, *v2);
  CHECK_EQUAL(30, *v3);

  __truk_map_deinit(&map);
}

TEST(MapIntegerKeys, U64Keys) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(unsigned long long), __truk_map_hash_u64,
                          __truk_map_cmp_mem);

  unsigned long long k1 = 12345, k2 = 67890, k3 = 99999;
  __truk_map_set_generic(&map, &k1, 100);
  __truk_map_set_generic(&map, &k2, 200);
  __truk_map_set_generic(&map, &k3, 300);

  int *v1 = (int *)__truk_map_get_(&map.base, &k1);
  int *v2 = (int *)__truk_map_get_(&map.base, &k2);
  int *v3 = (int *)__truk_map_get_(&map.base, &k3);

  CHECK(v1 != NULL);
  CHECK(v2 != NULL);
  CHECK(v3 != NULL);

  CHECK_EQUAL(100, *v1);
  CHECK_EQUAL(200, *v2);
  CHECK_EQUAL(300, *v3);

  __truk_map_deinit(&map);
}

TEST(MapIntegerKeys, BoolKeys) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(bool), __truk_map_hash_bool,
                          __truk_map_cmp_mem);

  bool k_true = true, k_false = false;
  __truk_map_set_generic(&map, &k_true, 1);
  __truk_map_set_generic(&map, &k_false, 0);

  int *v_true = (int *)__truk_map_get_(&map.base, &k_true);
  int *v_false = (int *)__truk_map_get_(&map.base, &k_false);

  CHECK(v_true != NULL);
  CHECK(v_false != NULL);

  CHECK_EQUAL(1, *v_true);
  CHECK_EQUAL(0, *v_false);

  __truk_map_deinit(&map);
}

TEST(MapIntegerKeys, F32Keys) {
  __truk_map_int_t map;
  __truk_map_init_generic(&map, sizeof(float), __truk_map_hash_f32,
                          __truk_map_cmp_mem);

  float k1 = 3.14f, k2 = 2.71f, k3 = 1.41f;
  __truk_map_set_generic(&map, &k1, 100);
  __truk_map_set_generic(&map, &k2, 200);
  __truk_map_set_generic(&map, &k3, 300);

  int *v1 = (int *)__truk_map_get_(&map.base, &k1);
  int *v2 = (int *)__truk_map_get_(&map.base, &k2);
  int *v3 = (int *)__truk_map_get_(&map.base, &k3);

  CHECK(v1 != NULL);
  CHECK(v2 != NULL);
  CHECK(v3 != NULL);

  CHECK_EQUAL(100, *v1);
  CHECK_EQUAL(200, *v2);
  CHECK_EQUAL(300, *v3);

  __truk_map_deinit(&map);
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
