#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/string_utils.h"

class StringReadArgRangeTest : public testenv::fixture<> {
};

TEST_F(StringReadArgRangeTest, empty) {
    char input[] = "";
    char r[6];
    
    ASSERT_TRUE(cpe_str_read_arg_range(r, sizeof(r), input, input + strlen(input), "a", ',', '=') != 0);
}

TEST_F(StringReadArgRangeTest, find_last) {
    char input[] = "a=1,b=2,c=33";
    char r[6];

    ASSERT_TRUE(cpe_str_read_arg_range(r, sizeof(r), input, input + strlen(input) - 1, "c", ',', '=') == 0);
    ASSERT_STREQ("3", r);
}

TEST_F(StringReadArgRangeTest, find_last_empty) {
    char input[] = "a=1,b=2,c=33";
    char r[6];

    ASSERT_TRUE(cpe_str_read_arg_range(r, sizeof(r), input, input + strlen(input) - 2, "c", ',', '=') == 0);
    ASSERT_STREQ("", r);
}

TEST_F(StringReadArgRangeTest, find_middle) {
    char input[] = "a=1,b=2,c=33";
    char r[6];

    ASSERT_TRUE(cpe_str_read_arg_range(r, sizeof(r), input, input + strlen(input) - 1, "b", ',', '=') == 0);
    ASSERT_STREQ("2", r);
}

TEST_F(StringReadArgRangeTest, not_exist) {
    char input[] = "a=1,b=22";
    char r[6];
    
    ASSERT_TRUE(cpe_str_read_arg_range(r, sizeof(r), input, input + strlen(input) - 1, "c", ',', '=') != 0);
}

TEST_F(StringReadArgRangeTest, value_overflow) {
    char input[] = "a=12345678";
    char r[6];

    ASSERT_TRUE(cpe_str_read_arg_range(r, sizeof(r), input, input + strlen(input) - 1, "a", ',', '=') == 0);
    ASSERT_STREQ("12345", r);
}
