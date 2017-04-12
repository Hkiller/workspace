#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_error.h"

static void stream_do_commit_to_error(struct write_stream_error * es) {
    assert(es->m_size < sizeof(es->m_buf));
    if (es->m_size > 0) {
        es->m_buf[es->m_size] = 0;
        cpe_error_do_notify(es->m_em, "%s", es->m_buf);
        es->m_size = 0;
    }
}

int stream_do_write_to_error(struct write_stream * stream, const void * input, size_t size) {
    struct write_stream_error * es = (struct write_stream_error *)stream;
    size_t pos = 0;
    const char * buf = (const char *)input;

    while(pos < size) {
        if(buf[pos] == '\n') {
            stream_do_commit_to_error(es);
        }
        else {
            if (es->m_size + 1 >= sizeof(es->m_buf)) {
                stream_do_commit_to_error(es);
            }

            es->m_buf[es->m_size++] = buf[pos];
        }

        ++pos;
    }

    return (int)size;
}

int stream_do_flush_to_error(struct write_stream * stream) {
    struct write_stream_error * es = (struct write_stream_error *)stream;
    int r = (int)es->m_size;
    stream_do_commit_to_error(es);
    return r;
}

void write_stream_error_init(struct write_stream_error * stream, error_monitor_t em, error_level_t level) {
    struct write_stream_error * es = (struct write_stream_error *)stream;

    es->m_em = em;
    es->m_size = 0;
    es->m_em->m_curent_location.m_level = level;
    es->m_em->m_curent_location.m_errno = -1;
}
