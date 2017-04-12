#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"

class HashStringTest : public testenv::fixture<> {
public:
};

TEST_F(HashStringTest, buf_basic) {
    cpe_hash_string_buf data = CPE_HS_BUF_MAKE("abc");

    EXPECT_EQ((size_t)3, cpe_hs_len((cpe_hash_string_t)data));

    EXPECT_STREQ("abc", cpe_hs_data((cpe_hash_string_t)data));

    EXPECT_EQ(
        cpe_hash_str(cpe_hs_data((cpe_hash_string_t)data), 3),
        cpe_hs_value((cpe_hash_string_t)data));

    EXPECT_EQ(
        (size_t)12,
        cpe_hs_binary_len((cpe_hash_string_t)data));
}

TEST_F(HashStringTest, alloc_basic) {
    cpe_hash_string_t data = cpe_hs_create(t_allocrator(), "abc");

    EXPECT_EQ((size_t)3, cpe_hs_len(data));

    EXPECT_STREQ("abc", cpe_hs_data(data));

    EXPECT_EQ(
        cpe_hash_str(cpe_hs_data(data), 3),
        cpe_hs_value(data));

    EXPECT_EQ(
        (size_t)12,
        cpe_hs_binary_len(data));

    mem_free(t_allocrator(), data);
}
