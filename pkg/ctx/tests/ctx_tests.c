#include "../ctx.h"
#include "test.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
  int type;
  union {
    int integer;
    double real;
    char *string;
    void *data;
  } value;
} test_data_t;

void test_data_free(void *data) {
  if (!data)
    return;
  test_data_t *td = (test_data_t *)data;
  if (td->type == 2 && td->value.string) {
    free(td->value.string);
  }
  free(td);
}

test_data_t *test_data_create_int(int val) {
  test_data_t *td = malloc(sizeof(test_data_t));
  td->type = 0;
  td->value.integer = val;
  return td;
}

test_data_t *test_data_create_real(double val) {
  test_data_t *td = malloc(sizeof(test_data_t));
  td->type = 1;
  td->value.real = val;
  return td;
}

test_data_t *test_data_create_string(const char *str) {
  test_data_t *td = malloc(sizeof(test_data_t));
  td->type = 2;
  td->value.string = strdup(str);
  return td;
}

void test_ctx_create_free(void) {
  ctx_t *ctx = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(ctx);
  ASSERT_NULL(ctx->parent);
  ASSERT_EQ(ctx->data.base.nnodes, 0);
  ctx_free(ctx);
}

void test_ctx_create_with_parent(void) {
  ctx_t *parent = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(parent);

  ctx_t *child = ctx_create(parent, test_data_free);
  ASSERT_NOT_NULL(child);
  ASSERT_EQ(child->parent, parent);

  ctx_free(child);
  ctx_free(parent);
}

void test_ctx_set_get_integer(void) {
  ctx_t *ctx = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(ctx);

  test_data_t *data = test_data_create_int(42);

  int result = ctx_set(ctx, "test_key", data);
  ASSERT_EQ(result, 0);

  test_data_t *retrieved = (test_data_t *)ctx_get(ctx, "test_key");
  ASSERT_NOT_NULL(retrieved);
  ASSERT_EQ(retrieved->type, 0);
  ASSERT_EQ(retrieved->value.integer, 42);

  ctx_free(ctx);
}

void test_ctx_set_get_real(void) {
  ctx_t *ctx = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(ctx);

  test_data_t *data = test_data_create_real(3.14159);

  ctx_set(ctx, "pi", data);

  test_data_t *retrieved = (test_data_t *)ctx_get(ctx, "pi");
  ASSERT_NOT_NULL(retrieved);
  ASSERT_EQ(retrieved->type, 1);
  ASSERT_TRUE(retrieved->value.real > 3.14 && retrieved->value.real < 3.15);

  ctx_free(ctx);
}

void test_ctx_set_get_symbol(void) {
  ctx_t *ctx = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(ctx);

  test_data_t *data = test_data_create_string("test_symbol");

  ctx_set(ctx, "symbol_key", data);

  test_data_t *retrieved = (test_data_t *)ctx_get(ctx, "symbol_key");
  ASSERT_NOT_NULL(retrieved);
  ASSERT_EQ(retrieved->type, 2);
  ASSERT_NOT_NULL(retrieved->value.string);
  ASSERT_EQ(strcmp(retrieved->value.string, "test_symbol"), 0);

  ctx_free(ctx);
}

void test_ctx_overwrite_frees_old(void) {
  ctx_t *ctx = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(ctx);

  test_data_t *data1 = test_data_create_int(100);

  ctx_set(ctx, "key", data1);
  ASSERT_EQ(ctx->data.base.nnodes, 1);

  test_data_t *retrieved1 = (test_data_t *)ctx_get(ctx, "key");
  ASSERT_NOT_NULL(retrieved1);
  ASSERT_EQ(retrieved1->value.integer, 100);

  test_data_t *data2 = test_data_create_int(200);

  ctx_set(ctx, "key", data2);
  ASSERT_EQ(ctx->data.base.nnodes, 1);

  test_data_t *retrieved2 = (test_data_t *)ctx_get(ctx, "key");
  ASSERT_NOT_NULL(retrieved2);
  ASSERT_EQ(retrieved2->value.integer, 200);

  ctx_free(ctx);
}

void test_ctx_get_nonexistent(void) {
  ctx_t *ctx = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(ctx);

  test_data_t *retrieved = (test_data_t *)ctx_get(ctx, "nonexistent");
  ASSERT_NULL(retrieved);

  ctx_free(ctx);
}

void test_ctx_get_context_if_exists_current(void) {
  ctx_t *ctx = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(ctx);

  test_data_t *data = test_data_create_int(42);

  ctx_set(ctx, "key", data);

  ctx_t *found = ctx_get_context_if_exists(ctx, "key", false);
  ASSERT_NOT_NULL(found);
  ASSERT_EQ(found, ctx);

  ctx_free(ctx);
}

void test_ctx_get_context_if_exists_not_found(void) {
  ctx_t *ctx = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(ctx);

  ctx_t *found = ctx_get_context_if_exists(ctx, "nonexistent", false);
  ASSERT_NULL(found);

  ctx_free(ctx);
}

void test_ctx_get_context_if_exists_parent_search(void) {
  ctx_t *parent = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(parent);

  test_data_t *data = test_data_create_int(100);

  ctx_set(parent, "parent_key", data);

  ctx_t *child = ctx_create(parent, test_data_free);
  ASSERT_NOT_NULL(child);

  ctx_t *found = ctx_get_context_if_exists(child, "parent_key", true);
  ASSERT_NOT_NULL(found);
  ASSERT_EQ(found, parent);

  test_data_t *retrieved = (test_data_t *)ctx_get(found, "parent_key");
  ASSERT_NOT_NULL(retrieved);
  ASSERT_EQ(retrieved->value.integer, 100);

  ctx_free(child);
  ctx_free(parent);
}

void test_ctx_get_context_if_exists_no_parent_search(void) {
  ctx_t *parent = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(parent);

  test_data_t *data = test_data_create_int(100);

  ctx_set(parent, "parent_key", data);

  ctx_t *child = ctx_create(parent, test_data_free);
  ASSERT_NOT_NULL(child);

  ctx_t *found = ctx_get_context_if_exists(child, "parent_key", false);
  ASSERT_NULL(found);

  ctx_free(child);
  ctx_free(parent);
}

void test_ctx_nested_contexts_three_levels(void) {
  ctx_t *root = ctx_create(NULL, test_data_free);
  ctx_t *level1 = ctx_create(root, test_data_free);
  ctx_t *level2 = ctx_create(level1, test_data_free);

  test_data_t *data_root = test_data_create_int(1);
  test_data_t *data_level1 = test_data_create_int(2);
  test_data_t *data_level2 = test_data_create_int(3);

  ctx_set(root, "root_key", data_root);
  ctx_set(level1, "level1_key", data_level1);
  ctx_set(level2, "level2_key", data_level2);

  ctx_t *found_root = ctx_get_context_if_exists(level2, "root_key", true);
  ASSERT_NOT_NULL(found_root);
  ASSERT_EQ(found_root, root);

  ctx_t *found_level1 = ctx_get_context_if_exists(level2, "level1_key", true);
  ASSERT_NOT_NULL(found_level1);
  ASSERT_EQ(found_level1, level1);

  ctx_t *found_level2 = ctx_get_context_if_exists(level2, "level2_key", true);
  ASSERT_NOT_NULL(found_level2);
  ASSERT_EQ(found_level2, level2);

  ctx_free(level2);
  ctx_free(level1);
  ctx_free(root);
}

void test_ctx_remove(void) {
  ctx_t *ctx = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(ctx);

  test_data_t *data = test_data_create_int(42);

  ctx_set(ctx, "key", data);
  ASSERT_EQ(ctx->data.base.nnodes, 1);

  test_data_t *retrieved = (test_data_t *)ctx_get(ctx, "key");
  ASSERT_NOT_NULL(retrieved);

  ctx_remove(ctx, "key");
  ASSERT_EQ(ctx->data.base.nnodes, 0);

  retrieved = (test_data_t *)ctx_get(ctx, "key");
  ASSERT_NULL(retrieved);

  ctx_free(ctx);
}

void test_ctx_remove_nonexistent(void) {
  ctx_t *ctx = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(ctx);

  ctx_remove(ctx, "nonexistent");
  ASSERT_EQ(ctx->data.base.nnodes, 0);

  ctx_free(ctx);
}

void test_ctx_multiple_keys(void) {
  ctx_t *ctx = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(ctx);

  test_data_t *data1 = test_data_create_int(1);
  test_data_t *data2 = test_data_create_int(2);
  test_data_t *data3 = test_data_create_int(3);

  ctx_set(ctx, "key1", data1);
  ctx_set(ctx, "key2", data2);
  ctx_set(ctx, "key3", data3);

  ASSERT_EQ(ctx->data.base.nnodes, 3);

  ASSERT_EQ(((test_data_t *)ctx_get(ctx, "key1"))->value.integer, 1);
  ASSERT_EQ(((test_data_t *)ctx_get(ctx, "key2"))->value.integer, 2);
  ASSERT_EQ(((test_data_t *)ctx_get(ctx, "key3"))->value.integer, 3);

  ctx_free(ctx);
}

void test_ctx_list_object(void) {
  ctx_t *ctx = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(ctx);

  test_data_t *data = test_data_create_int(42);

  ctx_set(ctx, "simple", data);

  test_data_t *retrieved = (test_data_t *)ctx_get(ctx, "simple");
  ASSERT_NOT_NULL(retrieved);
  ASSERT_EQ(retrieved->type, 0);
  ASSERT_EQ(retrieved->value.integer, 42);

  ctx_free(ctx);
}

void test_ctx_memory_leak_check(void) {
  for (int round = 0; round < 10; round++) {
    ctx_t *ctx = ctx_create(NULL, test_data_free);
    ASSERT_NOT_NULL(ctx);

    for (int i = 0; i < 50; i++) {
      char key[32];
      snprintf(key, sizeof(key), "key_%d", i);

      test_data_t *data = test_data_create_int(i);

      ctx_set(ctx, key, data);
    }

    for (int i = 0; i < 25; i++) {
      char key[32];
      snprintf(key, sizeof(key), "key_%d", i);
      ctx_remove(ctx, key);
    }

    ctx_free(ctx);
  }
}

void test_ctx_shadowing(void) {
  ctx_t *parent = ctx_create(NULL, test_data_free);
  ctx_t *child = ctx_create(parent, test_data_free);

  test_data_t *data_parent = test_data_create_int(100);
  test_data_t *data_child = test_data_create_int(200);

  ctx_set(parent, "key", data_parent);
  ctx_set(child, "key", data_child);

  ctx_t *found_in_child = ctx_get_context_if_exists(child, "key", true);
  ASSERT_NOT_NULL(found_in_child);
  ASSERT_EQ(found_in_child, child);

  test_data_t *retrieved = (test_data_t *)ctx_get(found_in_child, "key");
  ASSERT_EQ(retrieved->value.integer, 200);

  ctx_free(child);
  ctx_free(parent);
}

void test_ctx_null_key_handling(void) {
  ctx_t *ctx = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(ctx);

  test_data_t *data = test_data_create_int(42);

  int result = ctx_set(ctx, NULL, data);
  ASSERT_EQ(result, -1);

  test_data_free(data);

  test_data_t *retrieved = (test_data_t *)ctx_get(ctx, NULL);
  ASSERT_NULL(retrieved);

  ctx_remove(ctx, NULL);

  ctx_free(ctx);
}

void test_ctx_null_object_handling(void) {
  ctx_t *ctx = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(ctx);

  int result = ctx_set(ctx, "key", NULL);
  ASSERT_EQ(result, -1);

  ctx_free(ctx);
}

void test_ctx_empty_key(void) {
  ctx_t *ctx = ctx_create(NULL, test_data_free);
  ASSERT_NOT_NULL(ctx);

  test_data_t *data = test_data_create_int(999);

  ctx_set(ctx, "", data);

  test_data_t *retrieved = (test_data_t *)ctx_get(ctx, "");
  ASSERT_NOT_NULL(retrieved);
  ASSERT_EQ(retrieved->value.integer, 999);

  ctx_free(ctx);
}

int main(void) {

  printf("Running ctx tests...\n");
  clock_t start = clock();

  int num_tests = 0;

#define TEST(test_name)                                                        \
  do {                                                                         \
    num_tests++;                                                               \
    test_name();                                                               \
  } while (0)

  TEST(test_ctx_create_free);
  TEST(test_ctx_create_with_parent);
  TEST(test_ctx_set_get_integer);
  TEST(test_ctx_set_get_real);
  TEST(test_ctx_set_get_symbol);
  TEST(test_ctx_overwrite_frees_old);
  TEST(test_ctx_get_nonexistent);
  TEST(test_ctx_get_context_if_exists_current);
  TEST(test_ctx_get_context_if_exists_not_found);
  TEST(test_ctx_get_context_if_exists_parent_search);
  TEST(test_ctx_get_context_if_exists_no_parent_search);
  TEST(test_ctx_nested_contexts_three_levels);
  TEST(test_ctx_remove);
  TEST(test_ctx_remove_nonexistent);
  TEST(test_ctx_multiple_keys);
  TEST(test_ctx_list_object);
  TEST(test_ctx_memory_leak_check);
  TEST(test_ctx_shadowing);
  TEST(test_ctx_null_key_handling);
  TEST(test_ctx_null_object_handling);
  TEST(test_ctx_empty_key);

  clock_t end = clock();
  double elapsed_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

  printf("Scanner tests passed\n");
  printf("\tTotal tests run: %d\n", num_tests);
  printf("\tTotal test time: %.2f ms\n", elapsed_ms);

  return 0;
}
