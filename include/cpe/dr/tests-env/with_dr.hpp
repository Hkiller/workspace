#ifndef CPE_DR_TESTENV_WITH_DR_H
#define CPE_DR_TESTENV_WITH_DR_H
#include "cpe/utils/error.h"
#include "cpe/utils/error_list.h"
#include "cpe/utils/tests-env/test-env.hpp"
#include "../dr_types.h"

namespace cpe { namespace dr { namespace testenv {

class with_dr : public ::testenv::env<> {
public:
    LPDRMETALIB t_create_metalib(const char * xml);
    const char * t_dump_metalib_xml(LPDRMETALIB metalib);
};

}}}

#endif
