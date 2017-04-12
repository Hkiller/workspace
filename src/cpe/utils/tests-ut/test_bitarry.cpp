#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/bitarry.h"

class BitArryTest : public testenv::fixture<> {
public:
    BitArryTest() : m_ba(NULL) {
    }

    virtual void TearDown() {
        if (m_ba) mem_free(t_allocrator(), m_ba);
        m_ba = NULL;
        Base::TearDown();
    }
 
    cpe_ba_t m_ba;
};

TEST_F(BitArryTest, create_basic) {
    m_ba = cpe_ba_create(t_allocrator(), 4);
    cpe_ba_set_all(m_ba, 4, cpe_ba_true);

    EXPECT_STREQ("11110000", cpe_ba_to_str_create(t_tmp_allocrator(), m_ba, 8));
}

TEST_F(BitArryTest, create_multi_byte) {
    m_ba = cpe_ba_create(t_allocrator(), 12);
    cpe_ba_set_all(m_ba, 12, cpe_ba_true);

    EXPECT_STREQ(
        "1111111111110000",
        cpe_ba_to_str_create(t_tmp_allocrator(), m_ba, 16));
}

TEST_F(BitArryTest, to_str_basic) {
    char buf[] = { 0xFF, 0xFF };

    EXPECT_STREQ("1111111111111111", cpe_ba_to_str_create(t_tmp_allocrator(), (cpe_ba_t)buf, 16));
}

TEST_F(BitArryTest, set_false_begin) {
    char buf[] = { 0xFF, 0xFF, 0xFF };

    cpe_ba_set((cpe_ba_t)buf, 0, cpe_ba_false);

    EXPECT_STREQ(
        "011111111111111111111111",
        cpe_ba_to_str_create(t_tmp_allocrator(), (cpe_ba_t)buf, 24));
}

TEST_F(BitArryTest, set_false_last) {
    char buf[] = { 0xFF, 0xFF, 0xFF };

    cpe_ba_set((cpe_ba_t)buf, 23, cpe_ba_false);

    EXPECT_STREQ(
        "111111111111111111111110",
        cpe_ba_to_str_create(t_tmp_allocrator(), (cpe_ba_t)buf, 24));
}

TEST_F(BitArryTest, set_false_middle) {
    char buf[] = { 0xFF, 0xFF, 0xFF };

    cpe_ba_set((cpe_ba_t)buf, 17, cpe_ba_false);

    EXPECT_STREQ(
        "111111111111111110111111",
        cpe_ba_to_str_create(t_tmp_allocrator(), (cpe_ba_t)buf, 24));
}

TEST_F(BitArryTest, count_basic) {
    char buf[] = { 0xFF, 0xFF };

    EXPECT_EQ((size_t)8, cpe_ba_count((cpe_ba_t)buf, 8));
}

TEST_F(BitArryTest, next_pos_full) {
    char buf[] = { 0xFF, 0xFF };

    EXPECT_EQ(-1, cpe_ba_next_pos((cpe_ba_t)buf, 8));
}

TEST_F(BitArryTest, next_pos_full_big) {
    char buf[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    EXPECT_EQ(-1, cpe_ba_next_pos((cpe_ba_t)buf, 36));
}

TEST_F(BitArryTest, next_pos_basic) {
    char buf[] = { 0xFF, 0xFF };
    cpe_ba_set((cpe_ba_t)buf, 3, cpe_ba_false);

    EXPECT_EQ(3, cpe_ba_next_pos((cpe_ba_t)buf, 8));
}

TEST_F(BitArryTest, next_pos_big_begin) {
    char buf[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    cpe_ba_set((cpe_ba_t)buf, 3, cpe_ba_false);

    EXPECT_EQ(3, cpe_ba_next_pos((cpe_ba_t)buf, 36));
}

TEST_F(BitArryTest, next_pos_big_middle) {
    char buf[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    cpe_ba_set((cpe_ba_t)buf, 15, cpe_ba_false);

    EXPECT_EQ(15, cpe_ba_next_pos((cpe_ba_t)buf, 36));
}

TEST_F(BitArryTest, next_pos_big_last) {
    char buf[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    cpe_ba_set((cpe_ba_t)buf, 34, cpe_ba_false);

    EXPECT_EQ(34, cpe_ba_next_pos((cpe_ba_t)buf, 36));
}

TEST_F(BitArryTest, bytes_from_bits) {
    EXPECT_EQ((size_t)0, cpe_ba_bytes_from_bits(0));
    EXPECT_EQ((size_t)1, cpe_ba_bytes_from_bits(1));
    EXPECT_EQ((size_t)1, cpe_ba_bytes_from_bits(8));
    EXPECT_EQ((size_t)2, cpe_ba_bytes_from_bits(9));
}
