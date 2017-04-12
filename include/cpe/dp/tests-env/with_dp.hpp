#ifndef CPE_DP_TESTENV_WITHDP_H
#define CPE_DP_TESTENV_WITHDP_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../dp.h"

namespace cpe { namespace dp { namespace testenv {

class with_dp : public ::testenv::env<> {
public:
    with_dp();

    void SetUp();
    void TearDown();

    dp_mgr_t t_dp();
    dp_req_t t_dp_req_create(const char * type, size_t capacity);
    dp_req_t t_dp_req_create_child(dp_req_t req, const char * type, void * buf, size_t capacity);
private:
    dp_mgr_t m_dp;
};

}}}

#endif

