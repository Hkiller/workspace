#include "WriteTest.hpp"

TEST_F(WriteTest, map_empty) {
    EXPECT_EQ(0, write(m_root));
    EXPECT_STREQ(
        "--- {}\n"
        "...\n"
        , result());
}

TEST_F(WriteTest, map_basic) {
    cfg_struct_add_string(m_root, "a", "12", cfg_replace);

    EXPECT_EQ(0, write(m_root));
    EXPECT_STREQ(
        "---\n"
        "a: '12'\n"
        "...\n"
        , result());
}

TEST_F(WriteTest, map_map) {
    cfg_t m = cfg_struct_add_struct(m_root, "a", cfg_replace);
    cfg_struct_add_string(m, "b1", "12", cfg_replace);
    cfg_struct_add_string(m, "b2", "12", cfg_replace);

    EXPECT_EQ(0, write(m_root));
    EXPECT_STREQ(
        "---\n"
        "a:\n"
        "    b1: '12'\n"
        "    b2: '12'\n"
        "...\n"
        , result());
}

TEST_F(WriteTest, seq_empty) {
    cfg_t seq = cfg_struct_add_seq(m_root, "a", cfg_replace);

    EXPECT_EQ(0, write(seq));
    EXPECT_STREQ(
        "--- []\n"
        "...\n"
        , result());
}

TEST_F(WriteTest, seq_basic) {
    cfg_t seq = cfg_struct_add_seq(m_root, "a", cfg_replace);

    cfg_seq_add_string(seq, "12");
    cfg_seq_add_string(seq, "13");

    EXPECT_EQ(0, write(seq));
    EXPECT_STREQ(
        "---\n"
        "- '12'\n"
        "- '13'\n"
        "...\n"
        , result());
}

