#ifndef CPE_DR_TEST_WITH_RWPIPE_H
#define CPE_DR_TEST_WITH_RWPIPE_H
#include "cpe/utils/rwpipe.h"
#include "BufferTest.hpp"

class RWPipeTest : public BufferTest {
public:
    virtual void SetUp();
    virtual void TearDown();

    void rwpipe_use(rwpipe_t pipe, uint32_t capacity);
    rwpipe_t create_pipe(uint32_t capacity);
};

#endif
