#include "TlTest.hpp"

class TimeLineTest : public TlTest {
    void SetUp() {
        TlTest::SetUp();
        m_tl = tl_create(m_manage);
    }

    void TearDown() {
        m_tl = NULL;
        TlTest::SetUp();
    }
};

TEST_F(TimeLineTest, setopt_dispatcher) {
    EXPECT_EQ(
        0,
        tl_set_opt(
            m_tl, tl_set_event_dispatcher,
            (tl_event_process_t)12));

    EXPECT_TRUE(
        m_tl->m_event_dispatcher == (tl_event_process_t)12);
}

TEST_F(TimeLineTest, setopt_dispatcher_null) {
    EXPECT_EQ(
        0,
        tl_set_opt(
            m_tl, tl_set_event_dispatcher,
            (tl_event_process_t)0));

    EXPECT_TRUE(
        m_tl->m_event_dispatcher == (tl_event_process_t)0);
}

TEST_F(TimeLineTest, setopt_enqueue) {
    EXPECT_EQ(
        0,
        tl_set_opt(
            m_tl, tl_set_event_enqueue,
            (tl_event_enqueue_t)12));

    EXPECT_TRUE(
        m_tl->m_event_enqueue == (tl_event_enqueue_t)12);
}

TEST_F(TimeLineTest, setopt_enqueue_null) {
    EXPECT_EQ(
        CPE_TL_ERROR_EVENT_NO_ENQUEUE,
        tl_set_opt(
            m_tl, tl_set_event_enqueue,
            (tl_event_enqueue_t)0));

    EXPECT_TRUE(
        m_tl->m_event_enqueue == tl_event_enqueue_local);
}

TEST_F(TimeLineTest, setopt_destory) {
    EXPECT_EQ(
        0,
        tl_set_opt(
            m_tl, tl_set_event_destory,
            (tl_event_process_t)12));

    EXPECT_TRUE(
        m_tl->m_event_destory == (tl_event_process_t)12);
}

TEST_F(TimeLineTest, setopt_destory_null) {
    EXPECT_EQ(
        0,
        tl_set_opt(
            m_tl, tl_set_event_destory,
            (tl_event_process_t)0));

    EXPECT_TRUE(
        m_tl->m_event_destory == (tl_event_process_t)0);
}

TEST_F(TimeLineTest, setopt_construct) {
    EXPECT_EQ(
        0,
        tl_set_opt(
            m_tl, tl_set_event_construct,
            (tl_event_process_t)12));

    EXPECT_TRUE(
        m_tl->m_event_construct == (tl_event_process_t)12);
}

TEST_F(TimeLineTest, setopt_construct_null) {
    EXPECT_EQ(
        0,
        tl_set_opt(
            m_tl, tl_set_event_construct,
            (tl_event_process_t)0));

    EXPECT_TRUE(
        m_tl->m_event_construct == (tl_event_process_t)0);
}

TEST_F(TimeLineTest, setopt_op_context) {
    EXPECT_EQ(
        0,
        tl_set_opt(
            m_tl, tl_set_event_op_context,
            (void*)12));

    EXPECT_TRUE(
        m_tl->m_event_op_context == (void*)12);
}

TEST_F(TlTest, setopt_unknown) {
    EXPECT_EQ(
        CPE_TL_ERROR_BAD_ARG,
        tl_set_opt(
            m_tl, (tl_option_t)123));
}
