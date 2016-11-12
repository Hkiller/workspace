#include "ErrorTest.hpp"

TEST_F(ErrorTest, notify_multi_node) {
    CPE_DEF_ERROR_MONITOR(em, cpe_error_list_collect, m_el_1);
    CPE_DEF_ERROR_MONITOR_ADD(em_follow, &em, cpe_error_list_collect, m_el_2);

    cpe_error_do_notify(&em, "hello %s", "world");
    EXPECT_EQ(1, cpe_error_list_error_count(m_el_1));
    EXPECT_EQ(1, cpe_error_list_have_msg(m_el_1, "hello world"));

    EXPECT_EQ(1, cpe_error_list_error_count(m_el_2));
    EXPECT_EQ(1, cpe_error_list_have_msg(m_el_2, "hello world"));
}

static void call_notify_var(error_monitor_t monitor, const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    cpe_error_do_notify_var(monitor, fmt, args);
    va_end(args);
}

TEST_F(ErrorTest, notify_var_multi_node) {
    CPE_DEF_ERROR_MONITOR(em, cpe_error_list_collect, m_el_1);
    CPE_DEF_ERROR_MONITOR_ADD(em_follow, &em, cpe_error_list_collect, m_el_2);

    call_notify_var(&em, "hello %s", "world");
    EXPECT_EQ(1, cpe_error_list_error_count(m_el_1));
    EXPECT_EQ(1, cpe_error_list_have_msg(m_el_1, "hello world"));

    EXPECT_EQ(1, cpe_error_list_error_count(m_el_2));
    EXPECT_EQ(1, cpe_error_list_have_msg(m_el_2, "hello world"));
}

