#include <stdio.h>
#include <stdlib.h>
#include <sxs/runtime.h>

void sxs_panic(const char *msg, u64 len) {
  fprintf(stderr, "panic: %.*s\n", (int)len, msg);
  exit(1);
}

void sxs_bounds_check(u64 idx, u64 len) {
  if (idx >= len) {
    fprintf(stderr, "panic: index out of bounds: %llu >= %llu\n",
            (unsigned long long)idx, (unsigned long long)len);
    exit(1);
  }
}

void *sxs_alloc(u64 size) { return malloc(size); }

void sxs_free(void *ptr) { free(ptr); }

void *sxs_alloc_array(u64 elem_size, u64 count) {
  return malloc(elem_size * count);
}

void sxs_free_array(void *ptr) { free(ptr); }

u64 sxs_sizeof_type(u64 size) { return size; }
