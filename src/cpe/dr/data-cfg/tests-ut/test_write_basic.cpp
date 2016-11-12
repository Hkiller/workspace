#include "WriteTest.hpp"

TEST_F(WriteTest, basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>");

    EXPECT_EQ(
        0,
        write(
            "a1: 12\n"
            "a2: 14\n",
            "S"));

}

TEST_F(WriteTest, seq_count_dynamic_with_refer) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1' align='1'>"
        "	     <entry name='count' type='int16'/>"
        "	     <entry name='a1' type='int16' count='0' refer='count'/>"
        "    </struct>"
        "</metalib>");

    EXPECT_EQ(
        0,
        write(
            "a1: [12, 13, 14]\n",
            "S"));


    EXPECT_CFG_EQ(
        "count: 3\n"
        "a1: [12, 13, 14]"
        ,
        m_cfg);
}

TEST_F(WriteTest, union_basic) {
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
        0,
        write(
            "u1:\n"
            "  a1: 12\n"
            "  a2: 90\n"
            ,
            "U"));

    EXPECT_CFG_EQ(
        "a1: 12\n"
        "a2: 90\n"
        ,
        cfg_find_cfg(m_cfg, "u1"));

    EXPECT_TRUE(cfg_find_cfg(m_cfg, "u2"));
}

TEST_F(WriteTest, union_with_select) {
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
        0,
        write(
            "data:\n"
            "  u2:\n"
            "    b1: 12\n"
            "    b2: 90\n"
            ,
            "S"));

    EXPECT_CFG_EQ(
        "type: 3\n"
        "data:\n"
        "  u2:\n"
        "    b1: 12\n"
        "    b2: 90\n"
        ,
        m_cfg);
}
