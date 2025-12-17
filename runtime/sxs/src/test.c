#include <sxs/test.h>

__truk_void __truk_test_fail(__truk_test_context_s *t, const char *msg) {
  t->has_failed = 1;
  t->failed++;
  fprintf(stderr, "    FAIL: %s\n", msg);
}

__truk_void __truk_test_log(__truk_test_context_s *t, const char *msg) {
  (void)t;
  printf("    LOG: %s\n", msg);
}

__truk_void __truk_test_assert_i32(__truk_test_context_s *t,
                                   __truk_i32 expected, __truk_i32 actual,
                                   const char *msg) {
  if (expected != actual) {
    fprintf(stderr, "    FAIL: Expected %d, got %d", expected, actual);
    if (msg && *msg) {
      fprintf(stderr, " - %s", msg);
    }
    fprintf(stderr, "\n");
    t->has_failed = 1;
    t->failed++;
  } else {
    t->passed++;
  }
}

__truk_void __truk_test_assert_i64(__truk_test_context_s *t,
                                   __truk_i64 expected, __truk_i64 actual,
                                   const char *msg) {
  if (expected != actual) {
    fprintf(stderr, "    FAIL: Expected %lld, got %lld", (long long)expected,
            (long long)actual);
    if (msg && *msg) {
      fprintf(stderr, " - %s", msg);
    }
    fprintf(stderr, "\n");
    t->has_failed = 1;
    t->failed++;
  } else {
    t->passed++;
  }
}

__truk_void __truk_test_assert_u32(__truk_test_context_s *t,
                                   __truk_u32 expected, __truk_u32 actual,
                                   const char *msg) {
  if (expected != actual) {
    fprintf(stderr, "    FAIL: Expected %u, got %u", expected, actual);
    if (msg && *msg) {
      fprintf(stderr, " - %s", msg);
    }
    fprintf(stderr, "\n");
    t->has_failed = 1;
    t->failed++;
  } else {
    t->passed++;
  }
}

__truk_void __truk_test_assert_u64(__truk_test_context_s *t,
                                   __truk_u64 expected, __truk_u64 actual,
                                   const char *msg) {
  if (expected != actual) {
    fprintf(stderr, "    FAIL: Expected %llu, got %llu",
            (unsigned long long)expected, (unsigned long long)actual);
    if (msg && *msg) {
      fprintf(stderr, " - %s", msg);
    }
    fprintf(stderr, "\n");
    t->has_failed = 1;
    t->failed++;
  } else {
    t->passed++;
  }
}

__truk_void __truk_test_assert_f32(__truk_test_context_s *t,
                                   __truk_f32 expected, __truk_f32 actual,
                                   const char *msg) {
  if (expected != actual) {
    fprintf(stderr, "    FAIL: Expected %f, got %f", expected, actual);
    if (msg && *msg) {
      fprintf(stderr, " - %s", msg);
    }
    fprintf(stderr, "\n");
    t->has_failed = 1;
    t->failed++;
  } else {
    t->passed++;
  }
}

__truk_void __truk_test_assert_f64(__truk_test_context_s *t,
                                   __truk_f64 expected, __truk_f64 actual,
                                   const char *msg) {
  if (expected != actual) {
    fprintf(stderr, "    FAIL: Expected %lf, got %lf", expected, actual);
    if (msg && *msg) {
      fprintf(stderr, " - %s", msg);
    }
    fprintf(stderr, "\n");
    t->has_failed = 1;
    t->failed++;
  } else {
    t->passed++;
  }
}

__truk_void __truk_test_assert_bool(__truk_test_context_s *t,
                                    __truk_bool expected, __truk_bool actual,
                                    const char *msg) {
  if (expected != actual) {
    fprintf(stderr, "    FAIL: Expected %s, got %s",
            expected ? "true" : "false", actual ? "true" : "false");
    if (msg && *msg) {
      fprintf(stderr, " - %s", msg);
    }
    fprintf(stderr, "\n");
    t->has_failed = 1;
    t->failed++;
  } else {
    t->passed++;
  }
}

__truk_void __truk_test_assert_true(__truk_test_context_s *t,
                                    __truk_bool condition, const char *msg) {
  if (!condition) {
    fprintf(stderr, "    FAIL: Expected true, got false");
    if (msg && *msg) {
      fprintf(stderr, " - %s", msg);
    }
    fprintf(stderr, "\n");
    t->has_failed = 1;
    t->failed++;
  } else {
    t->passed++;
  }
}

__truk_void __truk_test_assert_false(__truk_test_context_s *t,
                                     __truk_bool condition, const char *msg) {
  if (condition) {
    fprintf(stderr, "    FAIL: Expected false, got true");
    if (msg && *msg) {
      fprintf(stderr, " - %s", msg);
    }
    fprintf(stderr, "\n");
    t->has_failed = 1;
    t->failed++;
  } else {
    t->passed++;
  }
}

__truk_void __truk_test_assert_ptr_ne_nil(__truk_test_context_s *t,
                                          __truk_void *ptr, const char *msg) {
  if (ptr == (__truk_void *)0) {
    fprintf(stderr, "    FAIL: Expected non-nil pointer, got nil");
    if (msg && *msg) {
      fprintf(stderr, " - %s", msg);
    }
    fprintf(stderr, "\n");
    t->has_failed = 1;
    t->failed++;
  } else {
    t->passed++;
  }
}

__truk_void __truk_test_assert_ptr_eq_nil(__truk_test_context_s *t,
                                          __truk_void *ptr, const char *msg) {
  if (ptr != (__truk_void *)0) {
    fprintf(stderr, "    FAIL: Expected nil pointer, got %p", ptr);
    if (msg && *msg) {
      fprintf(stderr, " - %s", msg);
    }
    fprintf(stderr, "\n");
    t->has_failed = 1;
    t->failed++;
  } else {
    t->passed++;
  }
}
