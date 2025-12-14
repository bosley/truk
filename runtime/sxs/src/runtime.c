#include <stdio.h>
#include <stdlib.h>
#include <sxs/runtime.h>

__truk_void __truk_runtime_sxs_panic(const char *msg, __truk_u64 len) {
  fprintf(stderr, "panic: %.*s\n", (int)len, msg);
  exit(1);
}

__truk_i32 __truk_runtime_sxs_start(__truk_runtime_sxs_target_app_s *app) {
  if (app->has_args) {
    __truk_runtime_sxs_entry_fn_with_args entry = (__truk_runtime_sxs_entry_fn_with_args)app->entry_fn;
    return entry(app->argc, app->argv);
  } else {
    __truk_runtime_sxs_entry_fn_no_args entry = (__truk_runtime_sxs_entry_fn_no_args)app->entry_fn;
    return entry();
  }
}
