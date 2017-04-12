#include <vector>
#include "EventCenterTest.hpp"

TEST_F(EventCenterTest, basic) {
    EventResponserMock eventResponser;

    t_evt_mgr_ex().registerResponser(
        "oid-1",
        eventResponser, 
        &EventResponserMock::on_event1);

    Gd::Evt::Event & event = t_evt_mgr_ex().createEvent("event1");
    event["a"] = 1;
    t_evt_mgr_ex().sendEvent("oid-1", event);
    
    EXPECT_CALL(eventResponser, on_event1(::testing::StrEq("oid-1"), ::testing::Ref(event)))
        .WillOnce(::testing::Return());

    t_app_tick();

    ::testing::Mock::VerifyAndClear(&eventResponser);
}

TEST_F(EventCenterTest, remove_by_responser) {
    EventResponserMock eventResponser;

    t_evt_mgr_ex().registerResponser(
        "oid-1",
        eventResponser, 
        &EventResponserMock::on_event1);

    t_evt_mgr_ex().unregisterResponser(eventResponser);

    Gd::Evt::Event & event = t_evt_mgr_ex().createEvent("event1");
    event["a"] = 1;
    t_evt_mgr_ex().sendEvent("oid-1", event);

    t_app_tick();

    ::testing::Mock::VerifyAndClear(&eventResponser);
}

TEST_F(EventCenterTest, many_registe) {
    EventResponserMock eventResponser;

    for(int i = 0; i < 2000; ++i) {
        t_evt_mgr_ex().registerResponser(
            "oid-1",
            eventResponser, 
            &EventResponserMock::on_event1);
    }

    Gd::Evt::Event & event = t_evt_mgr_ex().createEvent("event1");
    event["a"] = 1;
    t_evt_mgr_ex().sendEvent("oid-1", event);
    
    EXPECT_CALL(eventResponser, on_event1(::testing::StrEq("oid-1"), ::testing::Ref(event)))
        .Times(2000)
        .WillRepeatedly(::testing::Return());

    t_app_tick();

    ::testing::Mock::VerifyAndClear(&eventResponser);
}

TEST_F(EventCenterTest, many_responser) {
    ::std::vector<EventResponserMock*> eventResponsers;
    for(int i = 0; i < 2000; ++i) {
        eventResponsers.push_back(new EventResponserMock);
    }

    for(size_t i = 0; i < eventResponsers.size(); ++i) {
        t_evt_mgr_ex().registerResponser(
            "oid-1",
            *eventResponsers[i], 
            &EventResponserMock::on_event1);
    }

    Gd::Evt::Event & event = t_evt_mgr_ex().createEvent("event1");
    event["a"] = 1;
    t_evt_mgr_ex().sendEvent("oid-1", event);


    for(size_t i = 0; i < eventResponsers.size(); ++i) {
        EXPECT_CALL(*eventResponsers[i], on_event1(::testing::StrEq("oid-1"), ::testing::Ref(event)))
            .WillOnce(::testing::Return());
    }
    
    t_app_tick();

    for(size_t i = 0; i < eventResponsers.size(); ++i) {
        ::testing::Mock::VerifyAndClear(eventResponsers[i]);
        delete eventResponsers[i];
    }
}

