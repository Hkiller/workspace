#ifndef CPE_DR_TEST_WITH_INPUTMETALIBTEST_H
#define CPE_DR_TEST_WITH_INPUTMETALIBTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/buffer.h"

class BufferTest : public testenv::fixture<> {
public:
    virtual void SetUp();
    virtual void TearDown();

    struct mem_buffer m_buffer;

    size_t append_string(const char * data);
    size_t append_zero();

    struct mem_buffer_trunk *
    append_trunk(const char * data);

    char * as_string(void);

    size_t trunk_count(void);
    struct mem_buffer_trunk * trunk_at(size_t pos);

    size_t buffer_size(void);
};

#endif
