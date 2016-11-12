#include "cpe/dr/dr_ctypes_info.h"
#include "cpe/dr/dr_data.h"
#include "BuildFromXmlTest.hpp"

class BuildFromXmlEntryTest : public BuildFromXmlTest {
};

TEST_F(BuildFromXmlEntryTest, array_count_basic) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' type='int16' count='16'/>"
        "    </struct>"
        "</metalib>"
        );

    ASSERT_TRUE(t_em_no_error());

    LPDRMETAENTRY e = entry("PkgHead", "a1");
    EXPECT_EQ(16, e->m_array_count);
    EXPECT_EQ(2 * 16, e->m_unitsize);
}

TEST_F(BuildFromXmlEntryTest, array_count_default) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' type='int16'/>"
        "    </struct>"
        "</metalib>"
        );

    LPDRMETAENTRY e = entry("PkgHead", "a1");
    EXPECT_EQ(1, e->m_array_count);
}

TEST_F(BuildFromXmlEntryTest, array_count_zero) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' type='int16' count='0'/>"
        "    </struct>"
        "</metalib>"
        );

    LPDRMETAENTRY e = entry("PkgHead", "a1");
    EXPECT_EQ(0, e->m_array_count);
    EXPECT_EQ(2 * 1, e->m_unitsize);
}

TEST_F(BuildFromXmlEntryTest, array_count_negative) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' type='int16' count='-1'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_TRUE(t_em_have_errno(CPE_DR_ERROR_ENTRY_INVALID_COUNT_VALUE));
}

TEST_F(BuildFromXmlEntryTest, array_count_format_error) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' type='int16' count='xyz'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_TRUE(t_em_have_errno(CPE_DR_ERROR_UNDEFINED_MACRO_NAME));
}

TEST_F(BuildFromXmlEntryTest, array_refer_basic) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='fill' type='int16'/>"
        "	     <entry name='count' type='int16'/>"
        "	     <entry name='a1' type='int16' count='0' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    ASSERT_TRUE(t_em_no_error());

    LPDRMETAENTRY e = entry("PkgHead", "a1");
    ASSERT_TRUE(e);

    LPDRMETAENTRY referEntry = dr_entry_array_refer_entry(e);
    ASSERT_TRUE(referEntry);

    ASSERT_STREQ("count", dr_entry_name(referEntry));
    ASSERT_EQ(2, e->m_array_refer_data_start_pos);
}

TEST_F(BuildFromXmlEntryTest, array_refer_no_refer) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='count' type='int16'/>"
        "	     <entry name='a1' type='int16' count='0'/>"
        "    </struct>"
        "</metalib>"
        );

    LPDRMETAENTRY e = entry("PkgHead", "a1");
    ASSERT_TRUE(e);

    LPDRMETAENTRY referEntry = dr_entry_array_refer_entry(e);
    ASSERT_TRUE(referEntry == NULL);
}

TEST_F(BuildFromXmlEntryTest, array_refer_not_for_array) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='count' type='int16'/>"
        "	     <entry name='a1' type='int16' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_TRUE(t_em_have_errno(CPE_DR_ERROR_ENTRY_INVALID_REFER_VALUE));
}

TEST_F(BuildFromXmlEntryTest, array_refer_dyn_array_no_refer) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='count' type='int16'/>"
        "	     <entry name='a1' type='int16' count='0'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_TRUE(t_em_have_errno(CPE_DR_ERROR_VARIABLE_ARRAY_NO_REFER));
}

TEST_F(BuildFromXmlEntryTest, array_refer_not_exist) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' type='int16' count='0' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_TRUE(t_em_have_errno(CPE_DR_ERROR_ENTRY_INVALID_REFER_VALUE));
}
