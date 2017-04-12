#ifndef CPE_DR_TEST_WITH_STREAMTEST_H
#define CPE_DR_TEST_WITH_STREAMTEST_H
#include "cpe/utils/stream_buffer.h"
#include "BufferTest.hpp"

class StreamTest : public BufferTest {
public:
    virtual void SetUp();
    virtual void TearDown();

    struct write_stream_buffer m_stream;
};

#endif
