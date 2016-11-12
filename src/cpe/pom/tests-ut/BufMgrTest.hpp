#ifndef CPE_OM_TEST_BUFMGR_H
#define CPE_OM_TEST_BUFMGR_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/pom/pom_error.h"
#include "../pom_internal_ops.h"

typedef LOKI_TYPELIST_1(
    utils::testenv::with_em) BufMgrTestBase;

class BufMgrTest : public testenv::fixture<BufMgrTestBase> {
public:
    BufMgrTest();

    virtual void SetUp();
    virtual void TearDown();

    int init(size_t page_size, size_t buf_size);

    void * page_get(void);

    bool m_inited;
    struct pom_buffer_mgr m_bufMgr;
};

#endif
