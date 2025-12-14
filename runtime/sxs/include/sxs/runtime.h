#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void sxs_panic(const char *msg, u64 len);
void sxs_bounds_check(u64 idx, u64 len);

#ifdef __cplusplus
}
#endif
