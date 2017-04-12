#include "gtest/gtest.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "../../dr_ctype_ops.h"

class CtypeOpsPrintfTest : public ::testing::Test {
public:
    int printf(const char * name, const void * data) {
        const struct tagDRCTypeInfo * typeInfo =
            dr_find_ctype_info_by_name(name);

        struct write_stream_mem S = CPE_WRITE_STREAM_MEM_INITIALIZER(m_buf, 128);
        bzero(m_buf, 128);
        return dr_ctype_print_to_stream((write_stream_t)&S, data, typeInfo->m_id, NULL);
    }

    char m_buf[128];
};

TEST_F(CtypeOpsPrintfTest, char_basic) {
    char data = 21;
    EXPECT_EQ(2, printf("char", &data));
    EXPECT_STREQ("21", m_buf);
}

TEST_F(CtypeOpsPrintfTest, uchar_basic) {
    unsigned char data = 23;
    EXPECT_EQ(2, printf("uchar", &data));
    EXPECT_STREQ("23", m_buf);
}

TEST_F(CtypeOpsPrintfTest, int8_basic) {
    int8_t data = (int8_t)-129;
    EXPECT_EQ(3, printf("int8", &data));
    EXPECT_STREQ("127", m_buf);
}

TEST_F(CtypeOpsPrintfTest, int8_overflow) {
    int8_t data = (int8_t)129;
    EXPECT_EQ(4, printf("int8", &data));
    EXPECT_STREQ("-127", m_buf);
}

TEST_F(CtypeOpsPrintfTest, uint8_basic) {
    uint8_t data = (uint8_t)-1;
    EXPECT_EQ(3, printf("uint8", &data));
    EXPECT_STREQ("255", m_buf);
}

TEST_F(CtypeOpsPrintfTest, int16_basic) {
    int16_t data = (int16_t)-32768;
    EXPECT_EQ(6, printf("int16", &data));
    EXPECT_STREQ("-32768", m_buf);
}

TEST_F(CtypeOpsPrintfTest, int16_overflow) {
    int16_t data = (int16_t)-32769;
    EXPECT_EQ(5, printf("int16", &data));
    EXPECT_STREQ("32767", m_buf);
}

TEST_F(CtypeOpsPrintfTest, uint16_basic) {
    uint16_t data = (uint16_t)-1;
    EXPECT_EQ(5, printf("uint16", &data));
    EXPECT_STREQ("65535", m_buf);
}

TEST_F(CtypeOpsPrintfTest, int32_basic) {
    int32_t data = (int32_t)-2147483647;
    EXPECT_EQ(11, printf("int32", &data));
    EXPECT_STREQ("-2147483647", m_buf);
}

TEST_F(CtypeOpsPrintfTest, int32_overflow) {
    int32_t data = (int32_t)-2147483647 - 2;
    EXPECT_EQ(10, printf("int32", &data));
    EXPECT_STREQ("2147483647", m_buf);
}

TEST_F(CtypeOpsPrintfTest, uint32_basic) {
    uint32_t data = (uint32_t)-1;
    EXPECT_EQ(10, printf("uint32", &data));
    EXPECT_STREQ("4294967295", m_buf);
}

TEST_F(CtypeOpsPrintfTest, int64_basic) {
    int64_t data = -9223372036854775807LL;
    EXPECT_EQ(20, printf("int64", &data));
    EXPECT_STREQ("-9223372036854775807", m_buf);
}

TEST_F(CtypeOpsPrintfTest, int64_overflow) {
    int64_t data = -9223372036854775807LL - 2;
    EXPECT_EQ(19, printf("int64", &data));
    EXPECT_STREQ("9223372036854775807", m_buf);
}

TEST_F(CtypeOpsPrintfTest, uint64_basic) {
    uint64_t data = (uint64_t)-1;
    EXPECT_EQ(20, printf("uint64", &data));
    EXPECT_STREQ("18446744073709551615", m_buf);
}
