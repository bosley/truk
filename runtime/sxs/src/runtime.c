#include <stdio.h>
#include <stdlib.h>
#include <sxs/runtime.h>

void sxs_panic(const char *msg, u64 len) {
  fprintf(stderr, "panic: %.*s\n", (int)len, msg);
  exit(1);
}

i32 sxs_start(sxs_target_app_s *app) {
  if (app->has_args) {
    sxs_entry_fn_with_args entry = (sxs_entry_fn_with_args)app->entry_fn;
    return entry(app->argc, app->argv);
  } else {
    sxs_entry_fn_no_args entry = (sxs_entry_fn_no_args)app->entry_fn;
    return entry();
  }
}
