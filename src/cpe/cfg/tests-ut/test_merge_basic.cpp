#include "MergeTest.hpp"

TEST_F(MergeTest, map_basic) {
    merge(
        "a: abc\n"
        );

    EXPECT_STREQ(
        "---\n"
        "a: 'abc'\n"
        "...\n"
        , result());
}

