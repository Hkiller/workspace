#include "LogicTest.hpp"

class ContextBasicTest : public LogicTest {
};

TEST_F(ContextBasicTest, defaults) {
    logic_context_t context = t_logic_context_create(123);

    EXPECT_EQ((logic_context_id_t)1, logic_context_id(context));

    EXPECT_EQ((uint32_t)0, logic_context_flags(context));

    EXPECT_EQ(logic_context_state_init, logic_context_state(context));
    EXPECT_TRUE(logic_context_mgr(context) == t_logic_manage());
    EXPECT_TRUE(logic_context_app(context) == t_app());
    EXPECT_EQ((int32_t)0, logic_context_errno(context));
    EXPECT_EQ((size_t)123, logic_context_capacity(context));
    EXPECT_TRUE(logic_context_data(context));
}

TEST_F(ContextBasicTest, find) {
    EXPECT_TRUE(NULL == t_logic_context_find(1));

    logic_context_t context = t_logic_context_create();
    EXPECT_TRUE(context == t_logic_context_find(1));
}

TEST_F(ContextBasicTest, flags) {
    logic_context_t context = t_logic_context_create();

    EXPECT_TRUE(!logic_context_flag_is_enable(context, logic_context_flag_debug));

    //enable
    logic_context_flag_enable(context, logic_context_flag_debug);
    EXPECT_EQ((uint32_t)logic_context_flag_debug, logic_context_flags(context));
    EXPECT_TRUE(logic_context_flag_is_enable(context, logic_context_flag_debug));

    //disable
    logic_context_flag_disable(context, logic_context_flag_debug);
    EXPECT_EQ((uint32_t)0, logic_context_flags(context));
    EXPECT_TRUE(!logic_context_flag_is_enable(context, logic_context_flag_debug));
}

TEST_F(ContextBasicTest, auto_id_basic) {
    t_logic_context_create(0, (logic_context_id_t)0);
    t_logic_context_create(0, (logic_context_id_t)1);

    logic_context_t context = t_logic_context_create();
    EXPECT_EQ((logic_context_id_t)2, logic_context_id(context));
}

TEST_F(ContextBasicTest, id_duplicate) {
    t_logic_context_create(0, (logic_context_id_t)0);
    EXPECT_TRUE(NULL == logic_context_create(t_logic_manage(), 0, 0));
}

TEST_F(ContextBasicTest, id_auto_try_max) {
    for(int i = 0; i < 20000; ++i) {
        t_logic_context_create(0, (logic_context_id_t)i);
    }

    EXPECT_TRUE(NULL == logic_context_create(t_logic_manage(), INVALID_LOGIC_CONTEXT_ID, 0));
}

TEST_F(ContextBasicTest, state_error_by_errno) {
    logic_context_t context = t_logic_context_create();
    
    EXPECT_EQ(logic_context_state_init, logic_context_state(context));
    logic_context_errno_set(context, -1);
    EXPECT_EQ(logic_context_state_error, logic_context_state(context));
    logic_context_errno_set(context, 0);
    EXPECT_EQ(logic_context_state_init, logic_context_state(context));
}
