#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void sxs_panic(const char *msg, u64 len);
void sxs_bounds_check(u64 idx, u64 len);

void *sxs_alloc(u64 size);
void sxs_free(void *ptr);
void *sxs_alloc_array(u64 elem_size, u64 count);
void sxs_free_array(void *ptr);

u64 sxs_sizeof_type(u64 size);

#ifdef __cplusplus
}
#endif
