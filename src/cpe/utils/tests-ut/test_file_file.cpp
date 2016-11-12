#include "FileTest.hpp"

#ifndef TARGET_IPHONE_SIMULATOR

TEST_F(FileTest, file_write_from_buf_basic) {
    const char * data = "abc";

    EXPECT_EQ(
        3, 
        file_write_from_buf(
            t_path_make("a.txt"),
            data,
            3,
            t_em()));

    EXPECT_STREQ("abc", t_file_to_str("a.txt"));
}

TEST_F(FileTest, file_write_from_string_basic) {
    const char * data = "abc";

    EXPECT_EQ(
        3, 
        file_write_from_str(
            t_path_make("a.txt"),
            data,
            t_em()));

    EXPECT_STREQ("abc", t_file_to_str("a.txt"));
}

TEST_F(FileTest, file_size_basic) {
    t_write_to_file("a.txt", "abc");

    EXPECT_EQ(
        3, 
        file_size(t_path_make("a.txt"), t_em()));
}

TEST_F(FileTest, file_size_not_exist) {
    EXPECT_EQ(
        -1, 
        file_size(t_path_make("a.txt"), t_em()));
}

TEST_F(FileTest, file_stream_size) {
    t_write_to_file("a.txt", "abc");

    FILE * fp = file_stream_open(t_path_make("a.txt"), "r", t_em());
    ASSERT_TRUE(fp);

    EXPECT_EQ(
        3, 
        file_stream_size(fp, t_em()));
}

TEST_F(FileTest, file_append_from_buf_basic) {
    t_write_to_file("a.txt", "abc");

    EXPECT_EQ(
        3, 
        file_append_from_buf(
            t_path_make("a.txt"),
            "def",
            3,
            t_em()));

    EXPECT_STREQ("abcdef", t_file_to_str("a.txt"));
}

TEST_F(FileTest, file_append_from_string_basic) {
    t_write_to_file("a.txt", "abc");

    EXPECT_EQ(
        3, 
        file_append_from_str(
            t_path_make("a.txt"),
            "def",
            t_em()));

    EXPECT_STREQ("abcdef", t_file_to_str("a.txt"));
}

#endif
