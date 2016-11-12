#include "ReadTest.hpp"

TEST_F(ReadTest, struct_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='I' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='b1' type='I' id='1'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(4, read("S", "b1: { a1: 150 }"));

    EXPECT_CFG_EQ(
        "b1: { a1: 150 }"
        ,
        result());
}

TEST_F(ReadTest, union_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <union name='U1' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "	     <entry name='a2' type='uint32' id='2'/>"
        "    </union>"
        "    <struct name='S2' version='1' align='1'>"
        "        <entry name='s' type='uint32'/>"
        "	     <entry name='b1' type='U1' id='3' select='s'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(8, read("S2", "b1: { a1: 150 }"));
    
    EXPECT_CFG_EQ(
        "s: 1\n"
        "b1: { a1: 150 }"
        ,
        result());
}

TEST_F(ReadTest, union_select_carry) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <union name='U1' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "	     <entry name='a2' type='uint32' id='2'/>"
        "    </union>"
        "    <struct name='S2' version='1' align='1'>"
        "        <entry name='s' type='uint32' id='1'/>"
        "	     <entry name='b1' type='U1' id='3' select='s'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(8, read("S2", "b1: { a1: 150 }"));
    
    EXPECT_CFG_EQ(
        "s: 1\n"
        "b1: { a1: 150 }"
        ,
        result());
}

TEST_F(ReadTest, union_select_no_daata) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <union name='U1' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "	     <entry name='a2' type='uint32' id='2'/>"
        "    </union>"
        "    <struct name='S2' version='1' align='1'>"
        "        <entry name='s' type='uint32' id='1'/>"
        "	     <entry name='b1' type='U1' id='3' select='s'/>"
        "    </struct>"
        "</metalib>"
        );

    t_em_set_print();
    EXPECT_EQ(4, read("S2", "s: 3"));
    
    EXPECT_CFG_EQ(
        "s: 3\n"
        ,
        result());
}

TEST_F(ReadTest, union_no_select) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <union name='U1' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "	     <entry name='a2' type='uint32' id='2'/>"
        "    </union>"
        "    <struct name='S2' version='1' align='1'>"
        "	     <entry name='b1' type='U1' id='3'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(4, read("S2", "b1: { a1: 150 }"));
    
    EXPECT_CFG_EQ(
        "b1: { a1: 150, a2: 150 }"
        ,
        result());
}

TEST_F(ReadTest, struct_array) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <union name='U1' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "	     <entry name='a2' type='uint32' id='2'/>"
        "    </union>"
        "    <struct name='S2' version='1' align='1'>"
        "	     <entry name='type' type='uint16' id='1'/>"
        "	     <entry name='b1' type='U1' id='2' select='type'/>"
        "    </struct>"
        "    <struct name='A' version='1' align='1'>"
        "	     <entry name='count' type='uint16'/>"
        "	     <entry name='data' type='S2' id='1' count='0' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(8, read("A", "data: [ { type: 1, b1: { a1: 150 } } ]"));
    
    EXPECT_CFG_EQ(
        "count: 1\n"
        "data:\n"
        "  - type: 1\n"
        "    b1: { a1: 150 }"
        ,
        result());
}
