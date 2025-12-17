#pragma once

#include "types.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  __truk_i32 failed;
  __truk_i32 passed;
  const char *current_test_name;
  __truk_bool has_failed;
} __truk_test_context_s;

__truk_void __truk_test_fail(__truk_test_context_s *t, const char *msg);
__truk_void __truk_test_log(__truk_test_context_s *t, const char *msg);

__truk_void __truk_test_assert_i8(__truk_test_context_s *t, __truk_i8 expected,
                                  __truk_i8 actual, const char *msg);
__truk_void __truk_test_assert_i16(__truk_test_context_s *t,
                                   __truk_i16 expected, __truk_i16 actual,
                                   const char *msg);
__truk_void __truk_test_assert_i32(__truk_test_context_s *t,
                                   __truk_i32 expected, __truk_i32 actual,
                                   const char *msg);
__truk_void __truk_test_assert_i64(__truk_test_context_s *t,
                                   __truk_i64 expected, __truk_i64 actual,
                                   const char *msg);
__truk_void __truk_test_assert_u8(__truk_test_context_s *t, __truk_u8 expected,
                                  __truk_u8 actual, const char *msg);
__truk_void __truk_test_assert_u16(__truk_test_context_s *t,
                                   __truk_u16 expected, __truk_u16 actual,
                                   const char *msg);
__truk_void __truk_test_assert_u32(__truk_test_context_s *t,
                                   __truk_u32 expected, __truk_u32 actual,
                                   const char *msg);
__truk_void __truk_test_assert_u64(__truk_test_context_s *t,
                                   __truk_u64 expected, __truk_u64 actual,
                                   const char *msg);
__truk_void __truk_test_assert_f32(__truk_test_context_s *t,
                                   __truk_f32 expected, __truk_f32 actual,
                                   const char *msg);
__truk_void __truk_test_assert_f64(__truk_test_context_s *t,
                                   __truk_f64 expected, __truk_f64 actual,
                                   const char *msg);
__truk_void __truk_test_assert_bool(__truk_test_context_s *t,
                                    __truk_bool expected, __truk_bool actual,
                                    const char *msg);
__truk_void __truk_test_assert_true(__truk_test_context_s *t,
                                    __truk_bool condition, const char *msg);
__truk_void __truk_test_assert_false(__truk_test_context_s *t,
                                     __truk_bool condition, const char *msg);
__truk_void __truk_test_assert_ptr_ne_nil(__truk_test_context_s *t,
                                          __truk_void *ptr, const char *msg);
__truk_void __truk_test_assert_ptr_eq_nil(__truk_test_context_s *t,
                                          __truk_void *ptr, const char *msg);
__truk_void __truk_test_assert_bytes_eq(__truk_test_context_s *t,
                                        const __truk_u8 *expected,
                                        const __truk_u8 *actual, __truk_u64 len,
                                        const char *msg);

#ifdef __cplusplus
}
#endif
