#include "cpe/dr/dr_ctypes_info.h"
#include "BuildFromXmlTest.hpp"

struct TypeSizeCase {
    const char * m_name;
    size_t m_size;
};

class BuildFromXmlTypeSizeTest
    : public BuildFromXmlTest
    , public ::testing::WithParamInterface<TypeSizeCase>
{
};

TEST_P(BuildFromXmlTypeSizeTest, CheckSize) {
    TypeSizeCase caseData = GetParam();

    char buf[256];
    snprintf(
        buf, 256
        ,
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='S1' version='1' id='33' align='1'>"
        "	     <entry name='a1' type='%s'/>"
        "    </struct>"
        "</metalib>"
        ,
        caseData.m_name);

    parseMeta(buf);

    EXPECT_EQ(caseData.m_size, dr_meta_size(meta("S1")))
        << "size of type " << caseData.m_name << " error!";
}

static TypeSizeCase g_align_basic_case[] = {
    { "char", 1 }
    , { "float", 4 }
    , { "double", 8 }
    , { "date", 4 }
    , { "time", 4 }
    , { "datetime", 8 }
    , { "ip", 4 }
    , { "int8", 1 }
    , { "uint8", 1 }
    , { "int16", 2 }
    , { "uint16", 2 }
    , { "int32", 4 }
    , { "uint32", 4 }
    , { "int64", 8 }
    , { "uint64", 8 }
};

INSTANTIATE_TEST_CASE_P(,
    BuildFromXmlTypeSizeTest,
    testing::ValuesIn(g_align_basic_case));
