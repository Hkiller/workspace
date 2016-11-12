#ifndef CPE_UTILS_STREAM_FILE_H
#define CPE_UTILS_STREAM_FILE_H
#include <stdio.h>
#include "stream.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

struct write_stream_file {
    struct write_stream m_stream;
    FILE * m_fp;
    error_monitor_t m_em;
};

int stream_do_write_to_file(struct write_stream * stream, const void * buf, size_t size);
int stream_do_flush_to_file(struct write_stream * stream);

void write_stream_file_init(struct write_stream_file * stream, FILE * fp, error_monitor_t em);

#define CPE_WRITE_STREAM_FILE_INITIALIZER(__fp, __em)                  \
    { CPE_WRITE_STREAM_INITIALIZER(stream_do_write_to_file, stream_do_flush_to_file), __fp, __em }

struct read_stream_file {
    struct read_stream m_stream;
    FILE * m_fp;
    error_monitor_t m_em;
};

int stream_do_read_from_file(struct read_stream * stream, void * buf, size_t size);
void read_stream_file_init(struct read_stream_file * stream, FILE * fp, error_monitor_t em);

#define CPE_READ_STREAM_FILE_INITIALIZER(__fp, __em)                        \
    { CPE_READ_STREAM_INITIALIZER(stream_do_read_from_file), __fp, __em }


#ifdef __cplusplus
}
#endif

#endif
