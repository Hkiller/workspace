#include "StreamTest.hpp"

TEST_F(StreamTest, printf_char) {
    EXPECT_EQ(1, stream_printf((write_stream_t)&m_stream, "%c", 'a'));

    append_zero();
    EXPECT_STREQ("a", as_string());
}

TEST_F(StreamTest, int) {
    EXPECT_EQ(3, stream_printf((write_stream_t)&m_stream, "%d", 123));

    append_zero();
    EXPECT_STREQ("123", as_string());
}

TEST_F(StreamTest, printf_float) {
    EXPECT_EQ(10, stream_printf((write_stream_t)&m_stream, "%f", 123.456));

    append_zero();
    EXPECT_STREQ("123.456000", as_string());
}
