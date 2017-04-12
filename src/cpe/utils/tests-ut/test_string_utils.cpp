#include "cpe/utils/string_utils.h"
#include "cpe/utils/tests-env/test-fixture.hpp"

class StringUtilsTest : public testenv::fixture<> {
public:
};

TEST_F(StringUtilsTest, str_char_not_in_pair_found) {
    EXPECT_STREQ(
        ",",
        cpe_str_char_not_in_pair("abc[def,g],", ',', "{[(", ")]}")
        );
}

TEST_F(StringUtilsTest, str_char_not_in_pair_not_found_1) {
    EXPECT_TRUE(
        cpe_str_char_not_in_pair("abc[def,g]", ',', "{[(", ")]}") == NULL
        );
}

TEST_F(StringUtilsTest, str_char_not_in_pair_not_found_2) {
    EXPECT_TRUE(
        cpe_str_char_not_in_pair("abc(def,g)", ',', "{[(", ")]}") == NULL
        );
}

TEST_F(StringUtilsTest, str_char_not_in_pair_not_found_3) {
    EXPECT_TRUE(
        cpe_str_char_not_in_pair("abc{def,g}", ',', "{[(", ")]}") == NULL
        );
}
