#include "cpe/dr/dr_ctypes_op.h"
#include "ReadTest.hpp"

TEST_F(ReadTest, struct_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>");

    EXPECT_EQ(
        4,
        read(
            "a1: 12\n"
            "a2: 14\n",
            "S"));

    EXPECT_EQ(12, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
    EXPECT_EQ(14, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
}

TEST_F(ReadTest, struct_capacity_bigger) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>");

    EXPECT_EQ(
        4,
        read(
            "a1: 12\n"
            "a2: 14\n",
            "S",
            0,
            6));

    EXPECT_EQ(12, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
    EXPECT_EQ(14, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
}

TEST_F(ReadTest, seq_count_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16' count='12'/>"
        "	     <entry name='a3' type='int16'/>"
        "    </struct>"
        "</metalib>");

    EXPECT_EQ(
        26,
        read(
            "a1: [12, 14, 16]\n"
            "a3: 34",
            "S"));

    EXPECT_EQ(12, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
    EXPECT_EQ(14, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
    EXPECT_EQ(16, dr_ctype_read_int16(result(4), CPE_DR_TYPE_INT16));
    EXPECT_EQ(0, dr_ctype_read_int16(result(6), CPE_DR_TYPE_INT16));
    EXPECT_EQ(34, dr_ctype_read_int16(result(24), CPE_DR_TYPE_INT16));
}

TEST_F(ReadTest, seq_count_with_refer) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='count' type='int16'/>"
        "	     <entry name='a1' type='int16' count='12' refer='count'/>"
        "	     <entry name='a3' type='int16'/>"
        "    </struct>"
        "</metalib>");

    t_em_set_print();

    EXPECT_EQ(
        28,
        read(
            "a1: [12, 14]\n"
            "a3: 34",
            "S"));

    EXPECT_EQ(2, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
    EXPECT_EQ(12, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
    EXPECT_EQ(14, dr_ctype_read_int16(result(4), CPE_DR_TYPE_INT16));
    EXPECT_EQ(34, dr_ctype_read_int16(result(26), CPE_DR_TYPE_INT16));
}

TEST_F(ReadTest, seq_count_dft_no_refer) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16' count='12' defaultvalue='23'/>"
        "	     <entry name='a3' type='int16'/>"
        "    </struct>"
        "</metalib>");

    EXPECT_EQ(
        26,
        read(
            "a1: [12, 14]\n"
            "a3: 34",
            "S"));

    EXPECT_EQ(12, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
    EXPECT_EQ(14, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));

    for(int i = 0; i < 10; ++i) {
        EXPECT_EQ(23, dr_ctype_read_int16(result(4 + i * 2), CPE_DR_TYPE_INT16))
            << "default at " << (i + 2) << " error!";
    }

    EXPECT_EQ(34, dr_ctype_read_int16(result(24), CPE_DR_TYPE_INT16));
}

TEST_F(ReadTest, seq_count_dynamic_with_refer) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='count' type='int16'/>"
        "	     <entry name='a1' type='int16' count='0' refer='count'/>"
        "    </struct>"
        "</metalib>");

    EXPECT_EQ(
        6,
        read(
            "a1: [12, 14]\n"
            ,
            "S"
            ,
            0
            ,
            128));

    EXPECT_EQ(2, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
    EXPECT_EQ(12, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
    EXPECT_EQ(14, dr_ctype_read_int16(result(4), CPE_DR_TYPE_INT16));
}


TEST_F(ReadTest, int8_basic) {
    t_em_set_print();

    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int8'/>"
        "	     <entry name='a2' type='int8'/>"
        "    </struct>"
        "</metalib>");

    EXPECT_EQ(
        2,
        read(
            "a1: 12\n"
            "a2: 14\n",
            "S"));

    EXPECT_EQ(12, (int)dr_ctype_read_int8(result(), CPE_DR_TYPE_INT8));
    EXPECT_EQ(14, (int)dr_ctype_read_int8(result(1), CPE_DR_TYPE_INT8));
}

TEST_F(ReadTest, uint8_basic) {
    t_em_set_print();

    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='uint8'/>"
        "	     <entry name='a2' type='uint8'/>"
        "    </struct>"
        "</metalib>");

    EXPECT_EQ(
        2,
        read(
            "a1: 0\n"
            "a2: 14\n",
            "S"));

    EXPECT_EQ(0, (int)dr_ctype_read_uint8(result(), CPE_DR_TYPE_UINT8));
    EXPECT_EQ(14, (int)dr_ctype_read_uint8(result(1), CPE_DR_TYPE_UINT8));
}

TEST_F(ReadTest, struct_not_enouth_data) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>");

    EXPECT_EQ(
        4,
        read(
            "a1: 12\n"
            "a2: 14\n"
            ,
            "S"
            ,
            0
            ,
            2));

    EXPECT_EQ(12, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
}

TEST_F(ReadTest, union_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='A' version='1' align='1'>"
        "	     <entry name='a1' type='int32'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "    <struct name='B' version='1' align='1'>"
        "	     <entry name='b1' type='int16'/>"
        "	     <entry name='b2' type='int16'/>"
        "    </struct>"
        "    <union name='U' version='1' id='33'>"
        "	     <entry name='u1' type='A' />"
        "	     <entry name='u2' type='B'/>"
        "    </union>"
        "</metalib>");

    EXPECT_EQ(
        6,
        read(
            "u1:\n"
            "  a1: 12\n"
            "  a2: 90\n"
            ,
            "U"));

    EXPECT_EQ(12, dr_ctype_read_int32(result(), CPE_DR_TYPE_INT32));
    EXPECT_EQ(90, dr_ctype_read_int16(result(4), CPE_DR_TYPE_INT16));

    EXPECT_EQ(
        6,
        read(
            "u2:\n"
            "  b1: 21\n"
            "  b2: 78\n"
            ,
            "U"));

    EXPECT_EQ(21, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
    EXPECT_EQ(78, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
}

TEST_F(ReadTest, union_multi_input_entry) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='A' version='1' align='1'>"
        "	     <entry name='a1' type='int32'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "    <struct name='B' version='1' align='1'>"
        "	     <entry name='b1' type='int16'/>"
        "	     <entry name='b2' type='int16'/>"
        "    </struct>"
        "    <union name='U' version='1' id='33'>"
        "	     <entry name='u1' type='A' />"
        "	     <entry name='u2' type='B'/>"
        "    </union>"
        "</metalib>");

    EXPECT_EQ(
        6,
        read(
            "u1:\n"
            "  a1: 56\n"
            "  a2: 89\n"
            "u2:\n"
            "  b1: 21\n"
            "  b2: 78\n"
            ,
            "U"));

    EXPECT_EQ(21, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
    EXPECT_EQ(78, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
}

TEST_F(ReadTest, union_second_element) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='A' version='1' align='1'>"
        "	     <entry name='a1' type='int32'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "    <struct name='B' version='1' align='1'>"
        "	     <entry name='b1' type='int16'/>"
        "	     <entry name='b2' type='int16'/>"
        "    </struct>"
        "    <union name='U' version='1' id='33'>"
        "	     <entry name='u1' type='A' />"
        "	     <entry name='u2' type='B'/>"
        "    </union>"
        "</metalib>");

    EXPECT_EQ(
        6,
        read(
            "u1:\n"
            "  a1: 7\n"
            "  a2: 8\n"
            "u2:\n"
            "  b1: 12\n"
            "  b2: 90\n"
            ,
            "U"));

    EXPECT_EQ(12, dr_ctype_read_int16(result(), CPE_DR_TYPE_INT16));
    EXPECT_EQ(90, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
}

TEST_F(ReadTest, union_with_select) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='A' version='1' align='1'>"
        "	     <entry name='a1' type='int32'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "    <struct name='B' version='1' align='1'>"
        "	     <entry name='b1' type='int16'/>"
        "	     <entry name='b2' type='int16'/>"
        "    </struct>"
        "    <union name='U' version='1'>"
        "	     <entry name='u1' type='A' id='2'/>"
        "	     <entry name='u2' type='B' id='3'/>"
        "    </union>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='type' type='int16'/>"
        "        <entry name='data' type='U' select='type'/>"
        "    </struct>"
        "</metalib>");

    EXPECT_EQ(
        8,
        read(
            "data:\n"
            "  u2:\n"
            "    b1: 12\n"
            "    b2: 90\n"
            ,
            "S"));

    EXPECT_EQ(3, dr_ctype_read_int16(result(0), CPE_DR_TYPE_INT16));
    EXPECT_EQ(12, dr_ctype_read_int16(result(2), CPE_DR_TYPE_INT16));
    EXPECT_EQ(90, dr_ctype_read_int16(result(4), CPE_DR_TYPE_INT16));
}

TEST_F(ReadTest, union_with_select_no_data) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='A' version='1' align='1'>"
        "	     <entry name='a1' type='int32'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "    <struct name='B' version='1' align='1'>"
        "	     <entry name='b1' type='int16'/>"
        "	     <entry name='b2' type='int16'/>"
        "    </struct>"
        "    <union name='U' version='1'>"
        "	     <entry name='u1' type='A' id='2'/>"
        "	     <entry name='u2' type='B' id='3'/>"
        "    </union>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='type' type='int16'/>"
        "        <entry name='data' type='U' select='type'/>"
        "    </struct>"
        "</metalib>");

    EXPECT_EQ(
        8,
        read(
            "type: 5\n"
            ,
            "S"));

    EXPECT_EQ(5, dr_ctype_read_int16(result(0), CPE_DR_TYPE_INT16));
}

