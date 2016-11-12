#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/string_utils.h"

class StringReadArgTest : public testenv::fixture<> {
};

TEST_F(StringReadArgTest, empty) {
    char input[] = "";
    char r[6];
    
    ASSERT_TRUE(cpe_str_read_arg(r, sizeof(r), input, "a", ',', '=') != 0);
}

TEST_F(StringReadArgTest, find_last) {
    char input[] = "a=1,b=2,c=3";
    char r[6];

    ASSERT_TRUE(cpe_str_read_arg(r, sizeof(r), input, "c", ',', '=') == 0);
    ASSERT_STREQ("3", r);
    ASSERT_STREQ("a=1,b=2,c=3", input);
}

TEST_F(StringReadArgTest, find_middle) {
    char input[] = "a=1,b=2,c=3";
    char r[6];

    ASSERT_TRUE(cpe_str_read_arg(r, sizeof(r), input, "b", ',', '=') == 0);
    ASSERT_STREQ("2", r);
    ASSERT_STREQ("a=1,b=2,c=3", input);
}

TEST_F(StringReadArgTest, not_exist) {
    char input[] = "a=1,b=2";
    char r[6];
    
    ASSERT_TRUE(cpe_str_read_arg(r, sizeof(r), input, "c", ',', '=') != 0);
    ASSERT_STREQ("a=1,b=2", input);
}

TEST_F(StringReadArgTest, value_overflow) {
    char input[] = "a=1234567";
    char r[6];

    ASSERT_TRUE(cpe_str_read_arg(r, sizeof(r), input, "a", ',', '=') == 0);
    ASSERT_STREQ("12345", r);
    ASSERT_STREQ("a=1234567", input);
}
