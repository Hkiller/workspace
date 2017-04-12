#include "WriteTest.hpp"

TEST_F(WriteTest, type_struct_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='b1' type='S1' id='3'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(22, write("S2", "b1: { a1: 1 }"));
    
    EXPECT_STREQ(
        "0x16 0x00 0x00 0x00" /*S2.len*/
        " 0x03" /*b1.type*/
        " 0x62 0x31 0x00" /*b1.name*/
        " 0x0D 0x00 0x00 0x00" /*S1.len*/
        " 0x10" /*a1.type*/
        " 0x61 0x31 0x00" /*a1.name*/
        " 0x01 0x00 0x00 0x00" /*a1.value*/
        " 0x00" /*S1 end*/
        " 0x00"
        , result());
}

TEST_F(WriteTest, type_struct_basic_open) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='b1' type='S1' id='3'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(17, write("S2", "b1: { a1: 1 }", 1));
    
    EXPECT_STREQ(
        "0x03" /*b1.type*/
        " 0x62 0x31 0x00" /*b1.name*/
        " 0x0D 0x00 0x00 0x00" /*S1.len*/
        " 0x10" /*a1.type*/
        " 0x61 0x31 0x00" /*a1.name*/
        " 0x01 0x00 0x00 0x00" /*a1.value*/
        " 0x00" /*S1 end*/
        , result());
}

TEST_F(WriteTest, type_struct_array) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S1' version='1'>"
        "	     <entry name='a1' type='uint32' count='2'/>"
        "    </struct>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='b1' type='S1'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(0x25, write("S2", "b1: { a1: [1, 2] }"));
    
    EXPECT_STREQ(
        "0x25 0x00 0x00 0x00" /*S2.len*/
        " 0x03" /*b1.type*/
        " 0x62 0x31 0x00" /*b1.name*/
        " 0x1C 0x00 0x00 0x00" /*S1.len*/
        " 0x04"/*a1.type*/
        " 0x61 0x31 0x00" /*a1.name*/
        " 0x13 0x00 0x00 0x00" /*a1.document.len*/
        " 0x10" /*a1[0].type*/
        " 0x30 0x00" /*a1[0].name 0*/
        " 0x01 0x00 0x00 0x00" /*a1[0].value*/
        " 0x10" /*a1[1].type*/
        " 0x31 0x00" /*a1[1].name 1*/
        " 0x02 0x00 0x00 0x00" /*a1[1].value*/
        " 0x00" /*a1.document.end*/
        " 0x00" /*S1 end*/
        " 0x00"
        , result());
}

TEST_F(WriteTest, type_union_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <union name='U1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "	     <entry name='a2' type='uint32' id='2'/>"
        "    </union>"
        "    <struct name='S2' version='1'>"
        "        <entry name='s' type='uint32'/>"
        "	     <entry name='b1' type='U1' id='3' select='s'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(29, write("S2", "b1: { a1: 1 }"));
    
    EXPECT_STREQ(
        "0x1D 0x00 0x00 0x00" /*S2.len*/
        " 0x10" /*s.type*/
        " 0x73 0x00" /*s.name*/
        " 0x01 0x00 0x00 0x00" /*s.value*/
        " 0x03" /*b1.type*/
        " 0x62 0x31 0x00" /*b1.name*/
        " 0x0D 0x00 0x00 0x00" /*S1.len*/
        " 0x10" /*a1.type*/
        " 0x61 0x31 0x00" /*a1.name*/
        " 0x01 0x00 0x00 0x00" /*a1.value*/
        " 0x00" /*S1 end*/
        " 0x00"
        , result());
}

TEST_F(WriteTest, type_union_no_select) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <union name='U1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "	     <entry name='a2' type='uint32' id='2'/>"
        "    </union>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='b1' type='U1'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(30, write("S2", "b1: { a1: 1 }"));
    
    EXPECT_STREQ(
        "0x1E 0x00 0x00 0x00" /*S2.len*/
        " 0x03" /*b1.type*/
        " 0x62 0x31 0x00" /*b1.name*/
        " 0x15 0x00 0x00 0x00" /*U1.len*/
        " 0x10" /*a1.type*/
        " 0x61 0x31 0x00" /*a1.name*/
        " 0x01 0x00 0x00 0x00" /*a1.value*/
        " 0x10" /*a2.type*/
        " 0x61 0x32 0x00" /*a2.name*/
        " 0x01 0x00 0x00 0x00" /*a2.value*/
        " 0x00" /*U1 end*/
        " 0x00"
        , result());
}

TEST_F(WriteTest, type_union_select_no_data) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <union name='U1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "	     <entry name='a2' type='uint32' id='2'/>"
        "    </union>"
        "    <struct name='S2' version='1'>"
        "        <entry name='s' type='uint32'/>"
        "	     <entry name='b1' type='U1' id='3' select='s'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(21, write("S2", "s: 3"));
    
    EXPECT_STREQ(
        "0x15 0x00 0x00 0x00" /*S2.len*/
        " 0x10" /*s.type*/
        " 0x73 0x00" /*s.name*/
        " 0x03 0x00 0x00 0x00" /*s.value*/
        " 0x03" /*b1.type*/
        " 0x62 0x31 0x00" /*b1.name*/
        " 0x05 0x00 0x00 0x00" /*S1.len*/
        " 0x00" /*S1 end*/
        " 0x00"
        , result());
}
