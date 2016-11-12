#ifndef SVR_SET_SVR_TESTSENV_WITH_STUB_H
#define SVR_SET_SVR_TESTSENV_WITH_STUB_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../set_svr_stub.h"

namespace svr { namespace set { namespace stub { namespace testenv {

class with_stub : public ::testenv::env<> {
public:
    void SetUp();
    void TearDown();

    void t_install_set_svr_types(const char * svr_types);

    uint16_t t_set_svr_type_id_of(const char * svr_type);

    void t_set_stub_create(const char * svr_type, const char * name = "set_svr_stub");

    set_svr_stub_t t_set_stub(void) { return m_stub; }

private:
    set_svr_stub_t m_stub;
};

}}}}

#endif

