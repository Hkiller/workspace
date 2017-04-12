#ifndef CPEPP_UTILS_STREAM_MEM_H
#define CPEPP_UTILS_STREAM_MEM_H
#include "cpe/pal/pal_string.h"
#include "cpe/utils/stream_mem.h"
#include "ClassCategory.hpp"

namespace Cpe { namespace Utils {

class WriteStremBuf {
public:
    WriteStremBuf(void * buf, size_t size);

    operator write_stream_t() { return (write_stream_t)&m_stream; }

private:
    struct write_stream_mem m_stream;
};

}}

#endif
