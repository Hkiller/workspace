#include <algorithm>
#include <set>
#include "cpe/utils/random.h"
#include "RangeMgrTest.hpp"


TEST_F(RangeMgrTest, get_one_basic) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    EXPECT_EQ((ptr_int_t)18, cpe_range_get_one(&m_ra));
    EXPECT_EQ((ptr_int_t)19, cpe_range_get_one(&m_ra));
    EXPECT_EQ((ptr_int_t)-1, cpe_range_get_one(&m_ra));
}

TEST_F(RangeMgrTest, get_one_basic_with_multi_range) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 22, 23));

    EXPECT_EQ((ptr_int_t)18, cpe_range_get_one(&m_ra));
    EXPECT_EQ((ptr_int_t)19, cpe_range_get_one(&m_ra));
    EXPECT_EQ((ptr_int_t)22, cpe_range_get_one(&m_ra));
    EXPECT_EQ((ptr_int_t)-1, cpe_range_get_one(&m_ra));
}

TEST_F(RangeMgrTest, get_one_from_empty) {
    EXPECT_EQ(-1, cpe_range_get_one(&m_ra));
}

TEST_F(RangeMgrTest, get_one_left) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 6, 8));

    EXPECT_EQ(6, cpe_range_get_one(&m_ra));

    EXPECT_STREQ("[7~8),[10~12)", dump());
}

TEST_F(RangeMgrTest, get_one_empty) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 6, 7));

    EXPECT_EQ(6, cpe_range_get_one(&m_ra));

    EXPECT_STREQ("[10~12)", dump());
}

TEST_F(RangeMgrTest, get_range_from_empty) {
    struct cpe_range r = cpe_range_get_range(&m_ra, 10);
    EXPECT_EQ(-1, r.m_start);
    EXPECT_EQ(-1, r.m_end);
}

TEST_F(RangeMgrTest, get_range_left) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 6, 12));

    struct cpe_range r = cpe_range_get_range(&m_ra, 2);
    EXPECT_EQ(6, r.m_start);
    EXPECT_EQ(8, r.m_end);

    EXPECT_STREQ("[8~12),[18~20)", dump());
}

TEST_F(RangeMgrTest, get_range_full) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 6, 12));

    struct cpe_range r = cpe_range_get_range(&m_ra, 6);
    EXPECT_EQ(6, r.m_start);
    EXPECT_EQ(12, r.m_end);

    EXPECT_STREQ("[18~20)", dump());
}

TEST_F(RangeMgrTest, get_range_not_enouth) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 6, 12));

    struct cpe_range r = cpe_range_get_range(&m_ra, 8);
    EXPECT_EQ(6, r.m_start);
    EXPECT_EQ(12, r.m_end);

    EXPECT_STREQ("[18~20)", dump());
}

TEST_F(RangeMgrTest, put_one_to_empty) {
    EXPECT_EQ(0, cpe_range_put_one(&m_ra, 10));
    EXPECT_STREQ("[10~11)", dump());
}

TEST_F(RangeMgrTest, put_one_1) {
    EXPECT_EQ(0, cpe_range_put_one(&m_ra, 1));
    EXPECT_STREQ("[1~2)", dump());
}

TEST_F(RangeMgrTest, put_one_to_begin_not_connect) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_one(&m_ra, 8));

    EXPECT_STREQ("[8~9),[10~12)", dump());
}

TEST_F(RangeMgrTest, put_one_to_begin_connect) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_one(&m_ra, 9));

    EXPECT_STREQ("[9~12)", dump());
}

TEST_F(RangeMgrTest, put_one_to_end_not_connect) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_one(&m_ra, 13));

    EXPECT_STREQ("[10~12),[13~14)", dump());
}

TEST_F(RangeMgrTest, put_one_to_end_connect) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_one(&m_ra, 12));

    EXPECT_STREQ("[10~13)", dump());
}

TEST_F(RangeMgrTest, put_one_to_middle) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_one(&m_ra, 11));

    EXPECT_STREQ("[10~12)", dump());
}

TEST_F(RangeMgrTest, put_range_to_empty) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_STREQ("[10~12)", dump());
}

TEST_F(RangeMgrTest, put_range_start_invalid) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));

    EXPECT_EQ(-1, cpe_range_put_range(&m_ra, -3, 10));

    EXPECT_STREQ("[10~12)", dump());
}

TEST_F(RangeMgrTest, put_range_end_invalid) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));

    EXPECT_EQ(-1, cpe_range_put_range(&m_ra, 8, -1));

    EXPECT_STREQ("[10~12)", dump());
}

TEST_F(RangeMgrTest, put_range_end_lt_start) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));

    EXPECT_EQ(-1, cpe_range_put_range(&m_ra, 8, 7));

    EXPECT_STREQ("[10~12)", dump());
}

TEST_F(RangeMgrTest, put_range_empty) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));

    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 10));

    EXPECT_STREQ("[10~12)", dump());
}

TEST_F(RangeMgrTest, put_range_to_begin_not_connect) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 6, 8));

    EXPECT_STREQ("[6~8),[10~12)", dump());
}

TEST_F(RangeMgrTest, put_range_to_begin_connect) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 6, 10));

    EXPECT_STREQ("[6~12)", dump());
}

TEST_F(RangeMgrTest, put_range_to_begin_connect_in_middle) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 6, 11));

    EXPECT_STREQ("[6~12)", dump());
}

TEST_F(RangeMgrTest, put_range_to_begin_connect_pass_to_end) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 6, 13));

    EXPECT_STREQ("[6~13)", dump());
}

TEST_F(RangeMgrTest, put_range_to_begin_connect_pass_to_next) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 15, 16));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 17, 18));
    EXPECT_STREQ("[10~12),[15~16),[17~18)", dump());

    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 6, 17));

    EXPECT_STREQ("[6~18)", dump());
}

TEST_F(RangeMgrTest, put_range_to_end_not_connect) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 14, 16));

    EXPECT_STREQ("[10~12),[14~16)", dump());
}

TEST_F(RangeMgrTest, put_range_to_end_connect) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 12, 16));

    EXPECT_STREQ("[10~16)", dump());
}

TEST_F(RangeMgrTest, put_range_to_end_connect_in_middle) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 11, 16));

    EXPECT_STREQ("[10~16)", dump());
}

TEST_F(RangeMgrTest, put_range_to_end_connect_pass_to_begin) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 16));

    EXPECT_STREQ("[10~16)", dump());
}

TEST_F(RangeMgrTest, put_range_to_end_connect_pass_to_prev) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 9, 16));

    EXPECT_STREQ("[9~16)", dump());
}

TEST_F(RangeMgrTest, put_range_to_middle_not_connect) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));
    EXPECT_STREQ("[10~12),[18~20)", dump());

    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 13, 17));
    EXPECT_STREQ("[10~12),[13~17),[18~20)", dump());
}

TEST_F(RangeMgrTest, put_range_to_middle_connect_pre) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));
    EXPECT_STREQ("[10~12),[18~20)", dump());

    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 12, 17));
    EXPECT_STREQ("[10~17),[18~20)", dump());
}

TEST_F(RangeMgrTest, put_range_to_middle_connect_pre_to_middle) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));
    EXPECT_STREQ("[10~12),[18~20)", dump());

    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 11, 17));
    EXPECT_STREQ("[10~17),[18~20)", dump());
}

TEST_F(RangeMgrTest, put_range_to_middle_connect_pre_to_begin) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));
    EXPECT_STREQ("[10~12),[18~20)", dump());

    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 17));
    EXPECT_STREQ("[10~17),[18~20)", dump());
}

TEST_F(RangeMgrTest, put_range_to_middle_connect_pre_pass_begin) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));
    EXPECT_STREQ("[10~12),[18~20)", dump());

    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 9, 17));
    EXPECT_STREQ("[9~17),[18~20)", dump());
}

TEST_F(RangeMgrTest, put_range_to_middle_connect_pre_to_pre) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 8, 9));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 9, 17));
    EXPECT_STREQ("[8~17),[18~20)", dump());
}

TEST_F(RangeMgrTest, put_range_to_middle_connect_next) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 13, 18));
    EXPECT_STREQ("[10~12),[13~20)", dump());
}

TEST_F(RangeMgrTest, put_range_to_middle_connect_next_to_middle) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 13, 19));
    EXPECT_STREQ("[10~12),[13~20)", dump());
}

TEST_F(RangeMgrTest, put_range_to_middle_connect_next_to_end) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 13, 20));
    EXPECT_STREQ("[10~12),[13~20)", dump());
}

TEST_F(RangeMgrTest, put_range_to_middle_connect_next_pass_end) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 13, 21));
    EXPECT_STREQ("[10~12),[13~21)", dump());
}

TEST_F(RangeMgrTest, put_range_to_middle_connect_next_to_next) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 23, 27));

    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 13, 23));
    EXPECT_STREQ("[10~12),[13~27)", dump());
}

TEST_F(RangeMgrTest, put_range_to_middle_connect_both) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 12, 18));
    EXPECT_STREQ("[10~20)", dump());
}

TEST_F(RangeMgrTest, find_range_first_before) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    struct cpe_range range = cpe_range_find(&m_ra, 9);
    EXPECT_TRUE(!cpe_range_is_valid(range));
}

TEST_F(RangeMgrTest, find_range_first_begin) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    struct cpe_range range = cpe_range_find(&m_ra, 10);
    EXPECT_EQ(10, range.m_start);
}

TEST_F(RangeMgrTest, find_range_first_middle) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    struct cpe_range range = cpe_range_find(&m_ra, 11);
    EXPECT_EQ(10, range.m_start);
}

TEST_F(RangeMgrTest, find_range_first_last) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    struct cpe_range range = cpe_range_find(&m_ra, 12);
    ASSERT_TRUE(!cpe_range_is_valid(range));
}

TEST_F(RangeMgrTest, find_range_middle) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    struct cpe_range range = cpe_range_find(&m_ra, 13);
    ASSERT_TRUE(!cpe_range_is_valid(range));
}

TEST_F(RangeMgrTest, find_range_last_begin) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    struct cpe_range range = cpe_range_find(&m_ra, 18);
    EXPECT_EQ(18, range.m_start);
}

TEST_F(RangeMgrTest, find_range_last_middle) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    struct cpe_range range = cpe_range_find(&m_ra, 19);
    EXPECT_EQ(18, range.m_start);
}

TEST_F(RangeMgrTest, find_range_last_end) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    struct cpe_range range = cpe_range_find(&m_ra, 20);
    ASSERT_TRUE(!cpe_range_is_valid(range));
}

TEST_F(RangeMgrTest, find_range_last_pass_end) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    struct cpe_range range = cpe_range_find(&m_ra, 21);
    ASSERT_TRUE(!cpe_range_is_valid(range));
}

TEST_F(RangeMgrTest, remove_one_start) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 13));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    EXPECT_EQ(1, cpe_range_remove_one(&m_ra, 10));
    EXPECT_STREQ("[11~13),[18~20)", dump());
}

TEST_F(RangeMgrTest, remove_one_before_start) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 13));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    EXPECT_EQ(0, cpe_range_remove_one(&m_ra, 9));
    EXPECT_STREQ("[10~13),[18~20)", dump());
}

TEST_F(RangeMgrTest, remove_one_middle) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 13));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    EXPECT_EQ(1, cpe_range_remove_one(&m_ra, 11));
    EXPECT_STREQ("[10~11),[12~13),[18~20)", dump());
}

TEST_F(RangeMgrTest, remove_one_last) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 13));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    EXPECT_EQ(1, cpe_range_remove_one(&m_ra, 12));
    EXPECT_STREQ("[10~12),[18~20)", dump());
}

TEST_F(RangeMgrTest, remove_one_middle_range) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 13));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    EXPECT_EQ(0, cpe_range_remove_one(&m_ra, 13));
    EXPECT_EQ(0, cpe_range_remove_one(&m_ra, 17));
    EXPECT_STREQ("[10~13),[18~20)", dump());
}

TEST_F(RangeMgrTest, remove_one_after_last) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 13));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    EXPECT_EQ(0, cpe_range_remove_one(&m_ra, 20));
    EXPECT_EQ(0, cpe_range_remove_one(&m_ra, 21));
    EXPECT_STREQ("[10~13),[18~20)", dump());
}

TEST_F(RangeMgrTest, ranges_empty) {
    struct cpe_range_it it;
    cpe_range_mgr_ranges(&it, &m_ra);

    struct cpe_range range = cpe_range_it_next(&it);
    EXPECT_TRUE(!cpe_range_is_valid(range));
}

TEST_F(RangeMgrTest, ranges_basic) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 10, 12));
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 18, 20));

    struct cpe_range_it it;
    cpe_range_mgr_ranges(&it, &m_ra);

    struct cpe_range range = cpe_range_it_next(&it);
    EXPECT_EQ(10, range.m_start);
    EXPECT_EQ(12, range.m_end);

    range = cpe_range_it_next(&it);
    EXPECT_EQ(18, range.m_start);
    EXPECT_EQ(20, range.m_end);

    range = cpe_range_it_next(&it);
    EXPECT_TRUE(!cpe_range_is_valid(range));
}

TEST_F(RangeMgrTest, is_valid_basic) {
    struct cpe_range r = {1, 2};
    EXPECT_TRUE(cpe_range_is_valid(r));
}

TEST_F(RangeMgrTest, is_valid_empty) {
    struct cpe_range r = {1, 1};
    EXPECT_TRUE(cpe_range_is_valid(r));
}

TEST_F(RangeMgrTest, is_valid_start_negative) {
    struct cpe_range r = {-1, 1};
    EXPECT_FALSE(cpe_range_is_valid(r));
}

TEST_F(RangeMgrTest, is_valid_end_negative) {
    struct cpe_range r = {1, -1};
    EXPECT_FALSE(cpe_range_is_valid(r));
}

TEST_F(RangeMgrTest, is_valid_lenght_error) {
    struct cpe_range r = {2, 1};
    EXPECT_FALSE(cpe_range_is_valid(r));
}

TEST_F(RangeMgrTest, size_basic) {
    struct cpe_range r = {1, 2};
    EXPECT_EQ(1, cpe_range_size(r));
}

TEST_F(RangeMgrTest, size_empty) {
    struct cpe_range r = {1, 1};
    EXPECT_EQ(0, cpe_range_size(r));
}

TEST_F(RangeMgrTest, size_start_negative) {
    struct cpe_range r = {-1, 1};
    EXPECT_EQ(-1, cpe_range_size(r));
}

TEST_F(RangeMgrTest, size_end_negative) {
    struct cpe_range r = {1, -1};
    EXPECT_EQ(-1, cpe_range_size(r));
}

TEST_F(RangeMgrTest, size_lenght_error) {
    struct cpe_range r = {2, 1};
    EXPECT_EQ(-1, cpe_range_size(r));
}

TEST_F(RangeMgrTest, random_get_put) {
    EXPECT_EQ(0, cpe_range_put_range(&m_ra, 0, 20480));

    ::std::set<ptr_int_t> geted;

    size_t repeat_count = 50000;
    while(repeat_count > 0) {
        if (geted.empty() || cpe_rand_dft(100) < 60) {
            ptr_int_t v = cpe_range_get_one(&m_ra);
            EXPECT_TRUE(geted.insert(v).second);
        }
        else {
            ::std::set<ptr_int_t>::iterator pos = geted.begin();
            ::std::advance(pos, cpe_rand_dft(geted.size()));

            EXPECT_EQ(0, cpe_range_put_one(&m_ra, *pos));

            geted.erase(pos);
        }
        --repeat_count;
    }
}
