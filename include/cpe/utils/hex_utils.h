#ifndef CPE_UTILS_HEX_H
#define CPE_UTILS_HEX_H
#include "stream.h"
#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

char * cpe_hex_dup(read_stream_t input, mem_buffer_t buffer);
char * cpe_hex_dup_buf(const void * buf, size_t size, mem_buffer_t buffer);

#ifdef __cplusplus
}
#endif

#endif
