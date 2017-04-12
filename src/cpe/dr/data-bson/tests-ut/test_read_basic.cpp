#include "ReadTest.hpp"

TEST_F(ReadTest, type_uin32) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(4, read("S", "a1: 150"));

    EXPECT_CFG_EQ(
        "a1: 150"
        ,
        result());
}

TEST_F(ReadTest, type_uin64) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='uint64' id='1'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(8, read("S", "a1: 150"));

    EXPECT_CFG_EQ(
        "a1: 150"
        ,
        result());
}

TEST_F(ReadTest, type_in32) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int32' id='1'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(4, read("S", "a1: 150"));

    EXPECT_CFG_EQ(
        "a1: 150"
        ,
        result());
}

TEST_F(ReadTest, type_in64) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int64' id='1'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(8, read("S", "a1: 150"));

    EXPECT_CFG_EQ(
        "a1: 150"
        ,
        result());
}

TEST_F(ReadTest, type_float) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='float' id='1'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(4, read("S", "a1: 12.3"));

    EXPECT_CFG_EQ(
        "a1: 12.3"
        ,
        result());
}

TEST_F(ReadTest, type_double) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='double' id='1'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(8, read("S", "a1: 12.3"));

    EXPECT_CFG_EQ(
        "a1: 12.3"
        ,
        result());
}


TEST_F(ReadTest, type_string) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='string' id='1' size='5'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(5, read("S", "a1: abc"));

    EXPECT_CFG_EQ(
        "a1: abc"
        ,
        result());
}

TEST_F(ReadTest, type_string_overflow) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='I' version='1' align='1'>"
        "	     <entry name='a1' type='string' id='1' size='15'/>"
        "    </struct>"
        "    <struct name='O' version='1' align='1'>"
        "	     <entry name='a1' type='string' id='1' size='5'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(5, read("I", "O", "a1: abcde"));

    EXPECT_CFG_EQ(
        "a1: abcd"
        ,
        result());
}

