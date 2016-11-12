#ifndef CPEPP_UTILS_STREAM_FILE_H
#define CPEPP_UTILS_STREAM_FILE_H
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_file.h"
#include "ClassCategory.hpp"

namespace Cpe { namespace Utils {

class WriteStremFile {
public:
    WriteStremFile(const char * filename, error_monitor_t em = NULL, const char *mode = "w+");
    WriteStremFile(FILE * fp, error_monitor_t em = NULL);
    ~WriteStremFile();
        
    operator write_stream_t() { return (write_stream_t)&m_stream; }

    void set_auto_close(uint8_t auto_close) { m_auto_close = auto_close; }
private:
    struct write_stream_file m_stream;
    uint8_t m_auto_close;
};

class ReadStremFile {
public:
    ReadStremFile(const char * filename, error_monitor_t em = NULL);
    ReadStremFile(FILE * fp, error_monitor_t em = NULL);
    ~ReadStremFile();
        
    operator read_stream_t() { return (read_stream_t)&m_stream; }

    void set_auto_close(uint8_t auto_close) { m_auto_close = auto_close; }
private:
    struct read_stream_file m_stream;
    uint8_t m_auto_close;
};

}}

#endif
