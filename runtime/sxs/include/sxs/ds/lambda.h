#pragma once

#include "../types.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  void *fn_ptr;
  __truk_u32 param_count;
  __truk_bool has_return;
} __truk_lambda_t;

typedef struct {
  void *fn_ptr;
  void *env;
  __truk_u64 env_size;
  __truk_u32 param_count;
  __truk_bool has_return;
} __truk_closure_t;

#define __truk_lambda_init(lambda, func)                                       \
  do {                                                                         \
    (lambda)->fn_ptr = (void *)(func);                                         \
  } while (0)

static inline __truk_closure_t *__truk_closure_alloc(__truk_u64 env_size) {
  __truk_closure_t *closure =
      (__truk_closure_t *)malloc(sizeof(__truk_closure_t));
  closure->env = malloc(env_size);
  closure->env_size = env_size;
  return closure;
}

static inline void __truk_closure_free(__truk_closure_t *closure) {
  if (closure) {
    free(closure->env);
    free(closure);
  }
}

#ifdef __cplusplus
}
#endif