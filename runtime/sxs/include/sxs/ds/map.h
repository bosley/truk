/**
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 * - Modified for TRUK by bosley 2025
 */

#ifndef __TRUK_MAP_H
#define __TRUK_MAP_H

#include <string.h>

#define __TRUK_MAP_VERSION "0.1.0"

struct __truk_map_node_t;
typedef struct __truk_map_node_t __truk_map_node_t;

typedef struct {
  __truk_map_node_t **buckets;
  unsigned nbuckets, nnodes;
} __truk_map_base_t;

typedef struct {
  unsigned bucketidx;
  __truk_map_node_t *node;
} __truk_map_iter_t;

#define __truk_map_t(T)                                                        \
  struct {                                                                     \
    __truk_map_base_t base;                                                    \
    T *ref;                                                                    \
    T tmp;                                                                     \
  }

#define __truk_map_init(m) memset(m, 0, sizeof(*(m)))

#define __truk_map_deinit(m) __truk_map_deinit_(&(m)->base)

#define __truk_map_get(m, key) ((m)->ref = __truk_map_get_(&(m)->base, key))

#define __truk_map_set(m, key, value)                                          \
  ((m)->tmp = (value),                                                         \
   __truk_map_set_(&(m)->base, key, &(m)->tmp, sizeof((m)->tmp)))

#define __truk_map_remove(m, key) __truk_map_remove_(&(m)->base, key)

#define __truk_map_iter(m) __truk_map_iter_()

#define __truk_map_next(m, iter) __truk_map_next_(&(m)->base, iter)

void __truk_map_deinit_(__truk_map_base_t *m);
void *__truk_map_get_(__truk_map_base_t *m, const char *key);
int __truk_map_set_(__truk_map_base_t *m, const char *key, void *value,
                    int vsize);
void __truk_map_remove_(__truk_map_base_t *m, const char *key);
__truk_map_iter_t __truk_map_iter_(void);
const char *__truk_map_next_(__truk_map_base_t *m, __truk_map_iter_t *iter);

typedef __truk_map_t(void *) __truk_map_void_t;
typedef __truk_map_t(char *) __truk_map_str_t;
typedef __truk_map_t(int) __truk_map_int_t;
typedef __truk_map_t(char) __truk_map_char_t;
typedef __truk_map_t(float) __truk_map_float_t;
typedef __truk_map_t(double) __truk_map_double_t;

#endif
