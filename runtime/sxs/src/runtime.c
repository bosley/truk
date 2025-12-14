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
