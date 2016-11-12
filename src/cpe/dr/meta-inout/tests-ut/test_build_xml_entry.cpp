#include "cpe/dr/dr_ctypes_info.h"
#include "cpe/dr/dr_data.h"
#include "BuildFromXmlTest.hpp"

class BuildFromXmlEntryTest : public BuildFromXmlTest {
};

TEST_F(BuildFromXmlEntryTest, entry_data) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' "
        "               desc='a1.desc'\n"
        "               cname='a1.cname'\n"
        "               type='int16'\n"
        "               id='12'/>"
        "    </struct>"
        "</metalib>"
        );

    LPDRMETAENTRY e = entry("PkgHead", "a1");

    EXPECT_EQ(12, dr_entry_id(e));
    EXPECT_STREQ("a1", dr_entry_name(e));
    EXPECT_STREQ("a1.desc", dr_entry_desc(e));
    EXPECT_STREQ("a1.cname", dr_entry_cname(e));
    EXPECT_EQ(1, dr_entry_version(e));
    EXPECT_EQ(CPE_DR_TYPE_INT16, dr_entry_type(e));
    EXPECT_EQ(NULL, dr_entry_dft_value(e));
    EXPECT_EQ(2, (int)dr_entry_size(e));
}

TEST_F(BuildFromXmlEntryTest, id_use_macro) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <macro name='macro_1' value='100'/>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' type='int16' id='macro_1'/>"
        "    </struct>"
        "</metalib>"
        );

    LPDRMETAENTRY e = entry("PkgHead", "a1");
    EXPECT_EQ(100, dr_entry_id(e));
}

TEST_F(BuildFromXmlEntryTest, no_name) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry type='int16'\n/>"
        "    </struct>"
        "</metalib>"
        );

    LPDRMETA head = meta("PkgHead");

    EXPECT_EQ(0, dr_meta_entry_num(head));
    EXPECT_TRUE(t_em_have_errno(CPE_DR_ERROR_META_NO_NAME));
}

TEST_F(BuildFromXmlEntryTest, no_type) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1'\n/>"
        "    </struct>"
        "</metalib>"
        );

    LPDRMETA head = meta("PkgHead");

    EXPECT_EQ(0, dr_meta_entry_num(head));
    EXPECT_TRUE(t_em_have_errno(CPE_DR_ERROR_ENTRY_NO_TYPE));
}

TEST_F(BuildFromXmlEntryTest, type_composite) {
    EXPECT_EQ(
        0,
        parseMeta(
            "<metalib tagsetversion='1' name='net'  version='10'>"
            "    <struct name='A1' version='1' align='1'>"
            "	     <entry name='a1' type='int32'\n/>"
            "    </struct>"
            "    <struct name='A2' version='1' align='1'>"
            "	     <entry name='a1' type='A1'\n/>"
            "    </struct>"
            "</metalib>"
            ));

    LPDRMETA m = meta("A2");
    ASSERT_TRUE(m);

    EXPECT_EQ(1, dr_meta_entry_num(m));
}

TEST_F(BuildFromXmlEntryTest, version_new) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' version='2'\n type='int16'/>"
        "    </struct>"
        "</metalib>"
        );

    LPDRMETA head = meta("PkgHead");

    EXPECT_EQ(1, dr_meta_based_version(head));
    EXPECT_EQ(2, dr_meta_current_version(head));
}

TEST_F(BuildFromXmlEntryTest, version_bigger) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' version='11'\n type='int16'/>"
        "    </struct>"
        "</metalib>"
        );

    LPDRMETA head = meta("PkgHead");

    EXPECT_EQ(0, dr_meta_entry_num(head));
    EXPECT_TRUE(t_em_have_errno(CPE_DR_ERROR_INVALID_VERSION));
}

TEST_F(BuildFromXmlEntryTest, size_for_string) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' type='string' size='12'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ((size_t)12, dr_entry_size(entry("PkgHead", "a1")));
    EXPECT_EQ((size_t)12, dr_meta_size(meta("PkgHead")));
}

TEST_F(BuildFromXmlEntryTest, size_for_int) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' type='int16' size='12'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ((size_t)2, dr_entry_size(entry("PkgHead", "a1")));
}

TEST_F(BuildFromXmlEntryTest, size_for_complex_type) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='U1' version='1' align='1'>"
        "	     <entry name='a1' type='int16'/>"
        "	     <entry name='a1' type='int16'/>"
        "    </struct>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' type='U1' size='12'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ((size_t)4, dr_entry_size(entry("PkgHead", "a1")));
}

TEST_F(BuildFromXmlEntryTest, string_no_size) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' type='string'/>"
        "    </struct>"
        "</metalib>"
        );

    LPDRMETA head = meta("PkgHead");

    EXPECT_EQ(0, dr_meta_entry_num(head));
    EXPECT_TRUE(t_em_have_errno(CPE_DR_ERROR_ENTRY_INVALID_SIZE_VALUE));
}

TEST_F(BuildFromXmlEntryTest, string_array) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' type='string' size='13' count='12'/>"
        "    </struct>"
        "</metalib>"
        );

    ASSERT_TRUE(t_em_no_error());

    LPDRMETAENTRY e = entry("PkgHead", "a1");
    ASSERT_TRUE(e);

    EXPECT_EQ(dr_entry_array_count(e), 12);
    EXPECT_EQ(dr_entry_element_size(e), (size_t)13);
    EXPECT_EQ(dr_entry_size(e), (size_t)(12 * 13));
}

TEST_F(BuildFromXmlEntryTest, dftvalue_basic) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' defaultvalue='12' type='int16'/>"
        "    </struct>"
        "</metalib>"
        );

    ASSERT_TRUE(t_em_no_error());

    LPDRMETAENTRY e = entry("PkgHead", "a1");
    ASSERT_TRUE(e);

    const void * dftValue = dr_entry_dft_value(e);
    ASSERT_TRUE(dftValue) << "dftValue not exist";

    EXPECT_EQ(12, dr_entry_read_int32(dftValue, e));
}

TEST_F(BuildFromXmlEntryTest, string_def_value) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' type='string' size='5' defaultvalue='abc'/>"
        "    </struct>"
        "</metalib>"
        );

    LPDRMETAENTRY e = entry("PkgHead", "a1");
    ASSERT_TRUE(e);

    const void * dftValue = dr_entry_dft_value(e);
    ASSERT_TRUE(dftValue) << "dftValue not exist";

    EXPECT_STREQ("abc", (const char *)dftValue);
}

TEST_F(BuildFromXmlEntryTest, string_def_value_overflow) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='PkgHead' version='1' align='1'>"
        "	     <entry name='a1' type='string' size='5' defaultvalue='abcdef'/>"
        "    </struct>"
        "</metalib>"
        );

    LPDRMETAENTRY e = entry("PkgHead", "a1");
    ASSERT_TRUE(e);

    const void * dftValue = dr_entry_dft_value(e);
    ASSERT_TRUE(dftValue) << "dftValue not exist";

    EXPECT_STREQ("abcd", (const char *)dftValue);
}

TEST_F(BuildFromXmlEntryTest, sstruct_def_value_basic) {
    parseMeta(
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='color' version='1'>"
        "	     <entry name='a1' type='float' size='1'/>"
        "	     <entry name='a2' type='float' size='1' defaultvalue='2'/>"
        "    </struct>"
        "    <struct name='PkgHead' version='1'>"
        "	     <entry name='a1' type='color' defaultvalue='{\"a1\": 2}'/>"
        "    </struct>"
        "</metalib>"
        );

    struct color {
        float a1;
        float a2;
    };
    
    LPDRMETAENTRY e = entry("PkgHead", "a1");
    ASSERT_TRUE(e);

    const color * dftValue = (const color *)dr_entry_dft_value(e);
    ASSERT_TRUE(dftValue) << "dftValue not exist";

    EXPECT_FLOAT_EQ(2.0f, dftValue->a1);
    EXPECT_FLOAT_EQ(0.0f, dftValue->a2);
    
    //EXPECT_STREQ("abcd", (const char *)dftValue);
}
