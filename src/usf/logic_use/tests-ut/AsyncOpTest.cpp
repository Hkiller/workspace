#include "usf/logic_use/logic_op_async.h"
#include "usf/logic/logic_executor_type.h"
#include "AsyncOpTest.hpp"

static logic_op_exec_result_t
logic_op_send_adp(logic_context_t ctx, logic_stack_node_t stack_noe, void * user_data, cfg_t cfg) {
    AsyncOpTest * test = (AsyncOpTest*)user_data;
    return test->m_opMock.send(ctx);
}

static logic_op_exec_result_t
logic_op_recv_adp(logic_context_t ctx, logic_stack_node_t stack_noe, logic_require_t require, void * user_data, cfg_t cfg) {
    AsyncOpTest * test = (AsyncOpTest*)user_data;
    return test->m_opMock.recv(ctx);
}

AsyncOpTest::AsyncOpTest()
    : m_type(NULL)
    , m_executor(NULL)
    , m_context(NULL)
{
}

void AsyncOpTest::SetUp() {
    Base::SetUp();
    t_logic_executor_type_group();
    m_type = logic_op_async_type_create(t_app(), NULL, "test_op", logic_op_send_adp, logic_op_recv_adp, this, NULL, t_em());
    ASSERT_TRUE(m_type);

    m_executor = t_logic_executor_action_create("test_op");

    m_context = t_logic_context_create();
}

void AsyncOpTest::TearDown() {
    if (m_executor) {
        logic_executor_free(m_executor);
        m_executor = NULL;
    }

    if (m_type) {
        logic_executor_type_free(m_type);
        m_type = NULL;
    }

    Base::TearDown();
}

void AsyncOpTest::execute(void) {
    t_logic_execute(m_context, m_executor);
}

void AsyncOpTest::expect_send_return(logic_op_exec_result_t rv) {
    EXPECT_CALL(m_opMock, send(::testing::_))
        .WillOnce(::testing::Return(rv));
}

void AsyncOpTest::expect_recv_return(logic_op_exec_result_t rv) {
    EXPECT_CALL(m_opMock, recv(::testing::_))
        .WillOnce(::testing::Return(rv));
}

logic_context_state_t
AsyncOpTest::state(void) {
    return logic_context_state(m_context);
}

int32_t AsyncOpTest::rv(void) {
    return logic_context_errno(m_context);
}

