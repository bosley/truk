#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

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
#define TRUK_BOUNDS_CHECK(idx, len) __truk_runtime_sxs_bounds_check((idx), (len))

#define TRUK_DEFER_SCOPE_BEGIN() do {
#define TRUK_DEFER_SCOPE_END(...) } while(0); __VA_ARGS__
#define TRUK_ANONYMOUS(body) do { body } while(0)

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
 * - Modified for TRUK by bosley 2025
 */

#ifndef __TRUK_MAP_H
#define __TRUK_MAP_H


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
/**
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */


struct __truk_map_node_t {
  unsigned hash;
  void *value;
  __truk_map_node_t *next;
  /* char key[]; */
  /* char value[]; */
};

static unsigned __truk_map_hash(const char *str) {
  unsigned hash = 5381;
  while (*str) {
    hash = ((hash << 5) + hash) ^ *str++;
  }
  return hash;
}

static __truk_map_node_t *__truk_map_newnode(const char *key, void *value,
                                             int vsize) {
  __truk_map_node_t *node;
  int ksize = strlen(key) + 1;
  int voffset = ksize + ((sizeof(void *) - ksize) % sizeof(void *));
  node = malloc(sizeof(*node) + voffset + vsize);
  if (!node)
    return NULL;
  memcpy(node + 1, key, ksize);
  node->hash = __truk_map_hash(key);
  node->value = ((char *)(node + 1)) + voffset;
  memcpy(node->value, value, vsize);
  return node;
}

static int __truk_map_bucketidx(__truk_map_base_t *m, unsigned hash) {
  /* If the implementation is changed to allow a non-power-of-2 bucket count,
   * the line below should be changed to use mod instead of AND */
  return hash & (m->nbuckets - 1);
}

static void __truk_map_addnode(__truk_map_base_t *m, __truk_map_node_t *node) {
  int n = __truk_map_bucketidx(m, node->hash);
  node->next = m->buckets[n];
  m->buckets[n] = node;
}

static int __truk_map_resize(__truk_map_base_t *m, int nbuckets) {
  __truk_map_node_t *nodes, *node, *next;
  __truk_map_node_t **buckets;
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
      __truk_map_addnode(m, node);
      node = next;
    }
  }
  /* Return error code if realloc() failed */
  return (buckets == NULL) ? -1 : 0;
}

static __truk_map_node_t **__truk_map_getref(__truk_map_base_t *m,
                                             const char *key) {
  unsigned hash = __truk_map_hash(key);
  __truk_map_node_t **next;
  if (m->nbuckets > 0) {
    next = &m->buckets[__truk_map_bucketidx(m, hash)];
    while (*next) {
      if ((*next)->hash == hash && !strcmp((char *)(*next + 1), key)) {
        return next;
      }
      next = &(*next)->next;
    }
  }
  return NULL;
}

void __truk_map_deinit_(__truk_map_base_t *m) {
  __truk_map_node_t *next, *node;
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

void *__truk_map_get_(__truk_map_base_t *m, const char *key) {
  __truk_map_node_t **next = __truk_map_getref(m, key);
  return next ? (*next)->value : NULL;
}

int __truk_map_set_(__truk_map_base_t *m, const char *key, void *value,
                    int vsize) {
  int n, err;
  __truk_map_node_t **next, *node;
  /* Find & replace existing node */
  next = __truk_map_getref(m, key);
  if (next) {
    memcpy((*next)->value, value, vsize);
    return 0;
  }
  /* Add new node */
  node = __truk_map_newnode(key, value, vsize);
  if (node == NULL)
    goto fail;
  if (m->nnodes >= m->nbuckets) {
    n = (m->nbuckets > 0) ? (m->nbuckets << 1) : 1;
    err = __truk_map_resize(m, n);
    if (err)
      goto fail;
  }
  __truk_map_addnode(m, node);
  m->nnodes++;
  return 0;
fail:
  if (node)
    free(node);
  return -1;
}

void __truk_map_remove_(__truk_map_base_t *m, const char *key) {
  __truk_map_node_t *node;
  __truk_map_node_t **next = __truk_map_getref(m, key);
  if (next) {
    node = *next;
    *next = (*next)->next;
    free(node);
    m->nnodes--;
  }
}

__truk_map_iter_t __truk_map_iter_(void) {
  __truk_map_iter_t iter;
  iter.bucketidx = -1;
  iter.node = NULL;
  return iter;
}

const char *__truk_map_next_(__truk_map_base_t *m, __truk_map_iter_t *iter) {
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
  __truk_void* data;
  __truk_u64 len;
} truk_slice_void;

typedef struct DataA DataA;
struct DataA {
  __truk_i32 value;
  __truk_i32 _internal_state;
};

typedef struct LoggerX LoggerX;
struct LoggerX {
  __truk_u8* prefix;
  __truk_bool _enabled;
  __truk_i32 _count;
};

typedef struct ServiceB ServiceB;
struct ServiceB {
  __truk_u8* name;
  LoggerX _logger;
  __truk_bool _initialized;
};

static __truk_i32 _compute_internal(__truk_i32 x) {
  return (x * 3);
}
static __truk_bool _validate_data(DataA* d) {
  return ((*d)._internal_state >= 0);
}
DataA data_a_create(__truk_i32 v) {
  return (DataA){.value = v, ._internal_state = _compute_internal(v)};
}
__truk_i32 data_a_process(DataA* d) {
  if ((!_validate_data(d))) {
    return (-1);
  }
  (*d)._internal_state = _compute_internal(((*d).value + 5));
  return (*d)._internal_state;
}
__truk_i32 data_a_get_value(DataA* d) {
  return (*d).value;
}
__truk_void data_a_reset(DataA* d) {
  (*d)._internal_state = 0;
}
DataA api_create_data(__truk_i32 v) {
  return data_a_create(v);
}
__truk_i32 api_process_data(DataA* d) {
  return data_a_process(d);
}
__truk_i32 api_get_value(DataA* d) {
  return data_a_get_value(d);
}
__truk_void api_reset_data(DataA* d) {
  data_a_reset(d);
}
static __truk_i32 _global_log_level = 1;
static __truk_void _format_message(LoggerX* logger, __truk_u8* msg) {
  if ((*logger)._enabled) {
    printf("[%s] %s\n", (*logger).prefix, msg);
    (*logger)._count = ((*logger)._count + 1);
  }
}
LoggerX logger_x_create(__truk_u8* prefix) {
  return (LoggerX){.prefix = prefix, ._enabled = true, ._count = 0};
}
static __truk_void _adjust_log_level(__truk_i32 level) {
  _global_log_level = level;
}
__truk_void logger_x_log(LoggerX* logger, __truk_u8* msg) {
  if ((_global_log_level > 0)) {
    _format_message(logger, msg);
  }
}
__truk_void logger_x_disable(LoggerX* logger) {
  (*logger)._enabled = false;
}
__truk_i32 logger_x_get_count(LoggerX* logger) {
  return (*logger)._count;
}
__truk_void logger_x_set_level(__truk_i32 level) {
  _adjust_log_level(level);
}
__truk_void logger_x_reset(LoggerX* logger) {
  (*logger)._count = 0;
  (*logger)._enabled = true;
}
LoggerX logger_api_create(__truk_u8* prefix) {
  return logger_x_create(prefix);
}
__truk_void logger_api_log(LoggerX* logger, __truk_u8* msg) {
  logger_x_log(logger, msg);
}
__truk_void logger_api_disable(LoggerX* logger) {
  logger_x_disable(logger);
}
__truk_i32 logger_api_get_count(LoggerX* logger) {
  return logger_x_get_count(logger);
}
__truk_void logger_api_set_level(__truk_i32 level) {
  logger_x_set_level(level);
}
__truk_void logger_api_reset(LoggerX* logger) {
  logger_x_reset(logger);
}
static __truk_void _init_service(ServiceB* svc) {
  (*svc)._logger = logger_api_create((*svc).name);
  (*svc)._initialized = true;
  logger_api_log((&(*svc)._logger), "Service initialized");
}
ServiceB service_b_create(__truk_u8* name) {
  ServiceB svc = (ServiceB){.name = name, ._logger = logger_api_create("temp"), ._initialized = false};
  _init_service((&svc));
  return svc;
}
static __truk_void _log_operation(ServiceB* svc, __truk_u8* msg) {
  if ((*svc)._initialized) {
    logger_api_log((&(*svc)._logger), msg);
  }
}
__truk_i32 service_b_execute(ServiceB* svc, __truk_i32 value) {
  _log_operation(svc, "Executing operation");
  return (value * 2);
}
__truk_i32 service_b_get_log_count(ServiceB* svc) {
  return logger_api_get_count((&(*svc)._logger));
}
__truk_void service_b_shutdown(ServiceB* svc) {
  _log_operation(svc, "Shutting down");
  logger_api_disable((&(*svc)._logger));
}
ServiceB api_create_service(__truk_u8* name) {
  return service_b_create(name);
}
__truk_i32 api_execute(ServiceB* svc, __truk_i32 value) {
  return service_b_execute(svc, value);
}
__truk_i32 api_get_log_count(ServiceB* svc) {
  return service_b_get_log_count(svc);
}
__truk_void api_shutdown(ServiceB* svc) {
  service_b_shutdown(svc);
}
__truk_i32 truk_main_0() {
  DataA data = api_create_data(10);
  __truk_i32 processed = api_process_data((&data));
  ServiceB svc = api_create_service("MainService");
  __truk_i32 result = api_execute((&svc), processed);
  __truk_i32 log_count = api_get_log_count((&svc));
  api_shutdown((&svc));
  return result;
}

int main(int argc, char** argv) {
  __truk_runtime_sxs_target_app_s app = {
    .entry_fn = (__truk_void*)truk_main_0,
    .has_args = false,
    .argc = argc,
    .argv = (__truk_i8**)argv
  };
  return __truk_runtime_sxs_start(&app);
}
