#pragma once

#include "types.h"

void sxs_panic(const char *msg, u64 len);
void sxs_bounds_check(u64 idx, u64 len);
