#include "gtest/gtest.h"
#include "cpe/utils/error_list.h"
#include "cpe/dr/dr_ctypes_op.h"

class CtypeWriteTest : public ::testing::Test {
public:
    CtypeWriteTest() : m_errorList(NULL) {
    }

    virtual void SetUp() {
        m_errorList = cpe_error_list_create(NULL);
    }

    virtual void TearDown() {
        cpe_error_list_free(m_errorList);
        m_errorList = NULL;
    }

    error_list_t m_errorList;
};

#define TEST_CTYPE_WRITE_SUCCESS(__to, __from, __cn, __to_type, __input) \
TEST_F(CtypeWriteTest, __to ## _from_ ## __from ## _ ## __cn) {          \
    __from ## _t input = __input;                                       \
    CPE_DEF_ERROR_MONITOR(em, cpe_error_list_collect, m_errorList);     \
    __to ## _t result;                                                  \
    EXPECT_EQ(0, dr_ctype_set_from_ ## __from(&result, input, __to_type, &em)); \
    EXPECT_EQ((__to ## _t)(__input), result);                           \
    EXPECT_EQ(0, cpe_error_list_error_count(m_errorList));              \
}

#define TEST_CTYPE_WRITE_ERROR(__to, __from, __cn, __to_type, __input) \
TEST_F(CtypeWriteTest, __to ## _from_ ## __from ## _ ## __cn) {          \
    __from ## _t input = __input;                                       \
    CPE_DEF_ERROR_MONITOR(em, cpe_error_list_collect, m_errorList);     \
    __to ## _t result;                                                  \
    EXPECT_EQ(-1, dr_ctype_set_from_ ## __from(&result, input, __to_type, &em)); \
    EXPECT_EQ((__to ## _t)(__input), result);                           \
    EXPECT_EQ(1, cpe_error_list_error_count(m_errorList));              \
}

/*to int8*/
TEST_CTYPE_WRITE_SUCCESS(int8, int8, basic, CPE_DR_TYPE_INT8, 23)

TEST_CTYPE_WRITE_SUCCESS(int8, uint8, basic, CPE_DR_TYPE_INT8, 23)
TEST_CTYPE_WRITE_ERROR(int8, uint8, overflow, CPE_DR_TYPE_INT8, 128)

TEST_CTYPE_WRITE_SUCCESS(int8, int16, basic, CPE_DR_TYPE_INT8, 23)
TEST_CTYPE_WRITE_ERROR(int8, int16, overflow, CPE_DR_TYPE_INT8, 128)
TEST_CTYPE_WRITE_ERROR(int8, int16, underflow, CPE_DR_TYPE_INT8, -129)

TEST_CTYPE_WRITE_SUCCESS(int8, uint16, basic, CPE_DR_TYPE_INT8, 23)
TEST_CTYPE_WRITE_ERROR(int8, uint16, overflow, CPE_DR_TYPE_INT8, 128)

TEST_CTYPE_WRITE_SUCCESS(int8, int32, basic, CPE_DR_TYPE_INT8, 23)
TEST_CTYPE_WRITE_ERROR(int8, int32, overflow, CPE_DR_TYPE_INT8, 128)
TEST_CTYPE_WRITE_ERROR(int8, int32, underflow, CPE_DR_TYPE_INT8, -129)

TEST_CTYPE_WRITE_SUCCESS(int8, uint32, basic, CPE_DR_TYPE_INT8, 23)
TEST_CTYPE_WRITE_ERROR(int8, uint32, overflow, CPE_DR_TYPE_INT8, 128)

TEST_CTYPE_WRITE_SUCCESS(int8, int64, basic, CPE_DR_TYPE_INT8, 23)
TEST_CTYPE_WRITE_ERROR(int8, int64, overflow, CPE_DR_TYPE_INT8, 128)
TEST_CTYPE_WRITE_ERROR(int8, int64, underflow, CPE_DR_TYPE_INT8, -129)

TEST_CTYPE_WRITE_SUCCESS(int8, uint64, basic, CPE_DR_TYPE_INT8, 23)
TEST_CTYPE_WRITE_ERROR(int8, uint64, overflow, CPE_DR_TYPE_INT8, 128)

/*to uint8*/
TEST_CTYPE_WRITE_SUCCESS(uint8, int8, basic, CPE_DR_TYPE_UINT8, 23)
TEST_CTYPE_WRITE_ERROR(uint8, int8, underflow, CPE_DR_TYPE_UINT8, -1)

TEST_CTYPE_WRITE_SUCCESS(uint8, uint8, basic, CPE_DR_TYPE_UINT8, 23)

TEST_CTYPE_WRITE_SUCCESS(uint8, int16, basic, CPE_DR_TYPE_UINT8, 23)
TEST_CTYPE_WRITE_ERROR(uint8, int16, overflow, CPE_DR_TYPE_UINT8, 256)
TEST_CTYPE_WRITE_ERROR(uint8, int16, underflow, CPE_DR_TYPE_UINT8, -1)

TEST_CTYPE_WRITE_SUCCESS(uint8, uint16, basic, CPE_DR_TYPE_UINT8, 23)
TEST_CTYPE_WRITE_ERROR(uint8, uint16, overflow, CPE_DR_TYPE_UINT8, 256)

TEST_CTYPE_WRITE_SUCCESS(uint8, int32, basic, CPE_DR_TYPE_UINT8, 23)
TEST_CTYPE_WRITE_ERROR(uint8, int32, overflow, CPE_DR_TYPE_UINT8, 256)
TEST_CTYPE_WRITE_ERROR(uint8, int32, underflow, CPE_DR_TYPE_UINT8, -1)

TEST_CTYPE_WRITE_SUCCESS(uint8, uint32, basic, CPE_DR_TYPE_UINT8, 23)
TEST_CTYPE_WRITE_ERROR(uint8, uint32, overflow, CPE_DR_TYPE_UINT8, 256)

TEST_CTYPE_WRITE_SUCCESS(uint8, int64, basic, CPE_DR_TYPE_UINT8, 23)
TEST_CTYPE_WRITE_ERROR(uint8, int64, overflow, CPE_DR_TYPE_UINT8, 256)
TEST_CTYPE_WRITE_ERROR(uint8, int64, underflow, CPE_DR_TYPE_UINT8, -1)

TEST_CTYPE_WRITE_SUCCESS(uint8, uint64, basic, CPE_DR_TYPE_UINT8, 23)
TEST_CTYPE_WRITE_ERROR(uint8, uint64, overflow, CPE_DR_TYPE_UINT8, 256)

/*to int16*/
TEST_CTYPE_WRITE_SUCCESS(int16, int8, basic, CPE_DR_TYPE_INT16, 23)

TEST_CTYPE_WRITE_SUCCESS(int16, uint8, basic, CPE_DR_TYPE_INT16, 23)

TEST_CTYPE_WRITE_SUCCESS(int16, int16, basic, CPE_DR_TYPE_INT16, 23)

TEST_CTYPE_WRITE_SUCCESS(int16, uint16, basic, CPE_DR_TYPE_INT16, 23)
TEST_CTYPE_WRITE_ERROR(int16, uint16, overflow, CPE_DR_TYPE_INT16, 32768)

TEST_CTYPE_WRITE_SUCCESS(int16, int32, basic, CPE_DR_TYPE_INT16, 23)
TEST_CTYPE_WRITE_ERROR(int16, int32, overflow, CPE_DR_TYPE_INT16, 32768)
TEST_CTYPE_WRITE_ERROR(int16, int32, underflow, CPE_DR_TYPE_INT16, -32769)

TEST_CTYPE_WRITE_SUCCESS(int16, uint32, basic, CPE_DR_TYPE_INT16, 23)
TEST_CTYPE_WRITE_ERROR(int16, uint32, overflow, CPE_DR_TYPE_INT16, 32768)

TEST_CTYPE_WRITE_SUCCESS(int16, int64, basic, CPE_DR_TYPE_INT16, 23)
TEST_CTYPE_WRITE_ERROR(int16, int64, overflow, CPE_DR_TYPE_INT16, 32768)
TEST_CTYPE_WRITE_ERROR(int16, int64, underflow, CPE_DR_TYPE_INT16, -32769)

TEST_CTYPE_WRITE_SUCCESS(int16, uint64, basic, CPE_DR_TYPE_INT16, 23)
TEST_CTYPE_WRITE_ERROR(int16, uint64, overflow, CPE_DR_TYPE_INT16, 32768)

/*to uint16*/
TEST_CTYPE_WRITE_SUCCESS(uint16, int8, basic, CPE_DR_TYPE_UINT16, 23)
TEST_CTYPE_WRITE_ERROR(uint16, int8, underflow, CPE_DR_TYPE_UINT16, -1)

TEST_CTYPE_WRITE_SUCCESS(uint16, uint8, basic, CPE_DR_TYPE_UINT16, 23)

TEST_CTYPE_WRITE_SUCCESS(uint16, int16, basic, CPE_DR_TYPE_UINT16, 23)
TEST_CTYPE_WRITE_ERROR(uint16, int16, underflow, CPE_DR_TYPE_UINT16, -1)

TEST_CTYPE_WRITE_SUCCESS(uint16, uint16, basic, CPE_DR_TYPE_UINT16, 23)

TEST_CTYPE_WRITE_SUCCESS(uint16, int32, basic, CPE_DR_TYPE_UINT16, 23)
TEST_CTYPE_WRITE_ERROR(uint16, int32, overflow, CPE_DR_TYPE_UINT16, 65536)
TEST_CTYPE_WRITE_ERROR(uint16, int32, underflow, CPE_DR_TYPE_UINT16, -1)

TEST_CTYPE_WRITE_SUCCESS(uint16, uint32, basic, CPE_DR_TYPE_UINT16, 23)
TEST_CTYPE_WRITE_ERROR(uint16, uint32, overflow, CPE_DR_TYPE_UINT16, 65536)

TEST_CTYPE_WRITE_SUCCESS(uint16, int64, basic, CPE_DR_TYPE_UINT16, 23)
TEST_CTYPE_WRITE_ERROR(uint16, int64, overflow, CPE_DR_TYPE_UINT16, 65536)
TEST_CTYPE_WRITE_ERROR(uint16, int64, underflow, CPE_DR_TYPE_UINT16, -1)

TEST_CTYPE_WRITE_SUCCESS(uint16, uint64, basic, CPE_DR_TYPE_UINT16, 23)
TEST_CTYPE_WRITE_ERROR(uint16, uint64, overflow, CPE_DR_TYPE_UINT16, 65536)

/*to int32*/
TEST_CTYPE_WRITE_SUCCESS(int32, int8, basic, CPE_DR_TYPE_INT32, 23)

TEST_CTYPE_WRITE_SUCCESS(int32, uint8, basic, CPE_DR_TYPE_INT32, 23)

TEST_CTYPE_WRITE_SUCCESS(int32, int16, basic, CPE_DR_TYPE_INT32, 23)

TEST_CTYPE_WRITE_SUCCESS(int32, uint16, basic, CPE_DR_TYPE_INT32, 23)

TEST_CTYPE_WRITE_SUCCESS(int32, int32, basic, CPE_DR_TYPE_INT32, 23)

TEST_CTYPE_WRITE_SUCCESS(int32, uint32, basic, CPE_DR_TYPE_INT32, 23)
TEST_CTYPE_WRITE_ERROR(int32, uint32, overflow, CPE_DR_TYPE_INT32, 2147483648U)

TEST_CTYPE_WRITE_SUCCESS(int32, int64, basic, CPE_DR_TYPE_INT32, 23)
TEST_CTYPE_WRITE_ERROR(int32, int64, overflow, CPE_DR_TYPE_INT32, 2147483648U)
TEST_CTYPE_WRITE_ERROR(int32, int64, underflow, CPE_DR_TYPE_INT32, -2147483649LL)

TEST_CTYPE_WRITE_SUCCESS(int32, uint64, basic, CPE_DR_TYPE_INT32, 23)
TEST_CTYPE_WRITE_ERROR(int32, uint64, overflow, CPE_DR_TYPE_INT32, 2147483648LL)

/*to uint32*/
TEST_CTYPE_WRITE_SUCCESS(uint32, int8, basic, CPE_DR_TYPE_UINT32, 23)
TEST_CTYPE_WRITE_ERROR(uint32, int8, underflow, CPE_DR_TYPE_UINT32, -1)

TEST_CTYPE_WRITE_SUCCESS(uint32, uint8, basic, CPE_DR_TYPE_UINT32, 23)

TEST_CTYPE_WRITE_SUCCESS(uint32, int16, basic, CPE_DR_TYPE_UINT32, 23)
TEST_CTYPE_WRITE_ERROR(uint32, int16, underflow, CPE_DR_TYPE_UINT32, -1)

TEST_CTYPE_WRITE_SUCCESS(uint32, uint16, basic, CPE_DR_TYPE_UINT32, 23)

TEST_CTYPE_WRITE_SUCCESS(uint32, int32, basic, CPE_DR_TYPE_UINT32, 23)
TEST_CTYPE_WRITE_ERROR(uint32, int32, underflow, CPE_DR_TYPE_UINT32, -1)

TEST_CTYPE_WRITE_SUCCESS(uint32, uint32, basic, CPE_DR_TYPE_UINT32, 23)

TEST_CTYPE_WRITE_SUCCESS(uint32, int64, basic, CPE_DR_TYPE_UINT32, 23)
TEST_CTYPE_WRITE_ERROR(uint32, int64, underflow, CPE_DR_TYPE_UINT32, -1)
TEST_CTYPE_WRITE_ERROR(uint32, int64, overflow, CPE_DR_TYPE_UINT32, 4294967296LL)

TEST_CTYPE_WRITE_SUCCESS(uint32, uint64, basic, CPE_DR_TYPE_UINT32, 23)
TEST_CTYPE_WRITE_ERROR(uint32, uint64, overflow, CPE_DR_TYPE_UINT32, 4294967296LL)

/*to int64*/
TEST_CTYPE_WRITE_SUCCESS(int64, int8, basic, CPE_DR_TYPE_INT64, 23)

TEST_CTYPE_WRITE_SUCCESS(int64, uint8, basic, CPE_DR_TYPE_INT64, 23)

TEST_CTYPE_WRITE_SUCCESS(int64, int16, basic, CPE_DR_TYPE_INT64, 23)

TEST_CTYPE_WRITE_SUCCESS(int64, uint16, basic, CPE_DR_TYPE_INT64, 23)

TEST_CTYPE_WRITE_SUCCESS(int64, int32, basic, CPE_DR_TYPE_INT64, 23)

TEST_CTYPE_WRITE_SUCCESS(int64, uint32, basic, CPE_DR_TYPE_INT64, 23)

TEST_CTYPE_WRITE_SUCCESS(int64, int64, basic, CPE_DR_TYPE_INT64, 23)

TEST_CTYPE_WRITE_SUCCESS(int64, uint64, basic, CPE_DR_TYPE_INT64, 23)
TEST_CTYPE_WRITE_ERROR(int64, uint64, overflow, CPE_DR_TYPE_INT64, 9223372036854775808LLU)

/*to uint64*/
TEST_CTYPE_WRITE_SUCCESS(uint64, int8, basic, CPE_DR_TYPE_UINT64, 23)
TEST_CTYPE_WRITE_ERROR(uint64, int8, underflow, CPE_DR_TYPE_UINT64, -1)

TEST_CTYPE_WRITE_SUCCESS(uint64, uint8, basic, CPE_DR_TYPE_UINT64, 23)

TEST_CTYPE_WRITE_SUCCESS(uint64, int16, basic, CPE_DR_TYPE_UINT64, 23)
TEST_CTYPE_WRITE_ERROR(uint64, int16, underflow, CPE_DR_TYPE_UINT64, -1)

TEST_CTYPE_WRITE_SUCCESS(uint64, uint16, basic, CPE_DR_TYPE_UINT64, 23)

TEST_CTYPE_WRITE_SUCCESS(uint64, int32, basic, CPE_DR_TYPE_UINT64, 23)
TEST_CTYPE_WRITE_ERROR(uint64, int32, underflow, CPE_DR_TYPE_UINT64, -1)

TEST_CTYPE_WRITE_SUCCESS(uint64, uint32, basic, CPE_DR_TYPE_UINT64, 23)

TEST_CTYPE_WRITE_SUCCESS(uint64, int64, basic, CPE_DR_TYPE_UINT64, 23)
TEST_CTYPE_WRITE_ERROR(uint64, int64, underflow, CPE_DR_TYPE_UINT64, -1)

TEST_CTYPE_WRITE_SUCCESS(uint64, uint64, basic, CPE_DR_TYPE_UINT64, 23)
