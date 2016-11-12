#include "gtest/gtest.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "../../dr_ctype_ops.h"

class CtypeOpsFromNumericTest : public ::testing::Test {
public:
    int8_t as_int8(void) { return *((int8_t*)m_buf); }
    uint8_t as_uint8(void) { return *((uint8_t*)m_buf); }

    int16_t as_int16(void) { return *((int16_t*)m_buf); }
    uint16_t as_uint16(void) { return *((uint16_t*)m_buf); }

    int32_t as_int32(void) { return *((int32_t*)m_buf); }
    uint32_t as_uint32(void) { return *((uint32_t*)m_buf); }

    char m_buf[128];
};

TEST_F(CtypeOpsFromNumericTest, int8_from_int32_basic) {
    EXPECT_EQ(0, dr_ctype_set_from_int32(&m_buf, 12,  CPE_DR_TYPE_INT8, 0));
    EXPECT_EQ(12, (int)as_int8());
}
