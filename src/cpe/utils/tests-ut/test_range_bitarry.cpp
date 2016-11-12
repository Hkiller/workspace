#include "RangeBitarryTest.hpp"

TEST_F(RangeBitarryTest, init_basic_true) {
    const char * str_ba = "0111101010"; 
    size_t capacity = strlen(str_ba);

    EXPECT_EQ(
        0,
        cpe_range_put_from_ba(
            &m_ra,
            create_ba(str_ba),
            0,
            capacity,
            cpe_ba_true));

    EXPECT_STREQ("[1~5),[6~7),[8~9)", dump());
}

TEST_F(RangeBitarryTest, init_basic_false) {
    const char * str_ba = "0111101010"; 
    size_t capacity = strlen(str_ba);

    EXPECT_EQ(
        0,
        cpe_range_put_from_ba(
            &m_ra,
            create_ba(str_ba),
            0,
            capacity,
            cpe_ba_false));

    EXPECT_STREQ("[0~1),[5~6),[7~8),[9~10)", dump());
}

TEST_F(RangeBitarryTest, init_with_start) {
    const char * str_ba = "0111101010"; 
    size_t capacity = strlen(str_ba);

    EXPECT_EQ(
        0,
        cpe_range_put_from_ba(
            &m_ra,
            create_ba(str_ba),
            12,
            capacity,
            cpe_ba_true));

    EXPECT_STREQ("[13~17),[18~19),[20~21)", dump());
}
