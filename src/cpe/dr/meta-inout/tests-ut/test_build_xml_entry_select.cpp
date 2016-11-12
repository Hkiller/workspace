#include "cpe/dr/dr_ctypes_info.h"
#include "BuildFromXmlTest.hpp"

class BuildFromXmlEntrySelectTest : public BuildFromXmlTest {
};


TEST_F(BuildFromXmlEntrySelectTest, union_id_default) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <union name='PkgHead' version='1'>"
        "	     <entry name='a1' type='int16'/>"
        "    </union>"
        "</metalib>"
        );

    LPDRMETAENTRY e = entry("PkgHead", "a1");
    EXPECT_EQ(1, e->m_select_range_min);
    EXPECT_EQ(0, e->m_select_range_max);
}

TEST_F(BuildFromXmlEntrySelectTest, union_id_only_id) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <union name='PkgHead' version='1'>"
        "	     <entry name='a1' type='int16' id='34'/>"
        "    </union>"
        "</metalib>"
        );

    LPDRMETAENTRY e = entry("PkgHead", "a1");
    EXPECT_EQ(34, e->m_select_range_min);
    EXPECT_EQ(34, e->m_select_range_max);
}

TEST_F(BuildFromXmlEntrySelectTest, union_id_min_max) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <union name='PkgHead' version='1'>"
        "	     <entry name='a1' type='int16' minid='34' maxid='45'/>"
        "    </union>"
        "</metalib>"
        );

    LPDRMETAENTRY e = entry("PkgHead", "a1");
    EXPECT_EQ(34, e->m_select_range_min);
    EXPECT_EQ(45, e->m_select_range_max);
}

TEST_F(BuildFromXmlEntrySelectTest, union_id_min_bg_max) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <union name='PkgHead' version='1'>"
        "	     <entry name='a1' type='int16' minid='45' maxid='34'/>"
        "    </union>"
        "</metalib>"
        );

    EXPECT_TRUE(t_em_have_errno(CPE_DR_ERROR_ENTRY_INVALID_ID_VALUE));
}

TEST_F(BuildFromXmlEntrySelectTest, union_id_min_no_max) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <union name='PkgHead' version='1'>"
        "	     <entry name='a1' type='int16' minid='45'/>"
        "    </union>"
        "</metalib>"
        );

    EXPECT_TRUE(t_em_have_errno(CPE_DR_ERROR_ENTRY_INVALID_ID_VALUE));
}

TEST_F(BuildFromXmlEntrySelectTest, union_id_max_no_min) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <union name='PkgHead' version='1'>"
        "	     <entry name='a1' type='int16' maxid='34'/>"
        "    </union>"
        "</metalib>"
        );

    EXPECT_TRUE(t_em_have_errno(CPE_DR_ERROR_ENTRY_INVALID_ID_VALUE));
}

TEST_F(BuildFromXmlEntrySelectTest, select_default) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <union name='U' version='1'>"
        "	     <entry name='a1' type='int16' id='34'/>"
        "    </union>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='id' type='int16'/>"
        "	     <entry name='u' type='U'/>"
        "    </struct>"
        "</metalib>"
        );

    LPDRMETAENTRY e = entry("S", "u");
    EXPECT_EQ(-1, e->m_select_data_start_pos);
    EXPECT_EQ(-1, e->m_select_entry_pos);
}

TEST_F(BuildFromXmlEntrySelectTest, select_basic) {
    t_em_set_print();

    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <union name='U' version='1'>"
        "	     <entry name='a1' type='int16' id='34'/>"
        "    </union>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='fill' type='char'/>"
        "	     <entry name='id' type='int16'/>"
        "	     <entry name='u' type='U' select='id'/>"
        "    </struct>"
        "</metalib>"
        );

    ASSERT_TRUE(t_em_no_error());

    LPDRMETAENTRY refer = entry("S", "id");
    LPDRMETAENTRY e = entry("S", "u");
    EXPECT_EQ(refer->m_data_start_pos, e->m_select_data_start_pos);
    EXPECT_EQ(address_to_pos(refer), e->m_select_entry_pos);
}

TEST_F(BuildFromXmlEntrySelectTest, select_path_not_exist) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <union name='U' version='1'>"
        "	     <entry name='a1' type='int16' id='34'/>"
        "    </union>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='fill' type='char'/>"
        "	     <entry name='id' type='int16'/>"
        "	     <entry name='u' type='U' select='not-exist'/>"
        "    </struct>"
        "</metalib>"
        );

    LPDRMETAENTRY e = entry("S", "u");
    EXPECT_EQ(-1, e->m_select_data_start_pos);
    EXPECT_EQ(-1, e->m_select_entry_pos);
    EXPECT_TRUE(t_em_have_errno(CPE_DR_ERROR_ENTRY_INVALID_SELECT_VALUE));
}

TEST_F(BuildFromXmlEntrySelectTest, select_select_after_curent) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <union name='U' version='1'>"
        "	     <entry name='a1' type='int16' id='34'/>"
        "    </union>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='fill' type='char'/>"
        "	     <entry name='u' type='U' select='id'/>"
        "	     <entry name='id' type='int16'/>"
        "    </struct>"
        "</metalib>"
        );

    LPDRMETAENTRY e = entry("S", "u");
    EXPECT_EQ(-1, e->m_select_data_start_pos);
    EXPECT_EQ(-1, e->m_select_entry_pos);
    EXPECT_TRUE(t_em_have_errno(CPE_DR_ERROR_ENTRY_INVALID_SELECT_VALUE));
}
