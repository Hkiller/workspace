#include "usf/logic/logic_require.h"
#include "RunTest.hpp"

RunTest::RunTest()
    : m_context(0)
    , m_executor(0)
{
}

void RunTest::SetUp() {
    LogicTest::SetUp();
    m_context = t_logic_context_create();
    set_commit(m_context, m_commitMock);
}

void RunTest::TearDown() {
    if (m_executor) {
        logic_executor_free(m_executor);
        m_executor = NULL;
    }

    LogicTest::TearDown();
}

static void create_rquire(logic_stack_node_t stack) {
    logic_require_create(stack, "r1");
}

void RunTest::expect_create_require(LogicOpMock & op, logic_op_exec_result_t rv) {
    EXPECT_CALL(op, execute(::testing::_))
        .WillOnce(
            ::testing::DoAll(
                ::testing::Invoke(create_rquire),
                ::testing::Return(rv)));
}

void RunTest::expect_return(LogicOpMock & op, logic_op_exec_result_t rv) {
    EXPECT_CALL(op, execute(::testing::_))
        .WillOnce(::testing::Return(rv));
}

void RunTest::execute(const char * data) {
    m_executor = t_logic_executor_build(data);
    ASSERT_TRUE(m_executor);
    t_logic_execute(m_context, m_executor);
}

void RunTest::execute_again(void) {
    logic_context_execute(m_context);
}

void RunTest::expect_commit(void) {
    EXPECT_CALL(m_commitMock, commit(::testing::_));
}

void RunTest::set_execute_immediately(void) {
    logic_context_flag_enable(m_context, logic_context_flag_execute_immediately);
}

logic_context_state_t
RunTest::state(void) {
    return logic_context_state(m_context);
}

int32_t RunTest::rv(void) {
    return logic_context_errno(m_context);
}

void RunTest::cancel(void) {
    logic_context_cancel(m_context);
}

void RunTest::timeout(void) {
    logic_context_timeout(m_context);
}
