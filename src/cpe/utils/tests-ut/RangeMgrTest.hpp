#ifndef CPE_DR_TEST_RANGEALLOCRATOR_H
#define CPE_DR_TEST_RANGEALLOCRATOR_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/range.h"
#include "cpe/utils/buffer.h"

class RangeMgrTest : public testenv::fixture<> {
public:
    virtual void SetUp();
    virtual void TearDown();

    struct cpe_range_mgr m_ra;
    struct mem_buffer m_dump_buffer;

    const char * dump(void);
};

#endif
