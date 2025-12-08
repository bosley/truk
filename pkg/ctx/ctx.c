#include "ctx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ctx_t *ctx_create(ctx_t *parent, ctx_free_fn free_fn) {
  ctx_t *ctx = malloc(sizeof(ctx_t));
  if (!ctx) {
    fprintf(stderr, "Failed to allocate context\n");
    return NULL;
  }

  ctx->parent = parent;
  ctx->free_fn = free_fn;
  map_init(&ctx->data);

  return ctx;
}

void ctx_free(ctx_t *ctx) {
  if (!ctx) {
    return;
  }

  if (ctx->free_fn) {
    map_iter_t iter = map_iter(&ctx->data);
    const char *key;
    while ((key = map_next(&ctx->data, &iter))) {
      void **value = map_get(&ctx->data, key);
      if (value && *value) {
        ctx->free_fn(*value);
      }
    }
  }

  map_deinit(&ctx->data);
  free(ctx);
}

int ctx_set(ctx_t *ctx, const char *key, void *obj) {
  if (!ctx || !key || !obj) {
    return -1;
  }

  void **existing = map_get(&ctx->data, key);
  if (existing && *existing && ctx->free_fn) {
    ctx->free_fn(*existing);
  }

  if (map_set(&ctx->data, key, obj) != 0) {
    return -1;
  }

  return 0;
}

void *ctx_get(ctx_t *ctx, const char *key) {
  if (!ctx || !key) {
    return NULL;
  }

  void **value = map_get(&ctx->data, key);
  if (!value) {
    return NULL;
  }

  return *value;
}

ctx_t *ctx_get_context_if_exists(ctx_t *ctx, const char *key,
                                 bool search_parents) {
  if (!ctx || !key) {
    return NULL;
  }

  void **value = map_get(&ctx->data, key);
  if (value) {
    return ctx;
  }

  if (search_parents && ctx->parent) {
    return ctx_get_context_if_exists(ctx->parent, key, search_parents);
  }

  return NULL;
}

void ctx_remove(ctx_t *ctx, const char *key) {
  if (!ctx || !key) {
    return;
  }

  void **value = map_get(&ctx->data, key);
  if (value && *value && ctx->free_fn) {
    ctx->free_fn(*value);
  }

  map_remove(&ctx->data, key);
}
