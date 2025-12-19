/**
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 * - Modified for TRUK by bosley 2025 - Generic key support
 */

#include <stdlib.h>
#include <string.h>
#include <sxs/ds/map.h>
#include <sxs/runtime.h>

struct __truk_map_node_t {
  unsigned hash;
  void *value;
  __truk_map_node_t *next;
};

unsigned __truk_map_hash_str(const void *key, int ksize) {
  (void)ksize;
  const char *str = *(const char **)key;
  unsigned hash = 5381;
  while (*str) {
    hash = ((hash << 5) + hash) ^ *str++;
  }
  return hash;
}

unsigned __truk_map_hash_i8(const void *key, int ksize) {
  (void)ksize;
  return (unsigned)(*(const signed char *)key);
}

unsigned __truk_map_hash_i16(const void *key, int ksize) {
  (void)ksize;
  return (unsigned)(*(const short *)key);
}

unsigned __truk_map_hash_i32(const void *key, int ksize) {
  (void)ksize;
  return (unsigned)(*(const int *)key);
}

unsigned __truk_map_hash_i64(const void *key, int ksize) {
  (void)ksize;
  long long val = *(const long long *)key;
  return (unsigned)(val ^ (val >> 32));
}

unsigned __truk_map_hash_u8(const void *key, int ksize) {
  (void)ksize;
  return (unsigned)(*(const unsigned char *)key);
}

unsigned __truk_map_hash_u16(const void *key, int ksize) {
  (void)ksize;
  return (unsigned)(*(const unsigned short *)key);
}

unsigned __truk_map_hash_u32(const void *key, int ksize) {
  (void)ksize;
  return *(const unsigned int *)key;
}

unsigned __truk_map_hash_u64(const void *key, int ksize) {
  (void)ksize;
  unsigned long long val = *(const unsigned long long *)key;
  return (unsigned)(val ^ (val >> 32));
}

unsigned __truk_map_hash_f32(const void *key, int ksize) {
  (void)ksize;
  union {
    float f;
    unsigned u;
  } conv;
  conv.f = *(const float *)key;
  return conv.u;
}

unsigned __truk_map_hash_f64(const void *key, int ksize) {
  (void)ksize;
  union {
    double d;
    unsigned long long u;
  } conv;
  conv.d = *(const double *)key;
  return (unsigned)(conv.u ^ (conv.u >> 32));
}

unsigned __truk_map_hash_bool(const void *key, int ksize) {
  (void)ksize;
  return *(const unsigned char *)key;
}

int __truk_map_cmp_str(const void *a, const void *b, int ksize) {
  (void)ksize;
  const char *sa = *(const char **)a;
  const char *sb = *(const char **)b;
  return strcmp(sa, sb);
}

int __truk_map_cmp_mem(const void *a, const void *b, int ksize) {
  return memcmp(a, b, ksize);
}

static __truk_map_node_t *__truk_map_newnode(__truk_map_base_t *m,
                                             const void *key, void *value,
                                             int vsize) {
  __truk_map_node_t *node;
  int ksize = m->ksize;
  int voffset = ksize + ((sizeof(void *) - ksize) % sizeof(void *));
  node = __truk_runtime_sxs_alloc(sizeof(*node) + voffset + vsize);
  if (!node)
    return NULL;
  memcpy(node + 1, key, ksize);
  node->hash = m->hash_fn(key, ksize);
  node->value = ((char *)(node + 1)) + voffset;
  memcpy(node->value, value, vsize);
  return node;
}

static int __truk_map_bucketidx(__truk_map_base_t *m, unsigned hash) {
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
  buckets = realloc(m->buckets, sizeof(*m->buckets) * nbuckets);
  if (buckets != NULL) {
    m->buckets = buckets;
    m->nbuckets = nbuckets;
  }
  if (m->buckets) {
    memset(m->buckets, 0, sizeof(*m->buckets) * m->nbuckets);
    node = nodes;
    while (node) {
      next = node->next;
      __truk_map_addnode(m, node);
      node = next;
    }
  }
  return (buckets == NULL) ? -1 : 0;
}

static __truk_map_node_t **__truk_map_getref(__truk_map_base_t *m,
                                             const void *key) {
  unsigned hash = m->hash_fn(key, m->ksize);
  __truk_map_node_t **next;
  if (m->nbuckets > 0) {
    next = &m->buckets[__truk_map_bucketidx(m, hash)];
    while (*next) {
      if ((*next)->hash == hash &&
          m->cmp_fn((void *)(*next + 1), key, m->ksize) == 0) {
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
      __truk_runtime_sxs_free(node);
      node = next;
    }
  }
  __truk_runtime_sxs_free(m->buckets);
}

void *__truk_map_get_(__truk_map_base_t *m, const void *key) {
  __truk_map_node_t **next = __truk_map_getref(m, key);
  return next ? (*next)->value : NULL;
}

int __truk_map_set_(__truk_map_base_t *m, const void *key, void *value,
                    int vsize) {
  int n, err;
  __truk_map_node_t **next, *node;
  next = __truk_map_getref(m, key);
  if (next) {
    memcpy((*next)->value, value, vsize);
    return 0;
  }
  node = __truk_map_newnode(m, key, value, vsize);
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
    __truk_runtime_sxs_free(node);
  return -1;
}

void __truk_map_remove_(__truk_map_base_t *m, const void *key) {
  __truk_map_node_t *node;
  __truk_map_node_t **next = __truk_map_getref(m, key);
  if (next) {
    node = *next;
    *next = (*next)->next;
    __truk_runtime_sxs_free(node);
    m->nnodes--;
  }
}

__truk_map_iter_t __truk_map_iter_(void) {
  __truk_map_iter_t iter;
  iter.bucketidx = -1;
  iter.node = NULL;
  return iter;
}

void *__truk_map_next_(__truk_map_base_t *m, __truk_map_iter_t *iter) {
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
  return (void *)(iter->node + 1);
}
