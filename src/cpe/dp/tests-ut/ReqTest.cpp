#include "ReqTest.hpp"

ReqTest::ReqTest() : m_req(NULL) {
}

dp_req_t
 ReqTest::createReq(const char * type, size_t capacity) {
    if (m_req) dp_req_free(m_req);

    m_req = t_dp_req_create(type, capacity);
    return m_req;
}

void ReqTest::TearDown() {
    if (m_req) dp_req_free(m_req);
    m_req = NULL;

    Base::TearDown();
}

