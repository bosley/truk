#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef int8_t __truk_i8;
typedef int16_t __truk_i16;
typedef int32_t __truk_i32;
typedef int64_t __truk_i64;
typedef uint8_t __truk_u8;
typedef uint16_t __truk_u16;
typedef uint32_t __truk_u32;
typedef uint64_t __truk_u64;
typedef float __truk_f32;
typedef double __truk_f64;
typedef bool __truk_bool;

#define __truk_void void
typedef struct moot moot;
struct moot {
};

__truk_void x();
