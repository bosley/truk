#pragma once

#include "types.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

__truk_void __truk_runtime_sxs_panic(const char *msg, __truk_u64 len);

static inline __truk_void __truk_runtime_sxs_bounds_check(__truk_u64 idx, __truk_u64 len) {
  if (idx >= len) {
    fprintf(stderr, "panic: index out of bounds: %llu >= %llu\n",
            (unsigned long long)idx, (unsigned long long)len);
    exit(1);
  }
}

static inline __truk_void *__truk_runtime_sxs_alloc(__truk_u64 size) { return malloc(size); }

static inline __truk_void __truk_runtime_sxs_free(__truk_void *ptr) { free(ptr); }

static inline __truk_void *__truk_runtime_sxs_alloc_array(__truk_u64 elem_size, __truk_u64 count) {
  return malloc(elem_size * count);
}

static inline __truk_void __truk_runtime_sxs_free_array(__truk_void *ptr) { free(ptr); }

static inline __truk_u64 __truk_runtime_sxs_sizeof_type(__truk_u64 size) { return size; }

typedef __truk_i32 (*__truk_runtime_sxs_entry_fn_no_args)(__truk_void);
typedef __truk_i32 (*__truk_runtime_sxs_entry_fn_with_args)(__truk_i32 argc, __truk_i8 **argv);

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
