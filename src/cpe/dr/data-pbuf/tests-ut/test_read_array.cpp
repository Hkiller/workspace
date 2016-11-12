#include "ReadTest.hpp"

TEST_F(ReadTest, array_uin32) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='4'>"
        "	     <entry name='a1' type='uint32' id='1' count='5'/>"
        "    </struct>"
        "</metalib>"
        );

    t_em_set_print();
    EXPECT_EQ(20, read("S", "a1: [1, 2, 3, 4, 5]"));

    EXPECT_CFG_EQ(
        "a1: [1, 2, 3, 4, 5]"
        ,
        result());
}

TEST_F(ReadTest, array_uin64) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='uint64' id='1' count='5'/>"
        "    </struct>"
        "</metalib>"
        );

    t_em_set_print();
    EXPECT_EQ(40, read("S", "a1: [1, 2, 3, 4, 5]"));

    EXPECT_CFG_EQ(
        "a1: [1, 2, 3, 4, 5]"
        ,
        result());
}

TEST_F(ReadTest, array_with_refer_uin32) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='4'>"
        "        <entry name='count' type='int32'/>"
        "	     <entry name='a1' type='uint32' id='1' count='5' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    t_em_set_print();
    EXPECT_EQ(20, read("S", "a1: [1, 2, 3, 4]"));

    EXPECT_CFG_EQ(
        "count: 4\n"
        "a1: [1, 2, 3, 4]"
        ,
        result());
}

TEST_F(ReadTest, array_in32) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int32' id='1' count='5'/>"
        "    </struct>"
        "</metalib>"
        );

    t_em_set_print();
    EXPECT_EQ(20, read("S", "a1: [1, 2, 3, 4, 5]"));

    EXPECT_CFG_EQ(
        "a1: [1, 2, 3, 4, 5]"
        ,
        result());
}

TEST_F(ReadTest, array_uint8) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='uint8' id='1' count='5'/>"
        "    </struct>"
        "</metalib>"
        );

    t_em_set_print();
    EXPECT_EQ(5, read("S", "a1: [1, 2, 3, 4, 5]"));

    EXPECT_CFG_EQ(
        "a1: [1, 2, 3, 4, 5]"
        ,
        result());
}

TEST_F(ReadTest, array_with_refer_uint8) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "        <entry name='count' type='uint8'/>"
        "	     <entry name='a1' type='uint8' id='1' count='5' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    t_em_set_print();
    EXPECT_EQ(5, read("S", "a1: [1, 2, 3, 4]"));

    EXPECT_CFG_EQ(
        "count: 4\n"
        "a1: [1, 2, 3, 4]"
        ,
        result());
}

TEST_F(ReadTest, array_with_refer_uint8_dyn_len) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "        <entry name='count' type='uint8'/>"
        "	     <entry name='a1' type='uint8' id='1' count='0' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    t_em_set_print();
    EXPECT_EQ(5, read("S", "a1: [1, 2, 3, 4]"));

    EXPECT_CFG_EQ(
        "count: 4\n"
        "a1: [1, 2, 3, 4]"
        ,
        result());
}

TEST_F(ReadTest, array_with_refer_uint8_dyn_len_empty) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "        <entry name='count' type='uint8'/>"
        "	     <entry name='a1' type='uint8' id='1' count='0' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    t_em_set_print();
    EXPECT_EQ(1, read("S", "a1: []"));

    EXPECT_CFG_EQ(
        "count: 0\n"
        "a1: []"
        ,
        result());
}

TEST_F(ReadTest, array_empty) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "        <entry name='count' type='uint8'/>"
        "	     <entry name='a1' type='uint8' id='1' count='5' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    t_em_set_print();
    EXPECT_EQ(1, read("S", "a1: []"));

    EXPECT_CFG_EQ(
        "count: 0\n"
        "a1: []"
        ,
        result());
}

TEST_F(ReadTest, array_float) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='float' id='1' count='5'/>"
        "    </struct>"
        "</metalib>"
        );

    t_em_set_print();
    EXPECT_EQ(20, read("S", "a1: [1.1, 2.2, 3.3, 4.4, 5.5]"));

    EXPECT_CFG_EQ(
        "a1: [1.1, 2.2, 3.3, 4.4, 5.5]"
        ,
        result());
}

TEST_F(ReadTest, array_double) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='double' id='1' count='5'/>"
        "    </struct>"
        "</metalib>"
        );

    t_em_set_print();
    EXPECT_EQ(40, read("S", "a1: [1.1, 2.2, 3.3, 4.4, 5.5]"));

    EXPECT_CFG_EQ(
        "a1: [1.1, 2.2, 3.3, 4.4, 5.5]"
        ,
        result());
}

TEST_F(ReadTest, array_string) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='string' size='7' id='1' count='5'/>"
        "    </struct>"
        "</metalib>"
        );

    t_em_set_print();
    EXPECT_EQ(35, read("S", "a1: [a, b, c, d, e]"));

    EXPECT_CFG_EQ(
        "a1: [a, b, c, d, e]"
        ,
        result());
}


TEST_F(ReadTest, array_struct) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='I' version='1' align='8'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='S' version='1' align='8'>"
        "	     <entry name='b1' type='I' id='1' count='2'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(metaSize("S"), read("S", "b1: [ { a1: 150 }, { a1: 151 } ]"));

    EXPECT_CFG_EQ(
        "b1: [ { a1: 150 }, { a1: 151 } ]"
        ,
        result());
}

TEST_F(ReadTest, array_struct_with_refer) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='I' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='S' version='1' align='1'>"
        "        <entry name='count' type='int32'/>"
        "	     <entry name='b1' type='I' id='1' count='2' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(12, read("S", "b1: [ { a1: 150 }, { a1: 151 } ]"));

    EXPECT_CFG_EQ(
        "count: 2\n"
        "b1: [ { a1: 150 }, { a1: 151 } ]"
        ,
        result());
}

TEST_F(ReadTest, array_struct_with_refer_multi) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='I' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='S' version='1' align='1'>"
        "        <entry name='count_1' type='int32'/>"
        "	     <entry name='b1' type='I' id='1' count='5' refer='count_1'/>"
        "        <entry name='count_2' type='int32'/>"
        "	     <entry name='b2' type='I' id='2' count='5' refer='count_2'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(
        40, 
        read(
            "S",
            "b1: [ { a1: 150 }, { a1: 151 } ]\n"
            "b2: [ { a1: 250 }, { a1: 251 }, { a1: 253 } ]\n"
            ));

    EXPECT_CFG_EQ(
        "count_1: 2\n"
        "b1: [ { a1: 150 }, { a1: 151 } ]\n"
        "count_2: 3\n"
        "b2: [ { a1: 250 }, { a1: 251 }, { a1: 253 } ]"
        ,
        result());
}

TEST_F(ReadTest, array_struct_with_refer_empty) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='I' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='S' version='1' align='1'>"
        "        <entry name='count' type='int32'/>"
        "	     <entry name='b1' type='I' id='1' count='0' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(0, read("S", "b1: [ ]"));
}
