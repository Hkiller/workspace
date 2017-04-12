#ifndef CPE_UTILS_STREAM_BUFFER_H
#define CPE_UTILS_STREAM_BUFFER_H
#include "stream.h"
#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct write_stream_buffer {
    struct write_stream m_stream;
    struct mem_buffer * m_buffer;
};

int stream_do_write_to_buffer(struct write_stream * stream, const void * buf, size_t size);

void write_stream_buffer_init(struct write_stream_buffer * stream, struct mem_buffer * buffer);

#define CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer) \
    { CPE_WRITE_STREAM_INITIALIZER(stream_do_write_to_buffer, stream_do_flush_dummy), buffer }

#ifdef __cplusplus
}
#endif

#endif
