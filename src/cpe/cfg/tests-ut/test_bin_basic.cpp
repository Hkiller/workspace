#include "BinTest.hpp"

TEST_F(BinTest, basic_type) {
    ASSERT_STREQ(
        "--- !int8 6\n"
        "...\n"
        ,
        result(build_by_bin(CPE_CFG_TYPE_INT8, "6")));
}

TEST_F(BinTest, basic_type_string) {
    ASSERT_STREQ(
        "--- 'abcd'\n"
        "...\n"
        ,
        result(build_by_bin(CPE_CFG_TYPE_STRING, "abcd")));
}

TEST_F(BinTest, basic_struct) {
    ASSERT_STREQ(
        "---\n"
        "a: 6\n"
        "b: 7\n"
        "...\n"
        ,
        result(
            build_by_bin(
                t_cfg_parse(
                    "a: 6\n"
                    "b: 7\n"
                    ))));
}

TEST_F(BinTest, basic_struct_string) {
    ASSERT_STREQ(
        "---\n"
        "a: 'abcd'\n"
        "b: 7\n"
        "...\n"
        ,
        result(
            build_by_bin(
                t_cfg_parse(
                    "a: 'abcd'\n"
                    "b: 7\n"
                    ))));
}

TEST_F(BinTest, basic_struct_seq) {
    ASSERT_STREQ(
        "---\n"
        "a:\n"
        "- 'abcd'\n"
        "- 8\n"
        "b: 7\n"
        "...\n"
        ,
        result(
            build_by_bin(
                t_cfg_parse(
                    "a:\n"
                    "    - 'abcd'\n"
                    "    - 8\n"
                    "b: 7\n"
                    ))));
}

TEST_F(BinTest, basic_seq) {
    ASSERT_STREQ(
        "---\n"
        "- 6\n"
        "- 7\n"
        "...\n"
        ,
        result(
            build_by_bin(
                t_cfg_parse(
                    "- 6\n"
                    "- 7\n"
                    ))));
}


TEST_F(BinTest, basic_seq_str) {
    ASSERT_STREQ(
        "---\n"
        "- 'abcd'\n"
        "- 7\n"
        "...\n"
        ,
        result(
            build_by_bin(
                t_cfg_parse(
                    "- abcd\n"
                    "- 7\n"
                    ))));
}

TEST_F(BinTest, basic_seq_struct) {
    cfg_t input = t_cfg_parse(
                    "- a: abcd\n"
                    "  b: 8\n"
                    "- 7\n");

    ASSERT_STREQ(
        "---\n"
        "-   a: 'abcd'\n"
        "    b: 8\n"
        "- 7\n"
        "...\n"
        ,
        result(build_by_bin(input)));
}

TEST_F(BinTest, test_etc) {
    t_em_set_print();

    cfg_t input = build_by_bin_file("/Users/wangjian/Downloads/etc.bc");
    ASSERT_TRUE(input != NULL);

    ASSERT_STREQ(
        "---\n"
        "-   a: 'abcd'\n"
        "    b: 8\n"
        "- 7\n"
        "...\n"
        ,
        result(input));
}
