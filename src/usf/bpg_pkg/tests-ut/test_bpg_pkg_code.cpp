#include "BpgPkgCodeTest.hpp"

TEST_F(BpgPkgCodeTest, basic_8_to_1) {
    const char * model =
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='A' version='1'>"
        "        <entry name='a1' type='int32' id='1'/>"
        "        <entry name='a2' type='int8' id='2'/>"
        "    </struct>"
        "</metalib>"
        ;

    set_model(model, 8, "pkg-mgr-encode");
    add_cmd(2, "A", "pkg-mgr-encode");

    set_model(model, 1, "pkg-mgr-decode");
    add_cmd(2, "A", "pkg-mgr-decode");

    EXPECT_EQ(
        dr_cvt_result_success,
        encode(
            "sn: 1\n"
            "cmd: 2\n"
            , "pkg-mgr-decode"));

    EXPECT_STREQ(
        m_encode_pkg,
        decode("pkg-mgr-decode"));
}

TEST_F(BpgPkgCodeTest, array_union_8_to_1) {
    const char * model =
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='Pos' version='1'>"
        "        <entry name='category' type='uint8' id='1'/>"
        "        <entry name='pos' type='uint8' id='2'/>"
        "    </struct>"
        "    <struct name='Data' version='1'>"
        "        <entry name='gid' type='uint64' id='1'/>"
        "        <entry name='exp' type='uint32' id='2'/>"
        "        <entry name='hero_id' type='uint16' id='3'/>"
        "        <entry name='pos' type='Pos' id='4'/>"
        "        <entry name='level' type='uint8' id='5'/>"
        "        <entry name='qulity' type='uint8' id='6'/>"
        "    </struct>"
        "    <union name='DataOpData' version='1'>"
        "        <entry name='add' type='Data' id='1'/>"
        "        <entry name='update' type='Data' id='2'/>"
        "        <entry name='remove' type='uint64' id='3'/>"
        "    </union>"
        "    <struct name='DataOp' version='1'>"
        "        <entry name='type' type='int16' id='1'/>"
        "        <entry name='data' type='DataOpData' id='2' select='type'/>"
        "    </struct>"
        "    <struct name='DataOpList' version='1' id='5'>"
        "        <entry name='count' type='uint16' id='1'/>"
        "        <entry name='data' type='DataOp' id='2' count='0' refer='count'/>"
        "    </struct>"
        "</metalib>";

    set_model(model, 1, "pkg-mgr-encode");
    set_model(model, 1, "pkg-mgr-decode");

    EXPECT_EQ(
        dr_cvt_result_success,
        encode(
            "sn: 1\n"
            "cmd: 2\n"
            "DataOpList:\n"
            "    count: 2\n"
            "    data:\n"
            "      - type: 1\n"
            "        data:\n"
            "          add:\n"
            "            gid: 1\n"
            "            exp: 100\n"
            "            hero_id: 2\n"
            "            pos:\n"
            "              category: 10\n"
            "              pos: 11\n"
            "            level: 3\n"
            "            qulity: 12\n"
            "      - type: 2\n"
            "        data:\n"
            "          update:\n"
            "            gid: 2\n"
            "            exp: 200\n"
            "            hero_id: 3\n"
            "            pos:\n"
            "              category: 100\n"
            "              pos: 110\n"
            "            level: 4\n"
            "            qulity: 13\n"
            "      - type: 3\n"
            "        data:\n"
            "          remove: 3\n"
            ,
            "pkg-mgr-decode"));

    // EXPECT_STREQ(
    //     m_encode_pkg,
    //     decode("pkg-mgr-decode"));
}

TEST_F(BpgPkgCodeTest, mem_array_basic) {
    const char * model =
        "<metalib tagsetversion='1' name='net'  version='10'>"
        "    <struct name='A' version='1'>"
        "        <entry name='count' type='uint8'/>"
        "        <entry name='data' type='uint8' id='1' count='0' refer='count'/>"
        "    </struct>"
        "</metalib>"
        ;

    set_model(model, 8);
    add_cmd(2, "A");

    m_pkg = t_bpg_pkg_create();
    ASSERT_TRUE(m_pkg);

    char buf[] = {2, 7, 8};
    
    bpg_pkg_set_cmd(m_pkg, 2);
    EXPECT_EQ(0, bpg_pkg_set_main_data(m_pkg, buf, sizeof(buf), t_em()));

    EXPECT_EQ(dr_cvt_result_success, encode(m_pkg));

    EXPECT_STREQ(m_encode_pkg, decode());
}
