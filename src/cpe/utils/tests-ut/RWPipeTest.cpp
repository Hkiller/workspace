#include "RWPipeTest.hpp"

void RWPipeTest::SetUp() {
    BufferTest::SetUp();
}

void RWPipeTest::TearDown() {
    BufferTest::TearDown();
}

void RWPipeTest::rwpipe_use(rwpipe_t pipe, uint32_t capacity) {
    EXPECT_GE(capacity, 4);
    EXPECT_LE(capacity, 1024);

    char buf[1024] = {0};
    ASSERT_EQ(0, rwpipe_send(pipe, buf, capacity - 4));

    uint32_t len = sizeof(buf);
    ASSERT_EQ(0, rwpipe_recv(pipe, buf, &len));
}

rwpipe_t RWPipeTest::create_pipe(uint32_t capacity) {
    void * buf = t_tmp_alloc(capacity);
    return rwpipe_init(buf, capacity);
}
