#include "FileTest.hpp"

#ifndef TARGET_IPHONE_SIMULATOR

TEST_F(FileTest, dir_mk_recursion_basic) {
    EXPECT_EQ(
        0,
        dir_mk_recursion(
            t_path_make("a/b"),
            DIR_DEFAULT_MODE,
            t_em(), NULL));

    EXPECT_TRUE(dir_exist(t_path_make("a/b"), NULL));
}

TEST_F(FileTest, dir_mk_recursion_one_level) {
    EXPECT_EQ(
        0,
        dir_mk_recursion(
            t_path_make("a"),
            DIR_DEFAULT_MODE,
            t_em(), NULL));

    EXPECT_TRUE(dir_exist(t_path_make("a"), NULL));
}

TEST_F(FileTest, dir_mk_recursion_multi_level) {
    EXPECT_EQ(
        0,
        dir_mk_recursion(
            t_path_make("a/b/c"),
            DIR_DEFAULT_MODE,
            t_em(), NULL));

    EXPECT_TRUE(dir_exist(t_path_make("a"), NULL));
    EXPECT_TRUE(dir_exist(t_path_make("a/b"), NULL));
    EXPECT_TRUE(dir_exist(t_path_make("a/b/c"), NULL));
}

TEST_F(FileTest, dir_rm_recursion_with_file) {
    dir_mk(t_path_make("a"), DIR_DEFAULT_MODE, t_em());
    t_write_to_file("a/a.txt", "abc");

    EXPECT_EQ(
        0,
        dir_rm_recursion(t_path_make("a"), t_em(), NULL));

    EXPECT_FALSE(dir_exist(t_path_make("a"), NULL));
}

TEST_F(FileTest, dir_mk_recursion_empty) {
    EXPECT_EQ(
        -1,
        dir_mk_recursion(
            "",
            DIR_DEFAULT_MODE,
            t_em(), NULL));
}

TEST_F(FileTest, dir_exist_basic) {
    EXPECT_EQ(
        1,
        dir_exist(t_path_make("."), t_em()));
}

TEST_F(FileTest, dir_exist_not_exist) {
    EXPECT_EQ(
        0,
        dir_exist(t_path_make("not-exist"), t_em()));
}

TEST_F(FileTest, dir_exist_file) {
    EXPECT_EQ(
        0,
        file_write_from_str(
            t_path_make("f1"),
            "",
            t_em()));

    EXPECT_EQ(
        0,
        dir_exist(t_path_make("not-exist"), t_em()));
}

#endif
