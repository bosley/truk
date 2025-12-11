#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define TRUK_PANIC(msg, len)                                                   \
  do {                                                                         \
    fprintf(stderr, "panic: %.*s\n", (int)(len), (const char *)(msg));         \
    exit(1);                                                                   \
  } while (0)

#define TRUK_BOUNDS_CHECK(idx, len)                                            \
  do {                                                                         \
    if ((idx) >= (len)) {                                                      \
      fprintf(stderr, "panic: index out of bounds: %llu >= %llu\n",            \
              (unsigned long long)(idx), (unsigned long long)(len));           \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

typedef struct {
  i32 x;
  i32 y;
} Point;

i32 add(i32 a, i32 b) { return (a + b); }

i32 main() {
  Point p = (Point){.x = 10, .y = 20};
  i32 sum = add(p.x, p.y);
  return sum;
}
