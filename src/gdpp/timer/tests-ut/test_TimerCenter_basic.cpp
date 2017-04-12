#include "TimerCenterTest.hpp"

TEST_F(TimerCenterTest, basic) {
    TimerProcessorMock timerProcessor;

    EXPECT_EQ(
        0,
        t_timer_mgr_ex().registerTimer(
            timerProcessor, 
            &TimerProcessorMock::on_timer1,
            0,
            1));

    EXPECT_CALL(timerProcessor, on_timer1(::testing::Eq((Gd::Timer::TimerID)0)))
        .WillOnce(::testing::Return());

    t_app_tick();

    ::testing::Mock::VerifyAndClear(&timerProcessor);
}

TEST_F(TimerCenterTest, multi_times) {
    TimerProcessorMock timerProcessor;

    EXPECT_EQ(
        0,
        t_timer_mgr_ex().registerTimer(
            timerProcessor, 
            &TimerProcessorMock::on_timer1,
            0,
            1,
            -1));

    EXPECT_CALL(timerProcessor, on_timer1(::testing::Eq((Gd::Timer::TimerID)0)))
        .Times(3)
        .WillRepeatedly(::testing::Return());

    t_app_tick();
    t_app_tick();
    t_app_tick();

    ::testing::Mock::VerifyAndClear(&timerProcessor);
}


TEST_F(TimerCenterTest, remove_by_id) {
    TimerProcessorMock timerProcessor;

    EXPECT_EQ(
        0,
        t_timer_mgr_ex().registerTimer(
            timerProcessor, 
            &TimerProcessorMock::on_timer1,
            0,
            1));

    t_timer_mgr_ex().unregisterTimer(0);

    t_app_tick();

    ::testing::Mock::VerifyAndClear(&timerProcessor);
}

TEST_F(TimerCenterTest, remove_by_tl) {
    TimerProcessorMock timerProcessor;

    EXPECT_EQ(
        0,
        t_timer_mgr_ex().registerTimer(
            timerProcessor, 
            &TimerProcessorMock::on_timer1,
            0,
            1,
            1));

    EXPECT_TRUE(t_timer_mgr_ex().haveTimer(0));

    EXPECT_CALL(timerProcessor, on_timer1(::testing::Eq((Gd::Timer::TimerID)0)))
        .WillOnce(::testing::Return());

    t_app_tick();

    EXPECT_TRUE(!t_timer_mgr_ex().haveTimer(0));

    ::testing::Mock::VerifyAndClear(&timerProcessor);
}

TEST_F(TimerCenterTest, remove_by_responser) {
    TimerProcessorMock timerProcessor;

    EXPECT_EQ(
        0,
        t_timer_mgr_ex().registerTimer(
            timerProcessor, 
            &TimerProcessorMock::on_timer1,
            0,
            1));

    t_timer_mgr_ex().unregisterTimer(timerProcessor);

    t_app_tick();

    ::testing::Mock::VerifyAndClear(&timerProcessor);
}

TEST_F(TimerCenterTest, remove_by_responser_multi) {
    TimerProcessorMock timerProcessor;

    EXPECT_EQ(
        0,
        t_timer_mgr_ex().registerTimer(
            timerProcessor, 
            &TimerProcessorMock::on_timer1,
            0,
            1));

    EXPECT_EQ(
        1,
        t_timer_mgr_ex().registerTimer(
            timerProcessor, 
            &TimerProcessorMock::on_timer1,
            0,
            1));

    t_timer_mgr_ex().unregisterTimer(timerProcessor);

    t_app_tick();

    ::testing::Mock::VerifyAndClear(&timerProcessor);
}

TEST_F(TimerCenterTest, register_after_remove) {
    TimerProcessorMock timerProcessor;

    EXPECT_EQ(
        0,
        t_timer_mgr_ex().registerTimer(
            timerProcessor, 
            &TimerProcessorMock::on_timer1,
            0,
            1));

    t_timer_mgr_ex().unregisterTimer(0);

    EXPECT_EQ(
        0,
        t_timer_mgr_ex().registerTimer(
            timerProcessor, 
            &TimerProcessorMock::on_timer2,
            0,
            1));

    EXPECT_CALL(timerProcessor, on_timer2(::testing::Eq((Gd::Timer::TimerID)0)))
        .WillOnce(::testing::Return());

    t_app_tick();

    ::testing::Mock::VerifyAndClear(&timerProcessor);
}
