#include "EventCenterTest.hpp"

void EventCenterTest::SetUp() {
    Base::SetUp();

    t_evt_mgr_set_metalib(
        "<metalib tagsetversion='1' name='event'  version='1'>\n"
        "<struct name='event1' version='1'>\n"
        "    <entry name='a' type='int32'/>\n"
        "</struct>\n"
        "</metalib>\n"
        );

    t_app_set_timer_source_last_event();
}

void EventCenterTest::TearDown() {
    Base::TearDown();
}

