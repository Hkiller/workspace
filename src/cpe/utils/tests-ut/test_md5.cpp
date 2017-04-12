#include "MD5Test.hpp"

TEST_F(MD5Test, test_str_basic) {
    EXPECT_STREQ(
        "9e107d9d372bb6826bd81d3542a419d6",
        str_md5("The quick brown fox jumps over the lazy dog"));

    EXPECT_STREQ(
        "d41d8cd98f00b204e9800998ecf8427e",
        str_md5(""));
}

TEST_F(MD5Test, test_parse) {
    cpe_md5_value parse_md5;
    EXPECT_EQ(0, cpe_md5_parse(&parse_md5, "9e107d9d372bb6826bd81d3542a419d6"));
    EXPECT_STREQ("9e107d9d372bb6826bd81d3542a419d6", to_str(&parse_md5));
}
