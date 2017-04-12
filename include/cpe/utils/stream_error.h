#ifndef CPE_UTILS_STREAM_ERROR_H
#define CPE_UTILS_STREAM_ERROR_H
#include "stream.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

struct write_stream_error {
    struct write_stream m_stream;
    error_monitor_t m_em;
    size_t m_size;
    char m_buf[128];
};

int stream_do_write_to_error(struct write_stream * stream, const void * buf, size_t size);
int stream_do_flush_to_error(struct write_stream * stream);
void write_stream_error_init(struct write_stream_error * stream, error_monitor_t em, error_level_t level);

#define CPE_WRITE_STREAM_ERROR_INITIALIZER(__em)                  \
    { CPE_WRITE_STREAM_INITIALIZER(stream_do_write_to_error, stream_do_flush_to_error), __em }

#ifdef __cplusplus
}
#endif

#endif
