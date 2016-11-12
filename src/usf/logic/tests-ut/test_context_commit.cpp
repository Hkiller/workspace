#include "LogicTest.hpp"

class ContextCommitTest : public LogicTest {
public:
};

TEST_F(ContextCommitTest, commit_on_done) {
    LogicOpMock & op1 = installOp("Op1");

    EXPECT_CALL(op1, execute(::testing::_))
        .WillOnce(::testing::Return(logic_op_exec_result_true));

    CommitMock commitMock;
    EXPECT_CALL(commitMock, commit(::testing::_));

    logic_context_t context = t_logic_context_create();
    set_commit(context, commitMock);

    logic_executor_t executor = t_logic_executor_build("Op1");

    t_logic_execute(context, executor);

    EXPECT_EQ(logic_context_state_done, logic_context_state(context));
    EXPECT_EQ((int32_t)0, logic_context_errno(context));

    logic_executor_free(executor);
}

TEST_F(ContextCommitTest, commit_on_error) {
    LogicOpMock & op1 = installOp("Op1");

    EXPECT_CALL(op1, execute(::testing::_))
        .WillOnce(::testing::Return(logic_op_exec_result_false));

    CommitMock commitMock;
    EXPECT_CALL(commitMock, commit(::testing::_));

    logic_context_t context = t_logic_context_create();
    set_commit(context, commitMock);

    logic_executor_t executor = t_logic_executor_build("Op1");

    t_logic_execute(context, executor);

    EXPECT_EQ(logic_context_state_done, logic_context_state(context));
    EXPECT_EQ((int32_t)0, logic_context_errno(context));

    logic_executor_free(executor);
}
