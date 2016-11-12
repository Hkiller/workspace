#ifndef CPE_UTILS_STREAM_RINGBUFFER_H
#define CPE_UTILS_STREAM_RINGBUFFER_H
#include "stream.h"
#include "ringbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef ringbuffer_block_t (*write_stream_ringbuffer_alloc_fun_t)(void * ctx);

struct write_stream_ringbuffer {
    struct write_stream m_stream;
    ringbuffer_t m_ringbuf;
    write_stream_ringbuffer_alloc_fun_t m_alloc;
    void * m_alloc_ctx;
    ringbuffer_block_t m_first_blk;
    ringbuffer_block_t m_last_blk;
    int m_error;
    int m_pos;
};

int stream_do_write_to_ringbuffer(struct write_stream * stream, const void * buf, size_t size);
void write_stream_ringbuffer_init(
    struct write_stream_ringbuffer * stream, ringbuffer_t ringbuffer, write_stream_ringbuffer_alloc_fun_t alloc, void * alloc_ctx);

#define CPE_WRITE_STREAM_RINGBUFFER_INITIALIZER(__ringbuffer, __alloc, __alloc_ctx) \
    { CPE_WRITE_STREAM_INITIALIZER(stream_do_write_to_ringbuffer, stream_do_flush_dummy), __ringbuffer, __alloc, __alloc_ctx, NULL, NULL, 0, 0 }


#ifdef __cplusplus
}
#endif

#endif
