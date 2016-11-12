#include "RWPipeTest.hpp"

TEST_F(RWPipeTest, basic) {
    rwpipe_t pipe = create_pipe(128);

    ASSERT_TRUE(pipe);

    EXPECT_EQ((uint32_t)128, rwpipe_total_capacity(pipe));
    EXPECT_EQ((uint32_t)112, rwpipe_data_capacity(pipe));

    EXPECT_EQ((uint32_t)0, rwpipe_data_size(pipe));
    EXPECT_EQ((uint32_t)111, rwpipe_data_left_size(pipe));
}

TEST_F(RWPipeTest, send_max) {
    rwpipe_t pipe = create_pipe(128);

    ASSERT_TRUE(pipe);

    char buf[128] = {0};

    EXPECT_EQ(0, rwpipe_send(pipe, buf, 107));

    EXPECT_EQ((uint32_t)111, rwpipe_data_size(pipe));
    EXPECT_EQ((uint32_t)0, rwpipe_data_left_size(pipe));
}

TEST_F(RWPipeTest, send_overflow) {
    rwpipe_t pipe = create_pipe(128);

    ASSERT_TRUE(pipe);

    char buf[128] = {0};

    EXPECT_EQ(RWPIPE_ERR_SEND_DATA_OVERFLOW, rwpipe_send(pipe, buf, 108));

    EXPECT_EQ((uint32_t)0, rwpipe_data_size(pipe));
    EXPECT_EQ((uint32_t)111, rwpipe_data_left_size(pipe));
}

TEST_F(RWPipeTest, recv_max) {
    rwpipe_t pipe = create_pipe(128);

    ASSERT_TRUE(pipe);

    char buf1[128] = {'a'};
    EXPECT_EQ(0, rwpipe_send(pipe, buf1, 107));
    EXPECT_EQ((uint32_t)111, rwpipe_data_size(pipe));
    EXPECT_EQ((uint32_t)0, rwpipe_data_left_size(pipe));

    char buf2[128] = {0};
    uint32_t len = sizeof(buf2);
    EXPECT_EQ(0, rwpipe_recv(pipe, buf2, &len));

    EXPECT_EQ((uint32_t)107, len);
    EXPECT_EQ(0, memcmp(buf1, buf2, len));

    EXPECT_EQ((uint32_t)0, rwpipe_data_size(pipe));
    EXPECT_EQ((uint32_t)111, rwpipe_data_left_size(pipe));
}

TEST_F(RWPipeTest, pos_left_not_enough_datay) {
    rwpipe_t pipe = create_pipe(128);

    ASSERT_TRUE(pipe);

    rwpipe_use(pipe, 106);
    EXPECT_EQ((uint32_t)0, rwpipe_data_size(pipe));
    EXPECT_EQ((uint32_t)111, rwpipe_data_left_size(pipe));

    char buf1[128] = {'a'};
    EXPECT_EQ(0, rwpipe_send(pipe, buf1, 12));
    EXPECT_EQ((uint32_t)16, rwpipe_data_size(pipe));
    EXPECT_EQ((uint32_t)95, rwpipe_data_left_size(pipe));

    char buf2[128] = {0};
    uint32_t len = sizeof(buf2);
    EXPECT_EQ(0, rwpipe_recv(pipe, buf2, &len));

    EXPECT_EQ((uint32_t)12, len);
    EXPECT_EQ(0, memcmp(buf1, buf2, len));

    EXPECT_EQ((uint32_t)0, rwpipe_data_size(pipe));
    EXPECT_EQ((uint32_t)111, rwpipe_data_left_size(pipe));
}

TEST_F(RWPipeTest, pos_left_not_enough_size) {
    rwpipe_t pipe = create_pipe(128);

    ASSERT_TRUE(pipe);

    rwpipe_use(pipe, 110);

    char buf1[128] = {'a'};
    EXPECT_EQ(0, rwpipe_send(pipe, buf1, 12));
    EXPECT_EQ((uint32_t)16, rwpipe_data_size(pipe));
    EXPECT_EQ((uint32_t)95, rwpipe_data_left_size(pipe));

    char buf2[128] = {0};
    uint32_t len = sizeof(buf2);
    EXPECT_EQ(0, rwpipe_recv(pipe, buf2, &len));

    EXPECT_EQ((uint32_t)12, len);
    EXPECT_EQ(0, memcmp(buf1, buf2, len));

    EXPECT_EQ((uint32_t)0, rwpipe_data_size(pipe));
    EXPECT_EQ((uint32_t)111, rwpipe_data_left_size(pipe));
}

TEST_F(RWPipeTest, pos_write_middle) {
    rwpipe_t pipe = create_pipe(128);

    ASSERT_TRUE(pipe);

    rwpipe_use(pipe, 110);

    char buf1[128] = {'a'};
    EXPECT_EQ(0, rwpipe_send(pipe, buf1, 12));

    EXPECT_EQ(0, rwpipe_send(pipe, buf1, 13));
    EXPECT_EQ((uint32_t)33, rwpipe_data_size(pipe));
    EXPECT_EQ((uint32_t)78, rwpipe_data_left_size(pipe));

    char buf2[128] = {0};

    uint32_t len = sizeof(buf2);
    EXPECT_EQ(0, rwpipe_recv(pipe, buf2, &len));
    EXPECT_EQ((uint32_t)12, len);
    EXPECT_EQ(0, memcmp(buf1, buf2, len));
    EXPECT_EQ((uint32_t)17, rwpipe_data_size(pipe));
    EXPECT_EQ((uint32_t)94, rwpipe_data_left_size(pipe));

    len = sizeof(buf2);
    EXPECT_EQ(0, rwpipe_recv(pipe, buf2, &len));
    EXPECT_EQ((uint32_t)13, len);
    EXPECT_EQ(0, memcmp(buf1, buf2, len));
    EXPECT_EQ((uint32_t)0, rwpipe_data_size(pipe));
    EXPECT_EQ((uint32_t)111, rwpipe_data_left_size(pipe));
}

TEST_F(RWPipeTest, pos_write_middle_max) {
    rwpipe_t pipe = create_pipe(128);

    ASSERT_TRUE(pipe);

    rwpipe_use(pipe, 110);

    char buf1[128] = {'a'};
    EXPECT_EQ(0, rwpipe_send(pipe, buf1, 12));

    EXPECT_EQ(0, rwpipe_send(pipe, buf1, 91));
    EXPECT_EQ((uint32_t)111, rwpipe_data_size(pipe));
    EXPECT_EQ((uint32_t)0, rwpipe_data_left_size(pipe));
}

TEST_F(RWPipeTest, pos_write_middle_overflow) {
    rwpipe_t pipe = create_pipe(128);

    ASSERT_TRUE(pipe);

    rwpipe_use(pipe, 110);

    char buf1[128] = {'a'};
    EXPECT_EQ(0, rwpipe_send(pipe, buf1, 12));

    EXPECT_EQ(RWPIPE_ERR_SEND_DATA_OVERFLOW, rwpipe_send(pipe, buf1, 92));
    EXPECT_EQ((uint32_t)16, rwpipe_data_size(pipe));
    EXPECT_EQ((uint32_t)95, rwpipe_data_left_size(pipe));
}
