#ifndef CPE_UTILS_BASE64_H
#define CPE_UTILS_BASE64_H
#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t cpe_base64_encode(write_stream_t output, read_stream_t input);
size_t cpe_base64_decode(write_stream_t output, read_stream_t input);

#ifdef __cplusplus
}
#endif

#endif
