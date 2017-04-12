#include "cpe/dr/dr_metalib_manage.h"
#include "AsyncOpTest.hpp"

TEST_F(AsyncOpTest, send_fail) {
    expect_send_return(logic_op_exec_result_false);
    execute();

    ASSERT_EQ(logic_context_state_done, state());
    ASSERT_EQ((int32_t)0, rv());
}

TEST_F(AsyncOpTest, send_null) {
    expect_send_return(logic_op_exec_result_null);
    execute();

    ASSERT_EQ(logic_context_state_error, state());
    ASSERT_EQ((int32_t)-1, rv());
}
