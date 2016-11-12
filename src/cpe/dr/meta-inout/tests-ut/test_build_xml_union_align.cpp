#include "cpe/dr/dr_ctypes_info.h"
#include "../../dr_internal_types.h"
#include "BuildFromXmlTest.hpp"

class BuildFromXmlUnionAlign : public BuildFromXmlTest {
};

TEST_F(BuildFromXmlUnionAlign, align1_basic) {
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

    EXPECT_EQ(1, dr_meta_align(meta("S1")));
    EXPECT_EQ((size_t)2, dr_meta_size(meta("S1")));

    EXPECT_EQ(0, entry("S1", "b1")->m_data_start_pos);
    EXPECT_EQ(0, entry("S1", "b2")->m_data_start_pos);

    EXPECT_EQ(1, dr_meta_align(meta("S2")));
    EXPECT_EQ((size_t)3, dr_meta_size(meta("S2")));

    EXPECT_EQ(0, entry("S2", "a1")->m_data_start_pos);
    EXPECT_EQ(1, entry("S2", "m_u")->m_data_start_pos);
}

TEST_F(BuildFromXmlUnionAlign, align1_basic_align) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <union name='S1' version='1' id='33' align='4'>"
        "	     <entry name='b1' type='int8'/>"
        "	     <entry name='b2' type='int16'/>"
        "    </union>"
        "    <struct name='S2' version='1' id='34' align='4'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='m_u' type='S1'/>"
        "    </struct>"
        "</metalib>"
        );

    ASSERT_TRUE(t_em_no_error());

    EXPECT_EQ(4, dr_meta_align(meta("S1")));
    EXPECT_EQ((size_t)2, dr_meta_size(meta("S1")));

    EXPECT_EQ(4, dr_meta_align(meta("S2")));
    EXPECT_EQ((size_t)4, dr_meta_size(meta("S2")));

    EXPECT_EQ(0, entry("S2", "a1")->m_data_start_pos);
    EXPECT_EQ(2, entry("S2", "m_u")->m_data_start_pos);
}

TEST_F(BuildFromXmlUnionAlign, align1_struct_align) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='V1' version='1' align='4'>"
        "	     <entry name='v' type='float'/>"
        "    </struct>"
        "    <struct name='V2' version='1' align='4'>"
        "	     <entry name='v1' type='int8'/>"
        "	     <entry name='v2' type='float'/>"
        "    </struct>"
        "    <union name='S1' version='1' align='4'>"
        "	     <entry name='b1' type='V1'/>"
        "	     <entry name='b2' type='V2'/>"
        "    </union>"
        "    <struct name='S2' version='1' align='4'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='m_u' type='S1'/>"
        "    </struct>"
        "</metalib>"
        );

    ASSERT_TRUE(t_em_no_error());

    EXPECT_EQ((size_t)4, dr_meta_size(meta("V1")));
    EXPECT_EQ((size_t)8, dr_meta_size(meta("V2")));
    
    EXPECT_EQ(4, dr_meta_align(meta("S1")));
    EXPECT_EQ((size_t)8, dr_meta_size(meta("S1")));

    EXPECT_EQ(4, dr_meta_align(meta("S2")));
    EXPECT_EQ((size_t)12, dr_meta_size(meta("S2")));

    EXPECT_EQ(0, entry("S2", "a1")->m_data_start_pos);
    EXPECT_EQ(4, entry("S2", "m_u")->m_data_start_pos);
}
