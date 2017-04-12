#include "ReadTest.hpp"

class PathReadTest : public ReadTest {
};

TEST_F(PathReadTest, map_root) {
    cfg_t cfg = cfg_find_cfg(m_root, "");
    ASSERT_TRUE(cfg == m_root);
}

TEST_F(PathReadTest, map_empty_name) {
    EXPECT_EQ(
        0, read(
            "'': abc\n"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, ".");
    ASSERT_TRUE(cfg);
    EXPECT_STREQ("", cfg_name(cfg));
    EXPECT_STREQ("abc", (const char *)cfg_data(cfg));
}

TEST_F(PathReadTest, map_basic) {
    EXPECT_EQ(
        0, read(
            "a: abc\n"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "a");
    ASSERT_TRUE(cfg);
    EXPECT_STREQ("a", cfg_name(cfg));
    EXPECT_STREQ("abc", (const char *)cfg_data(cfg));
}

TEST_F(PathReadTest, map_not_exist) {
    EXPECT_EQ(
        0, read(
            "a: abc\n"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "c");
    ASSERT_FALSE(cfg);
}

TEST_F(PathReadTest, map_map) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "   b: abc"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "a.b");
    ASSERT_TRUE(cfg);
    EXPECT_STREQ("b", cfg_name(cfg));
}

TEST_F(PathReadTest, map_map_not_exist) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "   b: abc"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "a.c");
    ASSERT_FALSE(cfg);
}

TEST_F(PathReadTest, map_seq) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "   - abc"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "a[0]");
    ASSERT_TRUE(cfg);
    EXPECT_STREQ("abc", (const char *)cfg_data(cfg));
}

TEST_F(PathReadTest, map_seq_filter_string) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "   - a1: aa\n"
            "   - a1: bb"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "a[a1=bb]");
    ASSERT_TRUE(cfg);
    EXPECT_STREQ("bb", cfg_as_string(cfg_find_cfg(cfg, "a1"), NULL));
}

TEST_F(PathReadTest, map_seq_filter_string_not_exist) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "   - a1: aa\n"
            "   - a1: bb"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "a[a1=cc]");
    ASSERT_TRUE(cfg == NULL);
}

TEST_F(PathReadTest, map_seq_filter_int) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "   - a1: -4\n"
            "   - a1: -5"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "a[a1=-5]");
    ASSERT_TRUE(cfg);
    EXPECT_EQ((int32_t)-5, cfg_as_int32(cfg_find_cfg(cfg, "a1"), 0));
}

TEST_F(PathReadTest, map_seq_filter_uint) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "   - a1: 4\n"
            "   - a1: 5"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "a[a1=4]");
    ASSERT_TRUE(cfg);
    EXPECT_EQ((int32_t)4, cfg_as_int32(cfg_find_cfg(cfg, "a1"), 0));
}

TEST_F(PathReadTest, map_seq_seq) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "   - \n"
            "      - 123"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "a[0][0]");
    ASSERT_TRUE(cfg);
    EXPECT_EQ(123, cfg_as_int32(cfg, -1));
}

TEST_F(PathReadTest, map_seq_map) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "   - b: 123\n"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "a[0].b");
    ASSERT_TRUE(cfg);
    EXPECT_EQ(123, cfg_as_int32(cfg, -1));
}

TEST_F(PathReadTest, map_seq_seq_map) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "    -\n"
            "       - b: 123\n"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "a[0][0].b");
    ASSERT_TRUE(cfg);
    EXPECT_EQ(123, cfg_as_int32(cfg, -1));
}

TEST_F(PathReadTest, map_map_seq_no_name) {
    cfg_t subMap = cfg_struct_add_struct(m_root, "a", cfg_replace);
    cfg_seq_add_int8(
        cfg_struct_add_seq(subMap, "", cfg_replace),
        12);
    cfg_struct_add_int8(subMap, "b", 33, cfg_replace);

    cfg_t cfg1 = cfg_find_cfg(m_root, "a[0]");
    ASSERT_TRUE(cfg1);
    EXPECT_EQ(12, cfg_as_int32(cfg1, -1));

    cfg_t cfg2 = cfg_find_cfg(m_root, "a.b");
    ASSERT_TRUE(cfg2);
    EXPECT_EQ(33, cfg_as_int32(cfg2, -1));
}

TEST_F(PathReadTest, map_noname_seq_seq_map) {
    EXPECT_EQ(
        0, read(
            "-\n"
            "   - b: 123\n"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "[0][0].b");
    ASSERT_TRUE(cfg);
    EXPECT_EQ(123, cfg_as_int32(cfg, -1));
}

TEST_F(PathReadTest, map_seq_not_close) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "  - b: 123\n"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "a[0.b");
    ASSERT_FALSE(cfg);
}

TEST_F(PathReadTest, map_seq_pos_format_error) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "  - b: 123\n"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "a[aa].b");
    ASSERT_FALSE(cfg);
}

TEST_F(PathReadTest, map_seq_name_sep_in) {
    EXPECT_EQ(
        0, read(
            "a:\n"
            "  - b: 123\n"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "a[0.0].b");
    ASSERT_FALSE(cfg);
}

TEST_F(PathReadTest, map_name_with_space) {
    EXPECT_EQ(
        0, read(
            "a c:\n"
            "  - b: 123\n"
            ));

    cfg_t cfg = cfg_find_cfg(m_root, "a c[0].b");
    ASSERT_TRUE(cfg);
    EXPECT_EQ(123, cfg_as_int32(cfg, -1));
}

TEST_F(PathReadTest, seq_root_map) {
    cfg_t r = cfg_struct_add_seq(m_root, "xxx", cfg_replace);

    EXPECT_EQ(
        0, read(
            r,
            "b: 123\n"
            ));

    cfg_t cfg = cfg_find_cfg(r, "[0].b");
    ASSERT_TRUE(cfg);
    EXPECT_EQ(123, cfg_as_int32(cfg, -1));
}
