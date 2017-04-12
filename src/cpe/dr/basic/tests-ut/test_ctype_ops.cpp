#include "gtest/gtest.h"
#include "../../dr_ctype_ops.h"

struct CtypeNameIdInfo {
    int m_id;
    const char * m_name;
};

class CtypeOpsFindByNameTest : public ::testing::TestWithParam<CtypeNameIdInfo> {
};

TEST_P(CtypeOpsFindByNameTest, FindByName) {
    CtypeNameIdInfo caseInfo = GetParam();

    const struct tagDRCTypeInfo * ctypeInfo = dr_find_ctype_info_by_name(caseInfo.m_name);
    ASSERT_TRUE(ctypeInfo) << "get ctype by name " << caseInfo.m_name << " fail!";

    EXPECT_EQ(caseInfo.m_id, ctypeInfo->m_id);
}

class CtypeOpsFindByIdTest : public ::testing::TestWithParam<CtypeNameIdInfo> {
};

TEST_P(CtypeOpsFindByIdTest, FindById) {
    CtypeNameIdInfo caseInfo = GetParam();

    const struct tagDRCTypeInfo * ctypeInfo = dr_find_ctype_info_by_type(caseInfo.m_id);
    ASSERT_TRUE(ctypeInfo) << "get ctype by id " << caseInfo.m_id << " fail!";

    EXPECT_STREQ(caseInfo.m_name, ctypeInfo->m_name);
}

CtypeNameIdInfo ctypeCasees[] = {
    {CPE_DR_TYPE_UNION, "union"}
    , {CPE_DR_TYPE_STRUCT, "struct"}
    , {CPE_DR_TYPE_CHAR, "char"}
    , {CPE_DR_TYPE_UCHAR, "uchar"}
    , {CPE_DR_TYPE_INT8, "int8"}
    , {CPE_DR_TYPE_INT16, "int16"}
    , {CPE_DR_TYPE_UINT16, "uint16"}
    , {CPE_DR_TYPE_INT32, "int32"}
    , {CPE_DR_TYPE_UINT32, "uint32"}
    , {CPE_DR_TYPE_INT64, "int64"}
    , {CPE_DR_TYPE_UINT64, "uint64"}
    , {CPE_DR_TYPE_DATE, "date"}
    , {CPE_DR_TYPE_TIME, "time"}
    , {CPE_DR_TYPE_DATETIME, "datetime"}
    , {CPE_DR_TYPE_MONEY, "money"}
    , {CPE_DR_TYPE_FLOAT, "float"}
    , {CPE_DR_TYPE_DOUBLE, "double"}
    , {CPE_DR_TYPE_IP, "ip"}
    , {CPE_DR_TYPE_STRING, "string"}
    , {CPE_DR_TYPE_VOID, "void"}
};

INSTANTIATE_TEST_CASE_P(
    CheckName,
    CtypeOpsFindByNameTest,
    testing::ValuesIn(ctypeCasees));

INSTANTIATE_TEST_CASE_P(
    CheckId,
    CtypeOpsFindByIdTest,
    testing::ValuesIn(ctypeCasees));
