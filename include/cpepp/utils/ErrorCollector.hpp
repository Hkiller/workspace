#ifndef CPEPP_UTILS_ERRORCOLLECTOR_H
#define CPEPP_UTILS_ERRORCOLLECTOR_H
#include <sstream>
#include "cpe/utils/error.h"
#include "cpe/utils/error_list.h"
#include "ClassCategory.hpp"

namespace Cpe { namespace Utils {

class ErrorCollector : public Noncopyable {
public:
    ErrorCollector(mem_allocrator_t alloc = NULL);
    ~ErrorCollector();

    bool success(void) { return cpe_error_list_error_count(m_el) == 0; }
    bool error(void) { return cpe_error_list_error_count(m_el) != 0; }

    template<typename T>
    void checkThrowWithMsg(void) {
        if (error()) {
            throw T(genErrorMsg().c_str());
        }
    }

    template<typename T>
    void checkThrowWithMsg(const char * prefix) {
        if (error()) {
            ::std::ostringstream os;
            os << prefix;
            genErrorMsg(os);
            throw T(os.str().c_str());
        }
    }

    ::std::string genErrorMsg(void) const;
    void genErrorMsg(::std::ostream & os) const;

    operator error_monitor_t (void) { return &m_em; }

private:
    struct error_monitor m_em;
    error_list_t m_el;
};

}}

#endif
