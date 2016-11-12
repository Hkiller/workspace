#include "cpe/utils/string_url.h"
#include "cpe/utils/tests-env/test-fixture.hpp"

class StringUrlTest : public testenv::fixture<> {
public:
};

TEST_F(StringUrlTest, basic) {
    char output[128];
    const char * input = "开服最强战机自选大礼包";

    EXPECT_EQ(99, cpe_url_encode(output, sizeof(output), input, strlen(input), NULL));
    EXPECT_STREQ("%E5%BC%80%E6%9C%8D%E6%9C%80%E5%BC%BA%E6%88%98%E6%9C%BA%E8%87%AA%E9%80%89%E5%A4%A7%E7%A4%BC%E5%8C%85", output);
}
