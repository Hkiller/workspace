#include "cpe/dr/dr_metalib_manage.h"
#include "WriteTest.hpp"

TEST_F(WriteTest, type_uin32) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='hello' type='uint32'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(0x10, write("S", "hello: 123"));

    EXPECT_STREQ(
        "0x10 0x00 0x00 0x00 0x10 0x68 0x65 0x6C 0x6C 0x6F 0x00"
        " 0x7B 0x00 0x00 0x00 0x00"
        , result());
}

TEST_F(WriteTest, type_in32) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='hello' type='int32'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(0x10, write("S", "hello: -123"));

    EXPECT_STREQ(
        "0x10 0x00 0x00 0x00 0x10 0x68 0x65 0x6C 0x6C 0x6F 0x00"
        " 0x85 0xFF 0xFF 0xFF 0x00"
        , result());
}

TEST_F(WriteTest, type_uin64) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='hello' type='uint64'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(0x14, write("S", "hello: 123"));

    EXPECT_STREQ(
        "0x14 0x00 0x00 0x00 0x12 0x68 0x65 0x6C 0x6C 0x6F 0x00"
        " 0x7B 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00"
        , result());
}

TEST_F(WriteTest, type_in64) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='hello' type='int64'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(0x14, write("S", "hello: -123"));

    EXPECT_STREQ(
        "0x14 0x00 0x00 0x00 0x12 0x68 0x65 0x6C 0x6C 0x6F 0x00"
        " 0x85 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0x00"
        , result());
}

TEST_F(WriteTest, type_float) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='hello' type='float'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(0x14, write("S", "hello: 123.0"));

    EXPECT_STREQ(
        "0x14 0x00 0x00 0x00 0x01 0x68 0x65 0x6C 0x6C 0x6F 0x00"
        " 0x00 0x00 0x00 0x00 0x00 0xC0 0x5E 0x40 0x00"
        , result());
}

TEST_F(WriteTest, type_double) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='hello' type='double'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(0x14, write("S", "hello: 123.0"));

    EXPECT_STREQ(
        "0x14 0x00 0x00 0x00 0x01 0x68 0x65 0x6C 0x6C 0x6F 0x00"
        " 0x00 0x00 0x00 0x00 0x00 0xC0 0x5E 0x40 0x00"
        , result());
}

TEST_F(WriteTest, type_string) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='hello' type='string' id='1' size='30'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(0x16, write("S", "hello: world"));

    EXPECT_STREQ(
        "0x16 0x00 0x00 0x00 0x02 0x68 0x65 0x6C 0x6C 0x6F 0x00"
        " 0x06 0x00 0x00 0x00 0x77 0x6F 0x72 0x6C 0x64 0x00 0x00"
        , result());
}

TEST_F(WriteTest, type_array_uint32_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='a1' type='uint32' count='3'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(35, write("S", "a1: [1, 2, 3]"));

    EXPECT_STREQ(
        "0x23 0x00 0x00 0x00"
        " 0x04"
        " 0x61 0x31 0x00"
        " 0x1A 0x00 0x00 0x00"
        " 0x10 0x30 0x00 0x01 0x00 0x00 0x00" /* '1': 1*/
        " 0x10 0x31 0x00 0x02 0x00 0x00 0x00" /* '2': 2*/
        " 0x10 0x32 0x00 0x03 0x00 0x00 0x00" /* '3': 3*/
        " 0x00"
        " 0x00"
        , result());
}

TEST_F(WriteTest, type_array_float_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='a1' type='float' count='3'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(47, write("S", "a1: [1.1, 2.2, 3.3]"));

    EXPECT_STREQ(
        "0x2F 0x00 0x00 0x00"
        " 0x04"
        " 0x61 0x31 0x00"
        " 0x26 0x00 0x00 0x00"
        " 0x01 0x30 0x00 0x00 0x00 0x00 0xA0 0x99 0x99 0xF1 0x3F" /* '1': 1.1*/
        " 0x01 0x31 0x00 0x00 0x00 0x00 0xA0 0x99 0x99 0x01 0x40" /* '2': 2.2*/
        " 0x01 0x32 0x00 0x00 0x00 0x00 0x60 0x66 0x66 0x0A 0x40" /* '3': 3.3*/
        " 0x00"
        " 0x00"
        , result());
}

TEST_F(WriteTest, type_array_refer) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='count' type='uint32'/>"
        "	     <entry name='a1' type='uint32' count='0' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(46, write("S", "a1: [1, 2, 3]"));

    EXPECT_STREQ(
        "0x2E 0x00 0x00 0x00"
        " 0x10 0x63 0x6F 0x75 0x6E 0x74 0x00 0x03 0x00 0x00 0x00"
        " 0x04"
        " 0x61 0x31 0x00"
        " 0x1A 0x00 0x00 0x00"
        " 0x10 0x30 0x00 0x01 0x00 0x00 0x00" /* '1': 1*/
        " 0x10 0x31 0x00 0x02 0x00 0x00 0x00" /* '2': 2*/
        " 0x10 0x32 0x00 0x03 0x00 0x00 0x00" /* '3': 3*/
        " 0x00"
        " 0x00"
        , result());
}

TEST_F(WriteTest, type_array_empty) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='count' type='uint32'/>"
        "	     <entry name='a1' type='uint32' count='0' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(25, write("S", "count: 0"));

    EXPECT_STREQ(
        "0x19 0x00 0x00 0x00"
        " 0x10 0x63 0x6F 0x75 0x6E 0x74 0x00 0x00 0x00 0x00 0x00"
        " 0x04"
        " 0x61 0x31 0x00"
        " 0x05 0x00 0x00 0x00"
        " 0x00"
        " 0x00"
        , result());
}

TEST_F(WriteTest, type_array_string_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='a1' type='string' size='128' count='3'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(41, write("S", "a1: [a, b, c]"));
    
    EXPECT_STREQ(
        "0x29 0x00 0x00 0x00"
        " 0x04"
        " 0x61 0x31 0x00"
        " 0x20 0x00 0x00 0x00"
        " 0x02 0x30 0x00 0x02 0x00 0x00 0x00 0x61 0x00" /* '0': a*/
        " 0x02 0x31 0x00 0x02 0x00 0x00 0x00 0x62 0x00" /* '1': b*/
        " 0x02 0x32 0x00 0x02 0x00 0x00 0x00 0x63 0x00" /* '2': c*/
        " 0x00"
        " 0x00"
        ,
        result());
}

TEST_F(WriteTest, type_array_struct_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='b1' type='S1' id='3' count='3'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(62, write("S2", "b1: [{ a1: 1 }, { a1: 2 }, { a1: 3 }]"));
    
    EXPECT_STREQ(
        "0x3E 0x00 0x00 0x00"
        " 0x04" /*b1.type array*/
        " 0x62 0x31 0x00" /*b1.name*/
        " 0x35 0x00 0x00 0x00" /*b1.value document size*/

          " 0x03" /*0.type embered*/
          " 0x30 0x00" /*0.name*/
          " 0x0D 0x00 0x00 0x00" /*0.size*/
          " 0x10" /*0.a1.type*/
          " 0x61 0x31 0x00" /*0.a1.name*/
          " 0x01 0x00 0x00 0x00" /*0.a1.value*/
          " 0x00" /*0.end*/

          " 0x03" /*1.type embered*/
          " 0x31 0x00" /*1.name*/
          " 0x0D 0x00 0x00 0x00" /*1.size*/
          " 0x10" /*1.a1.type*/
          " 0x61 0x31 0x00" /*1.a1.name*/
          " 0x02 0x00 0x00 0x00" /*1.a1.value*/
          " 0x00" /*1.end*/
  
          " 0x03" /*2.type embered*/
          " 0x32 0x00" /*1.name*/
          " 0x0D 0x00 0x00 0x00" /*2.size*/
          " 0x10" /*2.a1.type*/
          " 0x61 0x31 0x00" /*2.a1.name*/
          " 0x03 0x00 0x00 0x00" /*2.a1.value*/
          " 0x00" /*2.end*/

        " 0x00"
        " 0x00"
        , result());
}
