#include "cpe/dr/dr_ctypes_op.h"
#include "SetDefaultsTest.hpp"

TEST_F(SetDefaultsTest, multi_entry_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net' version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16' defaultvalue='23'/>"
        "	     <entry name='a2' type='int16' defaultvalue='45'/>"
        "    </struct>"
        "</metalib>");

    set_defaults("S");

    EXPECT_EQ(23, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
    EXPECT_EQ(45, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
}

TEST_F(SetDefaultsTest, numeric_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net' version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16' defaultvalue='23'/>"
        "    </struct>"
        "</metalib>");

    set_defaults("S");

    EXPECT_EQ(23, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
}

TEST_F(SetDefaultsTest, numeric_no_default) {
    installMeta(
        "<metalib tagsetversion='1' name='net' version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16'/>"
        "    </struct>"
        "</metalib>");

    set_defaults("S");

    EXPECT_EQ(0, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
}

TEST_F(SetDefaultsTest, numeric_no_default_ignore) {
    installMeta(
        "<metalib tagsetversion='1' name='net' version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16'/>"
        "    </struct>"
        "</metalib>");

    set_defaults("S", DR_SET_DEFAULTS_POLICY_NO_DEFAULT_IGNORE);

    EXPECT_NE(0, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
}

TEST_F(SetDefaultsTest, string_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net' version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='string' size='6' defaultvalue='23'/>"
        "    </struct>"
        "</metalib>");

    set_defaults("S");

    EXPECT_STREQ("23", (const char *)result());
}

TEST_F(SetDefaultsTest, string_no_default) {
    installMeta(
        "<metalib tagsetversion='1' name='net' version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='string' size='6'/>"
        "    </struct>"
        "</metalib>");

    set_defaults("S");

    EXPECT_STREQ("", (const char *)result());
}

TEST_F(SetDefaultsTest, sequence_value_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net' version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16' count='4' defaultvalue='23'/>"
        "    </struct>"
        "</metalib>");

    set_defaults("S");

    EXPECT_EQ(23, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
    EXPECT_EQ(23, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
    EXPECT_EQ(23, dr_ctype_read_int16(result(4), CPE_DR_TYPE_INT16));
    EXPECT_EQ(23, dr_ctype_read_int16(result(6), CPE_DR_TYPE_INT16));
}

TEST_F(SetDefaultsTest, sequence_struct_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net' version='1'>"
        "    <struct name='S1' version='1' align='1'>"
        "	     <entry name='b1' type='int16' defaultvalue='23'/>"
        "    </struct>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='S1' count='4'/>"
        "    </struct>"
        "</metalib>");

    set_defaults("S");

    for(int i = 0; i < 4; ++i) {
        EXPECT_EQ(23, dr_ctype_read_int16(result(i * 2), CPE_DR_TYPE_INT16));
    }
}

TEST_F(SetDefaultsTest, sequence_struct_sequence) {
    installMeta(
        "<metalib tagsetversion='1' name='net' version='1'>"
        "    <struct name='S1' version='1' align='1'>"
        "	     <entry name='b1' type='int16' count='5' defaultvalue='23'/>"
        "    </struct>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='S1' count='4'/>"
        "    </struct>"
        "</metalib>");

    set_defaults("S");

    for(int i = 0; i < 20; ++i) {
        EXPECT_EQ(23, dr_ctype_read_int16(result(i * 2), CPE_DR_TYPE_INT16));
    }
}

TEST_F(SetDefaultsTest, union_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='A' version='1' align='1'>"
        "	     <entry name='a1' type='int32' defaultvalue='9'/>"
        "	     <entry name='a2' type='int16' defaultvalue='10'/>"
        "    </struct>"
        "    <struct name='B' version='1' align='1'>"
        "	     <entry name='b1' type='int16' defaultvalue='11'/>"
        "	     <entry name='b2' type='int16' defaultvalue='12'/>"
        "    </struct>"
        "    <union name='U' version='1'>"
        "	     <entry name='u1' type='A' id='2'/>"
        "	     <entry name='u2' type='B' id='3'/>"
        "    </union>"
        "</metalib>");

    set_defaults("U");

    EXPECT_EQ(11, dr_ctype_read_int16(result(0), CPE_DR_TYPE_INT16));
    EXPECT_EQ(12, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
}

TEST_F(SetDefaultsTest, union_with_selector) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='A' version='1' align='1'>"
        "	     <entry name='a1' type='int32' defaultvalue='9'/>"
        "	     <entry name='a2' type='int16' defaultvalue='10'/>"
        "    </struct>"
        "    <struct name='B' version='1' align='1'>"
        "	     <entry name='b1' type='int16' defaultvalue='11'/>"
        "	     <entry name='b2' type='int16' defaultvalue='12'/>"
        "    </struct>"
        "    <union name='U' version='1'>"
        "	     <entry name='u1' type='A' id='2'/>"
        "	     <entry name='u2' type='B' id='3'/>"
        "    </union>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='type' type='int16' defaultvalue='2'/>"
        "        <entry name='data' type='U' select='type'/>"
        "    </struct>"
        "</metalib>");

    set_defaults("S");

    EXPECT_EQ(2, dr_ctype_read_int16(result(0), CPE_DR_TYPE_INT16));
    EXPECT_EQ(9, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT32));
    EXPECT_EQ(10, dr_ctype_read_int16(result(6), CPE_DR_TYPE_INT16));
}

TEST_F(SetDefaultsTest, struct_block_default_baskc) {
    installMeta(
        "<metalib tagsetversion='1' name='net' version='1'>"
        "    <struct name='S1' version='1' align='1'>"
        "	     <entry name='a1' type='int16' count='4' defaultvalue='23'/>"
        "    </struct>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='o' type='S1' defaultvalue='{ a1: [23, 24, 25, 26] }'/>"
        "    </struct>"
        "</metalib>");

    set_defaults("S");

    EXPECT_EQ(23, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
    EXPECT_EQ(24, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
    EXPECT_EQ(25, dr_ctype_read_int16(result(4), CPE_DR_TYPE_INT16));
    EXPECT_EQ(26, dr_ctype_read_int16(result(6), CPE_DR_TYPE_INT16));
}
