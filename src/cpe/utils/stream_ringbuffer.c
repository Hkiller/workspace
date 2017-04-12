#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_ringbuffer.h"

int stream_do_write_to_ringbuffer(struct write_stream * input_stream, const void * buf, size_t size) {
    struct write_stream_ringbuffer * stream = (struct write_stream_ringbuffer *)input_stream;
    int total_write_size = 0;
    int once_write_size;
    void * data;
    int data_capacity;
    ringbuffer_block_t blk;

    while(total_write_size < (int)size) {
        if (stream->m_error) return -1;

        if (stream->m_last_blk == NULL) goto ALLOC_NEW_BUF;

        data_capacity = ringbuffer_block_data(stream->m_ringbuf, stream->m_last_blk, stream->m_pos, &data);
        if (data_capacity <= 0) goto ALLOC_NEW_BUF;

        assert(data);
        once_write_size = (int)size - total_write_size;
        if (once_write_size > data_capacity) {
            once_write_size = data_capacity;
        }

        memcpy(data, buf, once_write_size);

        buf = ((const char *)buf) + once_write_size;
        total_write_size += once_write_size;
        stream->m_pos +=once_write_size;

        continue;
    ALLOC_NEW_BUF:
        blk = stream->m_alloc(stream->m_alloc_ctx);
        if (blk == NULL) {
            stream->m_error = 1;
            return -1;
        }

        if (stream->m_first_blk == NULL) {
            stream->m_first_blk = stream->m_last_blk = blk;
        }
        else {
            ringbuffer_link(stream->m_ringbuf, stream->m_last_blk, blk);
            stream->m_last_blk = blk;
        }

        stream->m_pos = 0;
    }
    
    return total_write_size;
}

void write_stream_ringbuffer_init(struct write_stream_ringbuffer * stream, ringbuffer_t ringbuffer, write_stream_ringbuffer_alloc_fun_t alloc, void * alloc_ctx) {
    stream->m_stream.write = stream_do_write_to_ringbuffer;
    stream->m_stream.flush = stream_do_flush_dummy;

    stream->m_ringbuf = ringbuffer;
    stream->m_alloc = alloc;
    stream->m_alloc_ctx = alloc_ctx;
    stream->m_first_blk = NULL;
    stream->m_last_blk = NULL;
    stream->m_error = 0;
    stream->m_pos = 0;
}



