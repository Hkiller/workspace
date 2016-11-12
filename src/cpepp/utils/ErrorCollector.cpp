#include <limits>
#include <sstream>
#include "cpepp/utils/ErrorCollector.hpp"

namespace Cpe { namespace Utils {

ErrorCollector::ErrorCollector(mem_allocrator_t alloc)
    : m_el(NULL)
{
    m_el = cpe_error_list_create(alloc);
    if (m_el == NULL) {
        throw ::std::bad_alloc();
    }

    cpe_error_monitor_init(&m_em, cpe_error_list_collect, m_el);
}

ErrorCollector::~ErrorCollector() {
    cpe_error_list_free(m_el);
}

static void collect_msg(void * ctx, struct error_info * info, const char * msg) {
    ::std::ostream * os = reinterpret_cast< ::std::ostream *>(ctx);

    (*os) << msg << ::std::endl;
}

::std::string
ErrorCollector::genErrorMsg(void) const {
    ::std::ostringstream os;
    genErrorMsg(os);
    return os.str();
}

void ErrorCollector::genErrorMsg(::std::ostream & os) const {
    cpe_error_list_visit(m_el, collect_msg, &os);
}

}}

