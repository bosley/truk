#ifndef SXS_CTX_H
#define SXS_CTX_H

#include "map/map.h"
#include <stdbool.h>

typedef void (*ctx_free_fn)(void *data);

typedef struct ctx_s {
  struct ctx_s *parent;
  map_void_t data;
  ctx_free_fn free_fn;
} ctx_t;

ctx_t *ctx_create(ctx_t *parent, ctx_free_fn free_fn);

void ctx_free(ctx_t *ctx);

int ctx_set(ctx_t *ctx, const char *key, void *obj);

void *ctx_get(ctx_t *ctx, const char *key);

ctx_t *ctx_get_context_if_exists(ctx_t *ctx, const char *key,
                                 bool search_parents);

void ctx_remove(ctx_t *ctx, const char *key);

#endif
