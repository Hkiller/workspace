#include <limits.h>
#include "gtest/gtest.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "../../dr_ctype_ops.h"

class CtypeOpsFromStringTest : public ::testing::Test {
public:
    int parse(const char * name, const char * data) {
        const struct tagDRCTypeInfo * typeInfo =
            dr_find_ctype_info_by_name(name);

        EXPECT_TRUE(typeInfo);
        if (typeInfo == NULL) {
            return -1;
        }

        return dr_ctype_set_from_string(m_buf, typeInfo->m_id, data, NULL);
    }

    int8_t as_int8(void) { return *((int8_t*)m_buf); }
    uint8_t as_uint8(void) { return *((uint8_t*)m_buf); }

    int16_t as_int16(void) { return *((int16_t*)m_buf); }
    uint16_t as_uint16(void) { return *((uint16_t*)m_buf); }

    int32_t as_int32(void) { return *((int32_t*)m_buf); }
    uint32_t as_uint32(void) { return *((uint32_t*)m_buf); }

    int64_t as_int64(void) { return *((int64_t*)m_buf); }
    uint64_t as_uint64(void) { return *((uint64_t*)m_buf); }

    char m_buf[128];
};

TEST_F(CtypeOpsFromStringTest, char_basic) {
    EXPECT_EQ(0, parse("char", "12"));
    EXPECT_EQ(12, m_buf[0]);
}

TEST_F(CtypeOpsFromStringTest, char_too_many) {
    EXPECT_EQ(-1, parse("char", "ab"));
}

TEST_F(CtypeOpsFromStringTest, char_empty) {
    EXPECT_EQ(-1, parse("char", ""));
}

TEST_F(CtypeOpsFromStringTest, uchar_basic) {
    EXPECT_EQ(0, parse("uchar", "12"));
    EXPECT_EQ(12, m_buf[0]);
}

TEST_F(CtypeOpsFromStringTest, uchar_too_many) {
    EXPECT_EQ(-1, parse("uchar", "ab"));
}

TEST_F(CtypeOpsFromStringTest, uchar_empty) {
    EXPECT_EQ(-1, parse("uchar", ""));
}

TEST_F(CtypeOpsFromStringTest, int8_min) {
    EXPECT_EQ(0, parse("int8", "-128"));
    EXPECT_EQ(-128, as_int8());
}

TEST_F(CtypeOpsFromStringTest, int8_middle) {
    EXPECT_EQ(0, parse("int8", "123"));
    EXPECT_EQ(123, as_int8());
}

TEST_F(CtypeOpsFromStringTest, int8_max) {
    EXPECT_EQ(0, parse("int8", "127"));
    EXPECT_EQ(127, as_int8());
}

TEST_F(CtypeOpsFromStringTest, int8_up_overflow) {
    EXPECT_EQ(-1, parse("int8", "128"));
}

TEST_F(CtypeOpsFromStringTest, int8_down_overflow) {
    EXPECT_EQ(-1, parse("int8", "-129"));
}

TEST_F(CtypeOpsFromStringTest, int8_end_with_char) {
    EXPECT_EQ(-1, parse("int8", "15a"));
}

TEST_F(CtypeOpsFromStringTest, uint8_min) {
    EXPECT_EQ(0, parse("uint8", "0"));
    EXPECT_EQ(0, as_uint8());
}

TEST_F(CtypeOpsFromStringTest, uint8_max) {
    EXPECT_EQ(0, parse("uint8", "255"));
    EXPECT_EQ(255, as_uint8());
}

TEST_F(CtypeOpsFromStringTest, uint8_up_overflow) {
    EXPECT_EQ(-1, parse("uint8", "256"));
}

TEST_F(CtypeOpsFromStringTest, uint8_down_overflow) {
    EXPECT_EQ(-1, parse("uint8", "-1"));
}

TEST_F(CtypeOpsFromStringTest, uint8_end_with_char) {
    EXPECT_EQ(-1, parse("uint8", "15a"));
}

TEST_F(CtypeOpsFromStringTest, int16_min) {
    EXPECT_EQ(0, parse("int16", "-32768"));
    EXPECT_EQ(-32768, as_int16());
}

TEST_F(CtypeOpsFromStringTest, int16_max) {
    EXPECT_EQ(0, parse("int16", "32767"));
    EXPECT_EQ(32767, as_int16());
}

TEST_F(CtypeOpsFromStringTest, int16_up_overflow) {
    EXPECT_EQ(-1, parse("int16", "32768"));
}

TEST_F(CtypeOpsFromStringTest, int16_down_overflow) {
    EXPECT_EQ(-1, parse("int16", "-32769"));
}

TEST_F(CtypeOpsFromStringTest, int16_end_with_char) {
    EXPECT_EQ(-1, parse("int16", "15a"));
}

TEST_F(CtypeOpsFromStringTest, uint16_min) {
    EXPECT_EQ(0, parse("uint16", "0"));
    EXPECT_EQ(0, as_uint16());
}

TEST_F(CtypeOpsFromStringTest, uint16_max) {
    EXPECT_EQ(0, parse("uint16", "65535"));
    EXPECT_EQ(65535, as_uint16());
}

TEST_F(CtypeOpsFromStringTest, uint16_up_overflow) {
    EXPECT_EQ(-1, parse("uint16", "65536"));
}

TEST_F(CtypeOpsFromStringTest, uint16_down_overflow) {
    EXPECT_EQ(-1, parse("uint16", "-1"));
}

TEST_F(CtypeOpsFromStringTest, uint16_end_with_char) {
    EXPECT_EQ(-1, parse("uint16", "15a"));
}

TEST_F(CtypeOpsFromStringTest, int32_min) {
    EXPECT_EQ(0, parse("int32", "-2147483648"));
    EXPECT_EQ(INT_MIN, as_int32());
}

TEST_F(CtypeOpsFromStringTest, int32_max) {
    EXPECT_EQ(0, parse("int32", "2147483647"));
    EXPECT_EQ(2147483647, as_int32());
}

TEST_F(CtypeOpsFromStringTest, int32_up_overflow) {
    EXPECT_EQ(-1, parse("int32", "2147483648"));
}

TEST_F(CtypeOpsFromStringTest, int32_down_overflow) {
    EXPECT_EQ(-1, parse("int32", "-2147483649"));
}

TEST_F(CtypeOpsFromStringTest, int32_end_with_char) {
    EXPECT_EQ(-1, parse("int32", "15a"));
}

TEST_F(CtypeOpsFromStringTest, uint32_min) {
    EXPECT_EQ(0, parse("uint32", "0"));
    EXPECT_EQ(0U, as_uint32());
}

TEST_F(CtypeOpsFromStringTest, uint32_max) {
    EXPECT_EQ(0, parse("uint32", "4294967295"));
    EXPECT_EQ(0xFFFFFFFF, as_uint32());
}

TEST_F(CtypeOpsFromStringTest, uint32_up_overflow) {
    EXPECT_EQ(-1, parse("uint32", "4294967296"));
}

TEST_F(CtypeOpsFromStringTest, uint32_down_overflow) {
    EXPECT_EQ(-1, parse("uint32", "-1"));
}

TEST_F(CtypeOpsFromStringTest, uint32_end_with_char) {
    EXPECT_EQ(-1, parse("uint32", "15a"));
}

TEST_F(CtypeOpsFromStringTest, uint64_min) {
    EXPECT_EQ(0, parse("uint64", "0"));
    EXPECT_EQ(0U, as_uint64());
}

TEST_F(CtypeOpsFromStringTest, uint64_max) {
    EXPECT_EQ(0, parse("uint64", "0xFFFFFFFFFFFFFFFF"));
    EXPECT_EQ((uint64_t)0xFFFFFFFFFFFFFFFFull, as_uint64());
}


TEST_F(CtypeOpsFromStringTest, string_basic) {
    EXPECT_EQ(-1, parse("string", "a"));
}
