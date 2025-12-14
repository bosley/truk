#pragma once

#include <fmt/core.h>
#include <sstream>
#include <string>

namespace truk::emitc::cdef {

inline std::string emit_program_header() {
  return R"(#include <stdint.h>
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

typedef struct {
  void* data;
  u64 len;
} truk_slice_void;

typedef struct {
  u8* data;
  u64 len;
} truk_slice_u8;

static i32 __truk_argc = 0;
static char **__truk_argv = NULL;

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

static inline i32 truk_builtin_argc(void) {
  return __truk_argc;
}

static inline truk_slice_u8 truk_builtin_argv(i32 index) {
  if (index < 0 || index >= __truk_argc) {
    fprintf(stderr, "panic: argv index out of bounds: %d (argc=%d)\n", index, __truk_argc);
    exit(1);
  }
  char *arg = __truk_argv[index];
  u64 len = strlen(arg);
  return (truk_slice_u8){(u8*)arg, len};
}

)";
}

inline std::string emit_slice_typedef(const std::string &element_type,
                                      const std::string &slice_name) {
  return fmt::format("typedef struct {{\n  {}* data;\n  u64 len;\n}} {};\n\n",
                     element_type, slice_name);
}

inline std::string emit_builtin_alloc(const std::string &type_str) {
  return fmt::format("({0}*)malloc(sizeof({0}))", type_str);
}

inline std::string emit_builtin_free(const std::string &ptr_expr) {
  return fmt::format("free({})", ptr_expr);
}

inline std::string emit_builtin_alloc_array(const std::string &element_type,
                                            const std::string &slice_type,
                                            const std::string &count_expr) {
  return fmt::format("({{{0}*)malloc(sizeof({1}) * ({2})), ({2})}})",
                     element_type, element_type, count_expr);
}

inline std::string emit_builtin_free_array(const std::string &arr_expr) {
  return fmt::format("free(({}).data)", arr_expr);
}

inline std::string emit_builtin_len(const std::string &arr_expr) {
  return fmt::format("({}).len", arr_expr);
}

inline std::string emit_builtin_sizeof(const std::string &type_str) {
  return fmt::format("sizeof({})", type_str);
}

inline std::string emit_builtin_panic(const std::string &msg_expr) {
  return fmt::format("TRUK_PANIC(({0}).data, ({0}).len)", msg_expr);
}

inline std::string emit_bounds_check(const std::string &idx_expr,
                                     const std::string &len_expr) {
  return fmt::format("TRUK_BOUNDS_CHECK({}, {})", idx_expr, len_expr);
}

inline std::string indent(int level) { return std::string(level * 2, ' '); }

} // namespace truk::emitc::cdef