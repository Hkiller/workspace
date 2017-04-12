#include "cpepp/utils/StreamMem.hpp"

namespace Cpe { namespace Utils {

WriteStremBuf::WriteStremBuf(void * buf, size_t size) {
    write_stream_mem_init(&m_stream, buf, size);
}

}}

