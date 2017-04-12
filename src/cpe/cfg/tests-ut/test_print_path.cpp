#include "PrintPathTest.hpp"

TEST_F(PrintPathTest, root_basic) {
    installCfg("a: 1");
    EXPECT_STREQ("a", path("a"));
}

TEST_F(PrintPathTest, multi_level_basic) {
    installCfg(
        "a:\n"
        "  b: 1");
    EXPECT_STREQ("a.b", path("a.b"));
}

TEST_F(PrintPathTest, seq_basic) {
    installCfg(
        "a:\n"
        "  - 1");
    EXPECT_STREQ("a[0]", path("a[0]"));
}

TEST_F(PrintPathTest, seq_struct_basic) {
    installCfg(
        "a:\n"
        "  - b: 1");
    EXPECT_STREQ("a[0].b", path("a[0].b"));
}

TEST_F(PrintPathTest, with_root_seq_struct) {
    installCfg(
        "a:\n"
        "  - b: 1");
    EXPECT_STREQ("[0].b", path("a[0].b", "a"));
}
