#pragma once

#include "types.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void sxs_panic(const char *msg, u64 len);

static inline void sxs_bounds_check(u64 idx, u64 len) {
  if (idx >= len) {
    fprintf(stderr, "panic: index out of bounds: %llu >= %llu\n",
            (unsigned long long)idx, (unsigned long long)len);
    exit(1);
  }
}

static inline void *sxs_alloc(u64 size) { return malloc(size); }

static inline void sxs_free(void *ptr) { free(ptr); }

static inline void *sxs_alloc_array(u64 elem_size, u64 count) {
  return malloc(elem_size * count);
}

static inline void sxs_free_array(void *ptr) { free(ptr); }

static inline u64 sxs_sizeof_type(u64 size) { return size; }

typedef i32 (*sxs_entry_fn_no_args)(void);
typedef i32 (*sxs_entry_fn_with_args)(i32 argc, i8 **argv);

typedef struct {
  void *entry_fn;
  bool has_args;
  i32 argc;
  i8 **argv;
} sxs_target_app_s;

i32 sxs_start(sxs_target_app_s *app);

#ifdef __cplusplus
}
#endif
