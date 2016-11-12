#ifndef CPE_DR_MD5_H
#define CPE_DR_MD5_H
#include "cpe/utils/md5.h"
#include "dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void dr_md5_ctx_update(cpe_md5_ctx_t md5_ctx, const void * input, size_t input_capacity, LPDRMETA meta);

#ifdef __cplusplus
}
#endif

#endif
