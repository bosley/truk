#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

#define TRUK_PANIC(msg, len) do { \
  fprintf(stderr, "panic: %.*s\n", (int)(len), (const char*)(msg)); \
  exit(1); \
} while(0)

#define TRUK_BOUNDS_CHECK(idx, len) do { \
  if ((idx) >= (len)) { \
    fprintf(stderr, "panic: index out of bounds: %llu >= %llu\n", \
            (unsigned long long)(idx), (unsigned long long)(len)); \
    exit(1); \
  } \
} while(0)

#define TRUK_DEFER_SCOPE_BEGIN() do {
#define TRUK_DEFER_SCOPE_END(...) } while(0); __VA_ARGS__
#define TRUK_ANONYMOUS(body) do { body } while(0)

static inline void truk_bounds_check(u64 idx, u64 len) {
  if (idx >= len) {
    fprintf(stderr, "panic: index out of bounds: %llu >= %llu\n", 
            (unsigned long long)idx, (unsigned long long)len);
    exit(1);
  }
}

#include <stdio.h>
#include "filehelper.h"

typedef struct {
  void* data;
  u64 len;
} truk_slice_void;

i64 get_file_size(i8* path) {
  return read_file_size(path);
}
i32 get_first_byte(i8* path) {
  return read_first_byte(path);
}
i32 get_number_from_file(i8* path) {
  return read_file_as_number(path);
}
i32 main() {
  i64 size = get_file_size("test_data/input.txt");
  if ((size < 0)) {
    return 1;
  }
  i32 first = get_first_byte("test_data/input.txt");
  if ((first < 0)) {
    return 1;
  }
  i32 num = get_number_from_file("test_data/input.txt");
  if ((num != 42)) {
    return 1;
  }
  return 0;
}
