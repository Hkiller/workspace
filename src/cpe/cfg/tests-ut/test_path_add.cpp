#include "CfgTest.hpp"

class PathWriteTest : public CfgTest {
};

TEST_F(PathWriteTest, struct_basic) {
    EXPECT_TRUE(cfg_add_struct(m_root, "a", t_em()));
    EXPECT_TRUE(cfg_add_struct(m_root, "b", t_em()));

    EXPECT_STREQ(
        "---\n"
        "a: {}\n"
        "b: {}\n"
        "...\n"
        , result());
}

TEST_F(PathWriteTest, struct_duplicate) {
    EXPECT_TRUE(cfg_add_struct(m_root, "a", t_em()));
    EXPECT_TRUE(cfg_add_struct(m_root, "a", t_em()));

    EXPECT_STREQ(
        "---\n"
        "a: {}\n"
        "...\n"
        , result());
}

TEST_F(PathWriteTest, seq_basic) {
    EXPECT_TRUE(cfg_add_struct(m_root, "a[]", t_em()));
    EXPECT_TRUE(cfg_add_struct(m_root, "a[]", t_em()));

    EXPECT_STREQ(
        "---\n"
        "a:\n"
        "- {}\n"
        "- {}\n"
        "...\n"
        , result());
}

TEST_F(PathWriteTest, string_basic) {
    t_em_set_print();

    cfg_t cfg = cfg_add_string(m_root, "a", "v1", t_em());

    EXPECT_TRUE(cfg);

    EXPECT_STREQ(
        "---\n"
        "a: 'v1'\n"
        "...\n"
        , result());
}

TEST_F(PathWriteTest, string_reset) {
    t_em_set_print();

    EXPECT_TRUE(cfg_add_string(m_root, "a", "v1", t_em()));
    EXPECT_TRUE(cfg_add_string(m_root, "a", "v2", t_em()));

    EXPECT_STREQ(
        "---\n"
        "a: 'v2'\n"
        "...\n"
        , result());
}

TEST_F(PathWriteTest, string_to_sequence) {
    t_em_set_print();

    EXPECT_TRUE(cfg_add_string(m_root, "a[]", "v1", t_em()));
    EXPECT_TRUE(cfg_add_string(m_root, "a[]", "v2", t_em()));

    EXPECT_STREQ(
        "---\n"
        "a:\n"
        "- 'v1'\n"
        "- 'v2'\n"
        "...\n"
        , result());
}

/*
TEST_F(PathWriteTest, string_to_sequence_exist) {
    t_em_set_print();

    EXPECT_TRUE(cfg_add_string(m_root, "a[]", "v1", t_em()));
    EXPECT_TRUE(cfg_add_string(m_root, "a[]", "v2", t_em()));
    EXPECT_TRUE(cfg_add_string(m_root, "a[]", "v3", t_em()));

    EXPECT_TRUE(cfg_add_string(m_root, "a[1]", "v4", t_em()));

    EXPECT_STREQ(
        "---\n"
        "a:\n"
        "- 'v1'\n"
        "- 'v4'\n"
        "- 'v2'\n"
        "...\n"
        , result());
}
*/

TEST_F(PathWriteTest, string_to_sequence_last) {
    t_em_set_print();

    EXPECT_TRUE(cfg_add_string(m_root, "a[0]", "v1", t_em()));
    EXPECT_TRUE(cfg_add_string(m_root, "a[1]", "v2", t_em()));

    EXPECT_STREQ(
        "---\n"
        "a:\n"
        "- 'v1'\n"
        "- 'v2'\n"
        "...\n"
        , result());
}

