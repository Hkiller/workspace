#include <sstream>
#include "TlTest.hpp"

class DispatchTest : public TlTest {
public:
    void SetUp() {
        TlTest::SetUp();
        installTl();

        tl_manage_set_opt(m_manage, tl_set_time_source_context, m_manage);
        tl_manage_set_opt(m_manage, tl_set_time_source, tl_time_source_last_event);

        tl_set_opt(m_tl, tl_set_event_op_context, this);
        tl_set_opt(m_tl, tl_set_event_dispatcher, event_dispatch);

        m_tl_gen_action = tl_create(m_manage);
        tl_set_opt(m_tl_gen_action, tl_set_event_op_context, this);
        tl_set_opt(m_tl_gen_action, tl_set_event_dispatcher, event_dispatch_gen_action);

        m_tl_gen_event = tl_create(m_manage);
        tl_set_opt(m_tl_gen_event, tl_set_event_op_context, this);
        tl_set_opt(m_tl_gen_event, tl_set_event_dispatcher, event_dispatch_gen_event);
    }

    static void event_dispatch(tl_event_t event, void * context) {
        ((DispatchTest*)context)->executeS
            << ":" << (char*)tl_event_data(event);
    }

    static void event_dispatch_gen_action(tl_event_t event, void * context) {
        event_dispatch(event, context);

        ::std::ostringstream os;
        os << "action-" << (char*)tl_event_data(event);
        ((DispatchTest*)context)->addAction(os.str().c_str());
    }

    static void event_dispatch_gen_event(tl_event_t event, void * context) {
        event_dispatch(event, context);

        ::std::ostringstream os;
        os << "event-" << (char*)tl_event_data(event);
        ((DispatchTest*)context)->addEvent(os.str().c_str(), 5, 0, 1);
    }

    void addEvent(const char * value, tl_time_span_t delay,  tl_time_span_t span, int repeatCount) {
        addEvent(m_tl, value, delay, span, repeatCount);
    }

    void addEvent(tl_t tl, const char * value, tl_time_span_t delay,  tl_time_span_t span, int repeatCount) {
        tl_event_t event = tl_event_create(tl, strlen(value) + 1);
        memcpy(tl_event_data(event), value, tl_event_capacity(event));
        EXPECT_EQ(0, tl_event_send_ex(event, delay, span, repeatCount));
    }

    int addAction(const char * value) {
        return addAction(m_tl, value);
    }

    int addAction(tl_t tl, const char * value) {
        tl_event_t event = tl_action_add(tl);
        if (event == NULL) return -1;
        EXPECT_LT(strlen(value) + 1, tl_event_capacity(event));
        memcpy(tl_event_data(event), value, strlen(value) + 1);
        return 0;
    }

    tl_t m_tl_gen_action;
    tl_t m_tl_gen_event;
    ::std::stringstream executeS;
};

TEST_F(DispatchTest, empty) {
    EXPECT_EQ(
        0,
        tl_manage_tick(m_manage, 10));
}

TEST_F(DispatchTest, action_basic) {
    addAction("a");
    EXPECT_EQ(1, tl_manage_tick(m_manage, 10));
    EXPECT_STREQ(":a", executeS.str().c_str());
}

TEST_F(DispatchTest, action_add_overflow) {
    for(int i = 0; i < CPE_TL_ACTION_MAX - 1; ++i) {
        EXPECT_EQ(0, addAction("a")) << "add action " << i << " fail";
    }

    EXPECT_EQ(-1, addAction("a"));
}

TEST_F(DispatchTest, action_add_overflow_not_from_begin) {
    addAction("a");
    addAction("a");
    addAction("a");
    EXPECT_EQ(3, tl_manage_tick(m_manage, 10));

    for(int i = 0; i < CPE_TL_ACTION_MAX - 1; ++i) {
        EXPECT_EQ(0, addAction("a")) << "add action " << i << " fail";
    }

    EXPECT_EQ(-1, addAction("a"));
}

TEST_F(DispatchTest, action_order) {
    addAction("a");
    addAction("b");
    addAction("c");

    EXPECT_EQ(3, tl_manage_tick(m_manage, 10));
    EXPECT_STREQ(":a:b:c", executeS.str().c_str());
}

TEST_F(DispatchTest, action_execute_count) {
    addAction("a");
    addAction("b");
    addAction("c");

    EXPECT_EQ(2, tl_manage_tick(m_manage, 2));
    EXPECT_STREQ(":a:b", executeS.str().c_str());
}

TEST_F(DispatchTest, action_gen_action) {
    addAction(m_tl_gen_action, "a");
    addAction(m_tl_gen_action, "b");
    addAction(m_tl_gen_action, "c");

    EXPECT_EQ(6, tl_manage_tick(m_manage, 10));
    EXPECT_STREQ(":a:b:c:action-a:action-b:action-c", executeS.str().c_str());
}

TEST_F(DispatchTest, action_gen_event) {
    addAction(m_tl_gen_event, "a");
    addAction(m_tl_gen_event, "b");
    addAction(m_tl_gen_event, "c");

    EXPECT_EQ(3, tl_manage_tick(m_manage, 10));
    EXPECT_STREQ(":a:b:c", executeS.str().c_str());

    EXPECT_EQ(3, tl_manage_tick(m_manage, 10));
    EXPECT_STREQ(":a:b:c:event-a:event-b:event-c", executeS.str().c_str());
}

TEST_F(DispatchTest, event_basic) {
    addEvent("a", 0, 0, 1);
    EXPECT_EQ(1, tl_manage_tick(m_manage, 10));
    EXPECT_STREQ(":a", executeS.str().c_str());
}

TEST_F(DispatchTest, event_order_same_time) {
    addEvent("a", 0, 0, 1);
    addEvent("b", 0, 0, 1);
    addEvent("c", 0, 0, 1);

    EXPECT_EQ(3, tl_manage_tick(m_manage, 10));
    EXPECT_STREQ(":a:b:c", executeS.str().c_str());
}

TEST_F(DispatchTest, event_order_by_time) {
    addEvent("c", 2, 0, 1);
    addEvent("b", 1, 0, 1);
    addEvent("a", 0, 0, 1);

    EXPECT_EQ(3, tl_manage_tick(m_manage, 10));
    EXPECT_STREQ(":a:b:c", executeS.str().c_str());
}

TEST_F(DispatchTest, event_gen_action) {
    addEvent(m_tl_gen_action, "a", 0, 0, 1);
    addEvent(m_tl_gen_action, "b", 0, 0, 1);
    addEvent(m_tl_gen_action, "c", 0, 0, 1);

    EXPECT_EQ(6, tl_manage_tick(m_manage, 10));
    EXPECT_STREQ(":a:action-a:b:action-b:c:action-c", executeS.str().c_str());
}

TEST_F(DispatchTest, event_gen_event) {
    addEvent(m_tl_gen_event, "a", 0, 0, 1);
    addEvent(m_tl_gen_event, "b", 5, 0, 1);
    addEvent(m_tl_gen_event, "c", 6, 0, 1);

    EXPECT_EQ(3, tl_manage_tick(m_manage, 10));
    EXPECT_STREQ(":a:b:c", executeS.str().c_str());

    EXPECT_EQ(3, tl_manage_tick(m_manage, 10));
    EXPECT_STREQ(":a:b:c:event-a:event-b:event-c", executeS.str().c_str());
}
