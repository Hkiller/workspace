#ifndef CPE_XCALC_TOKEN_H
#define CPE_XCALC_TOKEN_H
#include "cpe/utils/stream.h"
#include "xcalc_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int xtoken_try_to_int32(xtoken_t token, int32_t * r);
int xtoken_try_to_uint32(xtoken_t token, uint32_t * r);
int xtoken_try_to_int64(xtoken_t token, int64_t * r);
int xtoken_try_to_uint64(xtoken_t token, uint64_t * r);
int xtoken_try_to_float(xtoken_t token, float * r);
int xtoken_try_to_double(xtoken_t token, double * r);    
int xtoken_try_to_bool(xtoken_t token, uint8_t * r);
const char * xtoken_try_to_str(xtoken_t token);
const char * xtoken_to_str(xtoken_t token, char * buf, size_t buf_size);
const char * xtoken_to_str_with_buffer(xtoken_t token, mem_buffer_t buffer);

xtoken_data_type_t xtoken_data_type(xtoken_t token);

int xtoken_cmp(xtoken_t l, xtoken_t r);

void xtoken_dump(write_stream_t s, xtoken_t token);

xtoken_t xtoken_it_next(xtoken_it_t token_it);

#ifdef __cplusplus
}
#endif

#endif
