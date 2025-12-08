#ifndef SXS_CTX_H
#define SXS_CTX_H

#include "map/map.h"
#include <stdbool.h>

/*
The CTX is a way to allocate some given object type with a custom destruction
process in a layered way such-that the context itself owns the data. This way
we can be sure we are freeing all our assets within nestable scopes.

hoist allows us to push-up a key into a parent context. this will overwrite the
object in the parent context if it exists, checking should be done manually if
it matters.

i mde this with the intention that as we work with symbol tables and interpretrs
in truk that we ensure we have the primary storage means for operating and
maintaining object lifetimes.
*/

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

int ctx_hoist(ctx_t *ctx, const char *key);

#endif
