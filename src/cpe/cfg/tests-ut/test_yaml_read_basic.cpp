#include "ReadTest.hpp"

TEST_F(ReadTest, map_basic) {
    EXPECT_EQ(
        0, read(
            "a: abc\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "a: 'abc'\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, map_no_value) {
    EXPECT_EQ(
        0, read(
            "a: \n"
            ));

    EXPECT_STREQ(
        "---\n"
        "a: ''\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, map_name_empty) {
    EXPECT_EQ(
        0, read(
            "'': 1\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "'': 1\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, map_two_item) {
    EXPECT_EQ(
        0, read(
            "a: abc\n"
            "b: abc\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "a: 'abc'\n"
        "b: 'abc'\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, map_map) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "  b: abc\n"
            "c: def\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "a:\n"
        "    b: 'abc'\n"
        "c: 'def'\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, map_map_multi_item) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "  b: abc\n"
            "  c: def\n"
            "d: gh\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "a:\n"
        "    b: 'abc'\n"
        "    c: 'def'\n"
        "d: 'gh'\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, map_to_parent) {
    EXPECT_EQ(
        -1, read(
            "    a: abc\n"
            "b: abc\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "a: 'abc'\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, map_map_map) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "  b:\n"
            "     c: def\n"
            "d: gh\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "a:\n"
        "    b:\n"
        "        c: 'def'\n"
        "d: 'gh'\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, map_seq) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "  - 1\n"
            "  - 2\n"
            "b:\n"
            "  - 3\n"
            "  - 4\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "a:\n"
        "- 1\n"
        "- 2\n"
        "b:\n"
        "- 3\n"
        "- 4\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, map_seq_map) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "  - b1: 1\n"
            "    b2: 2\n"
            "  - b3: 3\n"
            "    b4: 4\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "a:\n"
        "-   b1: 1\n"
        "    b2: 2\n"
        "-   b3: 3\n"
        "    b4: 4\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, map_input_seq) {
    EXPECT_EQ(
        0, read(
            "- 1\n"
            "- 2\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "'':\n"
        "- 1\n"
        "- 2\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, map_input_seq_seq) {
    EXPECT_EQ(
        0, read(
            "-\n"
            "  - '1.1'\n"
            "  - '1.2'\n"
            "-\n"
            "  - '2.1'\n"
            "  - '2.2'\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "'':\n"
        "-   - '1.1'\n"
        "    - '1.2'\n"
        "-   - '2.1'\n"
        "    - '2.2'\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, map_input_seq_map_seq) {
    EXPECT_EQ(
        0, read(
            "- a:\n"
            "  - '1.1'\n"
            "  - '1.2'\n"
            "- b:\n"
            "  - '2.1'\n"
            "  - '2.2'\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "'':\n"
        "-   a:\n"
        "    - '1.1'\n"
        "    - '1.2'\n"
        "-   b:\n"
        "    - '2.1'\n"
        "    - '2.2'\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, root_seq_no_value) {
    EXPECT_EQ(
        0, read(
            "- a:\n"
            "  - \n"
            "  - '1.2'\n"
            "- b:\n"
            "  - '2.1'\n"
            "  - '2.2'\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "'':\n"
        "-   a:\n"
        "    - '1.2'\n"
        "-   b:\n"
        "    - '2.1'\n"
        "    - '2.2'\n"
        "...\n"
        , result());
}
/*
TEST_F(ReadTest, omap_basic) {
    EXPECT_EQ(
        0, read(
            "--- !!omap\n"
            "a: 1\n"
            "c: 3\n"
            "b: 2\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "- a: 1\n"
        "- b: 2\n"
        "- c: 3\n"
        "...\n"
        , result());
}
*/
TEST_F(ReadTest, set_basic) {
    EXPECT_EQ(
        0, read(
            "--- !!set\n"
            "? a\n"
            "? b\n"
            "? c\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "a: ''\n"
        "b: ''\n"
        "c: ''\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, seq_basic) {
    cfg_t seq = cfg_struct_add_seq(m_root, "xxx", cfg_replace);

    EXPECT_EQ(
        0, read(
            seq, 
            "- a:\n"
            "  - \n"
            "  - '1.2'\n"
            "- b:\n"
            "  - '2.1'\n"
            "  - '2.2'\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "-   a:\n"
        "    - '1.2'\n"
        "-   b:\n"
        "    - '2.1'\n"
        "    - '2.2'\n"
        "...\n"
        , result(seq));
}

TEST_F(ReadTest, seq_input_map) {
    cfg_t seq = cfg_struct_add_seq(m_root, "xxx", cfg_replace);

    EXPECT_EQ(
        0, read(
            seq, 
            "a: 1\n"
            "b: 1\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "-   a: 1\n"
        "    b: 1\n"
        "...\n"
        , result(seq));
}

TEST_F(ReadTest, map_schela) {

    EXPECT_EQ(
        0, read(
            "a\n"
            ));

    EXPECT_STREQ(
        "--- {}\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, seq_schela) {
    cfg_t seq = cfg_struct_add_seq(m_root, "xxx", cfg_replace);

    EXPECT_EQ(
        0, read(
            seq,
            "a\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "- 'a'\n"
        "...\n"
        , result(seq));
}

TEST_F(ReadTest, map_multi_document) {
    EXPECT_EQ(
        0, read(
            "a: 1\n"
            "b: 2\n"
            "---\n"
            "c: 3\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "a: 1\n"
        "b: 2\n"
        "c: 3\n"
        "...\n"
        , result());
}

TEST_F(ReadTest, seq_multi_document) {
    cfg_t seq = cfg_struct_add_seq(m_root, "xxx", cfg_replace);

    EXPECT_EQ(
        0, read(
            seq,
            "a: 1\n"
            "b: 2\n"
            "---\n"
            "c: 3\n"
            ));

    EXPECT_STREQ(
        "---\n"
        "-   a: 1\n"
        "    b: 2\n"
        "-   c: 3\n"
        "...\n"
        , result(seq));
}
