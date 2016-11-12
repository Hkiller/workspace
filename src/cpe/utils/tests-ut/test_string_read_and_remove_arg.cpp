#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/string_utils.h"

class StringReadAndRemoveArgTest : public testenv::fixture<> {
};

TEST_F(StringReadAndRemoveArgTest, empty) {
    char input[] = "";

    ASSERT_TRUE(cpe_str_read_and_remove_arg(input, "a", ',', '=') == NULL);
    ASSERT_STREQ("", input);
}

TEST_F(StringReadAndRemoveArgTest, basic) {
    char input[] = "a=1";

    ASSERT_STREQ("1", cpe_str_read_and_remove_arg(input, "a", ',', '='));
    ASSERT_STREQ("", input);
}

TEST_F(StringReadAndRemoveArgTest, not_exist) {
    char input[] = "a=1,b=2";

    ASSERT_TRUE(NULL == cpe_str_read_and_remove_arg(input, "c", ',', '='));
    ASSERT_STREQ("a=1,b=2", input);
}
