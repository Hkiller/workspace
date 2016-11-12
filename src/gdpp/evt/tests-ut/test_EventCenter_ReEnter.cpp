#include <vector>
#include "EventCenterTest.hpp"

class EventResponser_RegistInEvent : public Gd::Evt::EventResponser {
public:
    EventResponser_RegistInEvent(Gd::Evt::EventCenter & ec)
        : m_ec(ec)
    {
    }

    void on_event(const char * oid, Gd::Evt::Event const & e) {
        // m_ec.registerResponser(
        //     "oid-1",
        //     *this, 
        //     &EventResponser_RegistInEvent::on_event);
    }

    Gd::Evt::EventCenter & m_ec;
};

TEST_F(EventCenterTest, register_in_process) {
    EventResponser_RegistInEvent eventResponser(t_evt_mgr_ex());

    t_evt_mgr_ex().registerResponser(
        "oid-1",
        eventResponser, 
        &EventResponser_RegistInEvent::on_event);

    Gd::Evt::Event & event = t_evt_mgr_ex().createEvent("event1");
    event["a"] = 1;
    t_evt_mgr_ex().sendEvent("oid-1", event);

    t_app_tick();
}

class EventResponser_UnregistInEvent : public Gd::Evt::EventResponser {
public:
    EventResponser_UnregistInEvent(Gd::Evt::EventCenter & ec, Gd::Evt::EventResponser & r)
        : m_ec(ec), m_r(r)
    {
    }

    void on_event(const char * oid, Gd::Evt::Event const & e) {
        m_ec.unregisterResponser(m_r);
    }

    Gd::Evt::EventCenter & m_ec;
    Gd::Evt::EventResponser & m_r;
};

TEST_F(EventCenterTest, unregister_in_process) {
    EventResponserMock otherResponser;
    t_evt_mgr_ex().registerResponser(
        "oid-1",
        otherResponser, 
        &EventResponserMock::on_event1);
    EXPECT_CALL(otherResponser, on_event1(::testing::StrEq("oid-1"), ::testing::_ ))
        .WillOnce(::testing::Return());

    EventResponser_UnregistInEvent eventResponser(t_evt_mgr_ex(), otherResponser);

    t_evt_mgr_ex().registerResponser(
        "oid-1",
        eventResponser, 
        &EventResponser_UnregistInEvent::on_event);

    Gd::Evt::Event & event = t_evt_mgr_ex().createEvent("event1");
    event["a"] = 1;
    t_evt_mgr_ex().sendEvent("oid-1", event);
    
    t_app_tick();
}

class EventResponser_UnregistSelfInEvent : public Gd::Evt::EventResponser {
public:
    EventResponser_UnregistSelfInEvent(Gd::Evt::EventCenter & ec) : m_ec(ec) {
    }

    void on_event(const char * oid, Gd::Evt::Event const & e) {
        m_ec.unregisterResponser(*this);
    }

    Gd::Evt::EventCenter & m_ec;
};

TEST_F(EventCenterTest, unregister_self_in_process) {
    EventResponserMock er1;
    t_evt_mgr_ex().registerResponser(
        "oid-1",
        er1, 
        &EventResponserMock::on_event1);
    EXPECT_CALL(er1, on_event1(::testing::StrEq("oid-1"), ::testing::_))
        .WillOnce(::testing::Return());

    EventResponser_UnregistSelfInEvent eventResponser(t_evt_mgr_ex());

    t_evt_mgr_ex().registerResponser(
        "oid-1",
        eventResponser, 
        &EventResponser_UnregistSelfInEvent::on_event);
    
    EventResponserMock er2;
    t_evt_mgr_ex().registerResponser(
        "oid-1",
        er2, 
        &EventResponserMock::on_event1);
    EXPECT_CALL(er2, on_event1(::testing::StrEq("oid-1"), ::testing::_))
        .WillOnce(::testing::Return());

    Gd::Evt::Event & event = t_evt_mgr_ex().createEvent("event1");
    event["a"] = 1;
    t_evt_mgr_ex().sendEvent("oid-1", event);

    t_app_tick();
}

