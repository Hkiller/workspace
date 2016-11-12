#include <limits>
#include <stdexcept>
#include "cpe/utils/file.h"
#include "cpepp/utils/StreamFile.hpp"
#include "cpe/pal/pal_stdio.h"

namespace Cpe { namespace Utils {

WriteStremFile::WriteStremFile(const char * filename, error_monitor_t em, const char * mode) 
    : m_auto_close(1)
{
    FILE * file = file_stream_open(filename, mode, em);
    if (file == NULL) {
        char buf[256];
        snprintf(buf, sizeof(buf), "WriteStremFile: open file(%s) fail!", filename);
        throw ::std::runtime_error(buf);
    }

    write_stream_file_init(&m_stream, file, em);
}

WriteStremFile::WriteStremFile(FILE * file, error_monitor_t em) 
    : m_auto_close(0)
{
    write_stream_file_init(&m_stream, file, em);
}

WriteStremFile::~WriteStremFile() {
    if (m_auto_close) {
        file_stream_close(m_stream.m_fp, m_stream.m_em);
    }
}

ReadStremFile::ReadStremFile(const char * filename, error_monitor_t em) 
    : m_auto_close(1)
{
    FILE * file = file_stream_open(filename, "r", em);
    if (file == NULL) {
        throw ::std::runtime_error("ReadStremFile: open file fail!");
    }

    read_stream_file_init(&m_stream, file, em);
}

ReadStremFile::ReadStremFile(FILE * file, error_monitor_t em) 
    : m_auto_close(0)
{
    read_stream_file_init(&m_stream, file, em);
}

ReadStremFile::~ReadStremFile() {
    if (m_auto_close) {
        file_stream_close(m_stream.m_fp, m_stream.m_em);
    }
}

}}

