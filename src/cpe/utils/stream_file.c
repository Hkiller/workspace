#include <errno.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_file.h"
#include "cpe/utils/file.h"

int stream_do_write_to_file(struct write_stream * stream, const void * buf, size_t size) {
    int rv;
    struct write_stream_file * fstream = (struct write_stream_file *)stream;

    if (fstream == NULL || fstream->m_fp == NULL)  return -1;

    rv = (int)fwrite(buf, 1, size, fstream->m_fp);

    if (ferror(fstream->m_fp)) {
        CPE_ERROR_EX(fstream->m_em, errno, "fwrite error!");
    }
    if (feof(fstream->m_fp)) {
        CPE_ERROR_EX(fstream->m_em, errno, "fwrite reach eof!");
    }

    return rv;
}

int stream_do_flush_to_file(struct write_stream * stream) {
    int rv;
    struct write_stream_file * fstream = (struct write_stream_file *)stream;

    if (fstream == NULL || fstream->m_fp == NULL)  return -1;

    rv = fflush(fstream->m_fp);

    if (rv != 0) {
        CPE_ERROR_EX(fstream->m_em, errno, "fflush fail, errno=%d (%s)!", errno, strerror(errno));
    }

    return rv;
}

void write_stream_file_init(struct write_stream_file * stream, FILE * fp, error_monitor_t em) {
    stream->m_stream.write = stream_do_write_to_file;
    stream->m_stream.flush = stream_do_flush_to_file;
    stream->m_fp = fp;
    stream->m_em = em;
}

int stream_do_read_from_file(struct read_stream * stream, void * buf, size_t size) {
    int rv;
    struct read_stream_file * fstream = (struct read_stream_file *)stream;

    if (fstream == NULL || fstream->m_fp == NULL)  return -1;

    rv = (int)fread(buf, 1, size, fstream->m_fp);

    if (ferror(fstream->m_fp)) {
        CPE_ERROR_EX(fstream->m_em, errno, "fread error, errno=%d (%s)!", errno, strerror(errno));
    }

    return rv;
}

void read_stream_file_init(struct read_stream_file * stream, FILE * fp, error_monitor_t em) {
    stream->m_stream.read = stream_do_read_from_file;
    stream->m_fp = fp;
    stream->m_em = em;
}

