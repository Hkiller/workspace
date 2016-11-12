#include "cpe/utils/time_utils.h"
#include "cpe/utils/tests-env/test-fixture.hpp"

class TimeUtilsTest : public testenv::fixture<> {
public:
    const char * ntirv(
        const char * after,
        uint32_t start_time_vn, uint32_t end_time_vn,
        uint32_t start_date_vn, uint32_t end_date_vn)
    {
        return time_to_str(
            next_time_in_range_vn(
                time_from_str(after),
                start_time_vn, end_time_vn, start_date_vn, end_date_vn),
            t_tmp_alloc(64), 64);
    }
};

TEST_F(TimeUtilsTest, str_basic) {
    time_t t = time(0);

    char buf[128];

    EXPECT_EQ(
        t,
        time_from_str(time_to_str(t, buf, sizeof(buf))));
}

TEST_F(TimeUtilsTest, ntirv_no_rule) {
    EXPECT_STREQ(
        "2014-12-12 09:32:33",
        ntirv("2014-12-12 09:32:33", 0, 0, 0, 0));
}

/***** ntirv: time ******/
TEST_F(TimeUtilsTest, ntirv_time_before) {
    EXPECT_STREQ(
        "2014-12-12 09:40:00",
        ntirv("2014-12-12 09:32:33", 94000, 95000, 0, 0));
}

TEST_F(TimeUtilsTest, ntirv_time_first) {
    EXPECT_STREQ(
        "2014-12-12 09:40:00",
        ntirv("2014-12-12 09:40:00", 94000, 95000, 0, 0));
}

TEST_F(TimeUtilsTest, ntirv_time_middle) {
    EXPECT_STREQ(
        "2014-12-12 09:45:32",
        ntirv("2014-12-12 09:45:32", 94000, 95000, 0, 0));
}

TEST_F(TimeUtilsTest, ntirv_time_last) {
    EXPECT_STREQ(
        "2014-12-12 09:50:00",
        ntirv("2014-12-12 09:50:00", 94000, 95000, 0, 0));
}

TEST_F(TimeUtilsTest, ntirv_time_no_end) {
    EXPECT_STREQ(
        "2014-12-12 09:40:00",
        ntirv("2014-12-12 09:40:00", 94000, 0, 0, 0));

    EXPECT_STREQ(
        "2014-12-12 10:40:00",
        ntirv("2014-12-12 10:40:00", 94000, 0, 0, 0));
}

TEST_F(TimeUtilsTest, ntirv_time_no_start) {
    EXPECT_STREQ(
        "2014-12-12 09:40:00",
        ntirv("2014-12-12 09:40:00", 0, 95000, 0, 0));
}

/***** ntirv: date ******/
TEST_F(TimeUtilsTest, ntirv_date_before) {
    EXPECT_STREQ(
        "2014-12-13 00:00:00",
        ntirv("2014-12-12 09:00:00", 0, 0, 20141213, 20141214));
}

TEST_F(TimeUtilsTest, ntirv_date_first) {
    EXPECT_STREQ(
        "2014-12-12 09:40:00",
        ntirv("2014-12-12 09:40:00", 0, 0, 20141212, 20141214));
}

TEST_F(TimeUtilsTest, ntirv_date_last) {
    EXPECT_STREQ(
        "2014-12-14 09:40:00",
        ntirv("2014-12-14 09:40:00", 0, 0, 20141212, 20141214));
}

TEST_F(TimeUtilsTest, ntirv_date_after) {
    EXPECT_EQ(
        (time_t)0,
        next_time_in_range_vn(time_from_str("2014-12-15 00:00:00"), 0, 0, 20141212, 20141214));
}

TEST_F(TimeUtilsTest, ntirv_date_no_start) {
    EXPECT_STREQ(
        "2014-12-14 23:59:59",
        ntirv("2014-12-14 23:59:59", 0, 0, 0, 20141214));
}


/***** ntirv: next day ******/
TEST_F(TimeUtilsTest, ntirv_next_day_basic) {
    EXPECT_STREQ(
        "2014-12-13 09:40:00",
        ntirv("2014-12-12 09:50:01", 94000, 95000, 0, 0));
}

TEST_F(TimeUtilsTest, ntirv_next_day_no_start) {
    EXPECT_STREQ(
        "2014-12-13 00:00:00",
        ntirv("2014-12-12 09:50:01", 0, 95000, 0, 0));
}

TEST_F(TimeUtilsTest, ntirv_next_day_last) {
    EXPECT_EQ(
        (time_t)0,
        next_time_in_range_vn(time_from_str("2014-12-12 09:50:01"), 0, 95000, 0, 20141212));
}

TEST_F(TimeUtilsTest, ntirv_next_day_over_mon) {
    EXPECT_STREQ(
        "2014-02-01 00:00:00",
        ntirv("2014-01-31 09:50:01", 0, 95000, 0, 0));
}

TEST_F(TimeUtilsTest, ntirv_next_day_over_mon_range) {
    EXPECT_STREQ(
        "2014-02-01 00:00:00",
        ntirv("2014-01-31 09:50:01", 0, 95000, 0, 20140201));
}
