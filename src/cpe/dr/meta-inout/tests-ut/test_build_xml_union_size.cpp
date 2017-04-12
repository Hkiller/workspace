#include "cpe/dr/dr_ctypes_info.h"
#include "../../dr_internal_types.h"
#include "BuildFromXmlTest.hpp"

class BuildFromXmlUnionSize : public BuildFromXmlTest {
};

TEST_F(BuildFromXmlUnionSize, size_basic) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <union name='S1' version='1' id='33' align='1'>"
        "	     <entry name='b1' type='int8'/>"
        "	     <entry name='b2' type='int16'/>"
        "    </union>"
        "    <struct name='S2' version='1' id='34' align='1'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='m_u' type='S1'/>"
        "    </struct>"
        "</metalib>"
        );

    ASSERT_TRUE(t_em_no_error());

    EXPECT_EQ((size_t)3, dr_meta_size(meta("S2")));
}

TEST_F(BuildFromXmlUnionSize, size_composiate_basic) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='U1' version='1' align='1'>"
        "	     <entry name='d' type='int8'/>"
        "    </struct>"
        "    <struct name='U2' version='1' align='1'>"
        "	     <entry name='d' type='int16'/>"
        "    </struct>"
        "    <union name='S1' version='1' align='1'>"
        "	     <entry name='b1' type='U1'/>"
        "	     <entry name='b2' type='U2'/>"
        "    </union>"
        "    <struct name='S2' version='1' id='34' align='1'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='m_u' type='S1'/>"
        "    </struct>"
        "</metalib>"
        );

    ASSERT_TRUE(t_em_no_error());

    EXPECT_EQ((size_t)3, dr_meta_size(meta("S2")));
}

TEST_F(BuildFromXmlUnionSize, size_composiate_order) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='U1' version='1' align='1'>"
        "	     <entry name='d' type='int8'/>"
        "    </struct>"
        "    <struct name='U2' version='1' align='1'>"
        "	     <entry name='d' type='int16'/>"
        "    </struct>"
        "    <union name='S1' version='1' align='1'>"
        "	     <entry name='b2' type='U2'/>"
        "	     <entry name='b1' type='U1'/>"
        "    </union>"
        "    <struct name='S2' version='1' id='34' align='1'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='m_u' type='S1'/>"
        "    </struct>"
        "</metalib>"
        );

    ASSERT_TRUE(t_em_no_error());

    EXPECT_EQ((size_t)3, dr_meta_size(meta("S2")));
}
