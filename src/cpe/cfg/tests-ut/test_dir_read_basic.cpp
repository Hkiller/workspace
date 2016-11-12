#include "cpe/utils/tests-env/with_file.hpp"
#include "ReadTest.hpp"

class ReadDirTest
    : public testenv::fixture<LOKI_TYPELIST_1(utils::testenv::with_file), ReadTest>
{
public:
    int read_dir(cfg_policy_t policy) {
        return cfg_read_dir(m_root, t_vfs(), t_dir_base(), policy, t_em(), t_allocrator());
    }
};

TEST_F(ReadDirTest, file_basic) {
    t_write_to_file("f1.yml", "a: abc\n");

    EXPECT_EQ(
        0,
        read_dir(cfg_merge_use_new));

    EXPECT_STREQ(
        "---\n"
        "f1:\n"
        "    a: 'abc'\n"
        "...\n"
        , result());
}

TEST_F(ReadDirTest, file_read_seq) {
    t_write_to_file(
        "f1.yml",
        "- a: abc\n"
        "- b: def\n"
        );

    EXPECT_EQ(
        0,
        read_dir(cfg_merge_use_new));

    EXPECT_STREQ(
        "---\n"
        "f1:\n"
        "-   a: 'abc'\n"
        "-   b: 'def'\n"
        "...\n"
        , result());
}

TEST_F(ReadDirTest, file_ignore_other) {
    t_write_to_file("f1.yml", "a: abc\n");
    t_write_to_file("f1.txt", "a: abc\n");

    EXPECT_EQ(
        0,
        read_dir(cfg_merge_use_new));

    EXPECT_STREQ(
        "---\n"
        "f1:\n"
        "    a: 'abc'\n"
        "...\n"
        , result());
}

TEST_F(ReadDirTest, dir_basic) {
    t_dir_make("a/b");
    t_write_to_file("a/b/f1.yml", "a: abc\n");

    EXPECT_EQ(
        0,
        read_dir(cfg_merge_use_new));

    EXPECT_STREQ(
        "---\n"
        "a:\n"
        "    b:\n"
        "        f1:\n"
        "            a: 'abc'\n"
        "...\n"
        , result());
}

