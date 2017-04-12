#include "cpe/dp/tests-env/with_dp.hpp"

namespace cpe { namespace dp { namespace testenv {

with_dp::with_dp()
    : m_dp(NULL)
{
}

void with_dp::SetUp() {
    m_dp = dp_mgr_create(t_allocrator());
}


void with_dp::TearDown() {
    dp_mgr_free(m_dp);
    m_dp = NULL;
}

dp_mgr_t with_dp::t_dp() {
    return m_dp;
}

dp_req_t
with_dp::t_dp_req_create(const char * type, size_t capacity) {
    dp_req_t req = dp_req_create(t_dp(), capacity);
    if (req == NULL) return NULL;

    if (type) dp_req_set_type(req, t_tmp_strdup(type)); 

    return req;
}

dp_req_t
with_dp::t_dp_req_create_child(dp_req_t req, const char * type, void * buf, size_t capacity) {
    dp_req_t r = t_dp_req_create(type, capacity);
    dp_req_add_to_parent(r, req);
    dp_req_set_buf(r, buf, capacity);
    return r;
}

}}}

