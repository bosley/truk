/**
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>
#include <string.h>
#include <sxs/ds/map.h>

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
