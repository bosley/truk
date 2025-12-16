#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int8_t __truk_i8;
typedef int16_t __truk_i16;
typedef int32_t __truk_i32;
typedef int64_t __truk_i64;
typedef uint8_t __truk_u8;
typedef uint16_t __truk_u16;
typedef uint32_t __truk_u32;
typedef uint64_t __truk_u64;
typedef float __truk_f32;
typedef double __truk_f64;
typedef bool __truk_bool;

#define __truk_void void
#ifdef __cplusplus
extern "C" {
#endif

__truk_void __truk_runtime_sxs_panic(const char *msg, __truk_u64 len);

static inline __truk_void __truk_runtime_sxs_bounds_check(__truk_u64 idx,
                                                          __truk_u64 len) {
  if (idx >= len) {
    fprintf(stderr, "panic: index out of bounds: %llu >= %llu\n",
            (unsigned long long)idx, (unsigned long long)len);
    exit(1);
  }
}

static inline __truk_void *__truk_runtime_sxs_alloc(__truk_u64 size) {
  return malloc(size);
}

static inline __truk_void __truk_runtime_sxs_free(__truk_void *ptr) {
  free(ptr);
}

static inline __truk_void *__truk_runtime_sxs_alloc_array(__truk_u64 elem_size,
                                                          __truk_u64 count) {
  return malloc(elem_size * count);
}

static inline __truk_void __truk_runtime_sxs_free_array(__truk_void *ptr) {
  free(ptr);
}

static inline __truk_u64 __truk_runtime_sxs_sizeof_type(__truk_u64 size) {
  return size;
}

typedef __truk_i32 (*__truk_runtime_sxs_entry_fn_no_args)(__truk_void);
typedef __truk_i32 (*__truk_runtime_sxs_entry_fn_with_args)(__truk_i32 argc,
                                                            __truk_i8 **argv);

typedef struct {
  __truk_void *entry_fn;
  __truk_bool has_args;
  __truk_i32 argc;
  __truk_i8 **argv;
} __truk_runtime_sxs_target_app_s;

__truk_i32 __truk_runtime_sxs_start(__truk_runtime_sxs_target_app_s *app);

#ifdef __cplusplus
}
#endif
#define TRUK_PANIC(msg, len) __truk_runtime_sxs_panic((msg), (len))
#define TRUK_BOUNDS_CHECK(idx, len)                                            \
  __truk_runtime_sxs_bounds_check((idx), (len))

#define TRUK_DEFER_SCOPE_BEGIN() do {
#define TRUK_DEFER_SCOPE_END(...)                                              \
  }                                                                            \
  while (0)                                                                    \
    ;                                                                          \
  __VA_ARGS__
#define TRUK_ANONYMOUS(body)                                                   \
  do {                                                                         \
    body                                                                       \
  } while (0)

__truk_void __truk_runtime_sxs_panic(const char *msg, __truk_u64 len) {
  fprintf(stderr, "panic: %.*s\n", (int)len, msg);
  exit(1);
}

__truk_i32 __truk_runtime_sxs_start(__truk_runtime_sxs_target_app_s *app) {
  if (app->has_args) {
    __truk_runtime_sxs_entry_fn_with_args entry =
        (__truk_runtime_sxs_entry_fn_with_args)app->entry_fn;
    return entry(app->argc, app->argv);
  } else {
    __truk_runtime_sxs_entry_fn_no_args entry =
        (__truk_runtime_sxs_entry_fn_no_args)app->entry_fn;
    return entry();
  }
}
/**
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef MAP_H
#define MAP_H

#define MAP_VERSION "0.1.0"

struct map_node_t;
typedef struct map_node_t map_node_t;

typedef struct {
  map_node_t **buckets;
  unsigned nbuckets, nnodes;
} map_base_t;

typedef struct {
  unsigned bucketidx;
  map_node_t *node;
} map_iter_t;

#define map_t(T)                                                               \
  struct {                                                                     \
    map_base_t base;                                                           \
    T *ref;                                                                    \
    T tmp;                                                                     \
  }

#define map_init(m) memset(m, 0, sizeof(*(m)))

#define map_deinit(m) map_deinit_(&(m)->base)

#define map_get(m, key) ((m)->ref = map_get_(&(m)->base, key))

#define map_set(m, key, value)                                                 \
  ((m)->tmp = (value), map_set_(&(m)->base, key, &(m)->tmp, sizeof((m)->tmp)))

#define map_remove(m, key) map_remove_(&(m)->base, key)

#define map_iter(m) map_iter_()

#define map_next(m, iter) map_next_(&(m)->base, iter)

void map_deinit_(map_base_t *m);
void *map_get_(map_base_t *m, const char *key);
int map_set_(map_base_t *m, const char *key, void *value, int vsize);
void map_remove_(map_base_t *m, const char *key);
map_iter_t map_iter_(void);
const char *map_next_(map_base_t *m, map_iter_t *iter);

typedef map_t(void *) map_void_t;
typedef map_t(char *) map_str_t;
typedef map_t(int) map_int_t;
typedef map_t(char) map_char_t;
typedef map_t(float) map_float_t;
typedef map_t(double) map_double_t;

#endif
/**
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

struct map_node_t {
  unsigned hash;
  void *value;
  map_node_t *next;
  /* char key[]; */
  /* char value[]; */
};

static unsigned map_hash(const char *str) {
  unsigned hash = 5381;
  while (*str) {
    hash = ((hash << 5) + hash) ^ *str++;
  }
  return hash;
}

static map_node_t *map_newnode(const char *key, void *value, int vsize) {
  map_node_t *node;
  int ksize = strlen(key) + 1;
  int voffset = ksize + ((sizeof(void *) - ksize) % sizeof(void *));
  node = malloc(sizeof(*node) + voffset + vsize);
  if (!node)
    return NULL;
  memcpy(node + 1, key, ksize);
  node->hash = map_hash(key);
  node->value = ((char *)(node + 1)) + voffset;
  memcpy(node->value, value, vsize);
  return node;
}

static int map_bucketidx(map_base_t *m, unsigned hash) {
  /* If the implementation is changed to allow a non-power-of-2 bucket count,
   * the line below should be changed to use mod instead of AND */
  return hash & (m->nbuckets - 1);
}

static void map_addnode(map_base_t *m, map_node_t *node) {
  int n = map_bucketidx(m, node->hash);
  node->next = m->buckets[n];
  m->buckets[n] = node;
}

static int map_resize(map_base_t *m, int nbuckets) {
  map_node_t *nodes, *node, *next;
  map_node_t **buckets;
  int i;
  /* Chain all nodes together */
  nodes = NULL;
  i = m->nbuckets;
  while (i--) {
    node = (m->buckets)[i];
    while (node) {
      next = node->next;
      node->next = nodes;
      nodes = node;
      node = next;
    }
  }
  /* Reset buckets */
  buckets = realloc(m->buckets, sizeof(*m->buckets) * nbuckets);
  if (buckets != NULL) {
    m->buckets = buckets;
    m->nbuckets = nbuckets;
  }
  if (m->buckets) {
    memset(m->buckets, 0, sizeof(*m->buckets) * m->nbuckets);
    /* Re-add nodes to buckets */
    node = nodes;
    while (node) {
      next = node->next;
      map_addnode(m, node);
      node = next;
    }
  }
  /* Return error code if realloc() failed */
  return (buckets == NULL) ? -1 : 0;
}

static map_node_t **map_getref(map_base_t *m, const char *key) {
  unsigned hash = map_hash(key);
  map_node_t **next;
  if (m->nbuckets > 0) {
    next = &m->buckets[map_bucketidx(m, hash)];
    while (*next) {
      if ((*next)->hash == hash && !strcmp((char *)(*next + 1), key)) {
        return next;
      }
      next = &(*next)->next;
    }
  }
  return NULL;
}

void map_deinit_(map_base_t *m) {
  map_node_t *next, *node;
  int i;
  i = m->nbuckets;
  while (i--) {
    node = m->buckets[i];
    while (node) {
      next = node->next;
      free(node);
      node = next;
    }
  }
  free(m->buckets);
}

void *map_get_(map_base_t *m, const char *key) {
  map_node_t **next = map_getref(m, key);
  return next ? (*next)->value : NULL;
}

int map_set_(map_base_t *m, const char *key, void *value, int vsize) {
  int n, err;
  map_node_t **next, *node;
  /* Find & replace existing node */
  next = map_getref(m, key);
  if (next) {
    memcpy((*next)->value, value, vsize);
    return 0;
  }
  /* Add new node */
  node = map_newnode(key, value, vsize);
  if (node == NULL)
    goto fail;
  if (m->nnodes >= m->nbuckets) {
    n = (m->nbuckets > 0) ? (m->nbuckets << 1) : 1;
    err = map_resize(m, n);
    if (err)
      goto fail;
  }
  map_addnode(m, node);
  m->nnodes++;
  return 0;
fail:
  if (node)
    free(node);
  return -1;
}

void map_remove_(map_base_t *m, const char *key) {
  map_node_t *node;
  map_node_t **next = map_getref(m, key);
  if (next) {
    node = *next;
    *next = (*next)->next;
    free(node);
    m->nnodes--;
  }
}

map_iter_t map_iter_(void) {
  map_iter_t iter;
  iter.bucketidx = -1;
  iter.node = NULL;
  return iter;
}

const char *map_next_(map_base_t *m, map_iter_t *iter) {
  if (iter->node) {
    iter->node = iter->node->next;
    if (iter->node == NULL)
      goto nextBucket;
  } else {
  nextBucket:
    do {
      if (++iter->bucketidx >= m->nbuckets) {
        return NULL;
      }
      iter->node = m->buckets[iter->bucketidx];
    } while (iter->node == NULL);
  }
  return (char *)(iter->node + 1);
}
typedef struct {
  __truk_void *data;
  __truk_u64 len;
} truk_slice_void;

typedef map_t(__truk_i32) __truk_map___truk_i32;

__truk_i32 truk_main_0() {
  __truk_map___truk_i32 m = ({
    __truk_map___truk_i32 __tmp;
    map_init(&__tmp);
    __tmp;
  });
  {
    (m).tmp = 10;
    map_set_(&(m).base, "a", &(m).tmp, sizeof((m).tmp));
  }
  {
    (m).tmp = 20;
    map_set_(&(m).base, "b", &(m).tmp, sizeof((m).tmp));
  }
  {
    (m).tmp = 20;
    map_set_(&(m).base, "c", &(m).tmp, sizeof((m).tmp));
  }
  map_remove(&(m), "b");
  __truk_i32 *ptr_a = map_get(&(m), "a");
  __truk_i32 *ptr_b = map_get(&(m), "b");
  __truk_i32 *ptr_c = map_get(&(m), "c");
  __truk_i32 result = 0;
  if ((ptr_a != NULL)) {
    result = (result + (*ptr_a));
  }
  if ((ptr_b == NULL)) {
    result = (result + 20);
  }
  if ((ptr_c != NULL)) {
    result = (result + (*ptr_c));
  }
  map_deinit(&(m));
  return result;
}

int main(int argc, char **argv) {
  __truk_runtime_sxs_target_app_s app = {.entry_fn = (__truk_void *)truk_main_0,
                                         .has_args = false,
                                         .argc = argc,
                                         .argv = (__truk_i8 **)argv};
  return __truk_runtime_sxs_start(&app);
}
