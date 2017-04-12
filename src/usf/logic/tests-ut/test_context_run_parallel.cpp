#include "usf/logic/logic_require.h"
#include "RunTest.hpp"

class ContextRunParallelTest : public RunTest {
};


TEST_F(ContextRunParallelTest, success_on_all_t_t) {
    LogicOpMock & op1 = installOp("Op1");
    expect_return(op1, logic_op_exec_result_true);

    LogicOpMock & op2 = installOp("Op2");
    expect_return(op2, logic_op_exec_result_true);

    expect_commit();
    execute(
        "parallel:\n"
        "  policy: SUCCESS_ON_ALL\n"
        "  childs:\n"
        "    - Op1\n"
        "    - Op2\n"
        );

    EXPECT_EQ(logic_context_state_done, state());
}

TEST_F(ContextRunParallelTest, success_on_all_t_f) {
    LogicOpMock & op1 = installOp("Op1");
    expect_return(op1, logic_op_exec_result_true);

    LogicOpMock & op2 = installOp("Op2");
    expect_return(op2, logic_op_exec_result_false);

    expect_commit();
    execute(
        "parallel:\n"
        "  policy: SUCCESS_ON_ALL\n"
        "  childs:\n"
        "    - Op1\n"
        "    - Op2\n"
        );

    EXPECT_EQ(logic_context_state_done, state());
}

TEST_F(ContextRunParallelTest, success_on_all_f_t) {
    LogicOpMock & op1 = installOp("Op1");
    expect_return(op1, logic_op_exec_result_false);

    LogicOpMock & op2 = installOp("Op2");
    expect_return(op2, logic_op_exec_result_true);

    expect_commit();
    execute(
        "parallel:\n"
        "  policy: SUCCESS_ON_ALL\n"
        "  childs:\n"
        "    - Op1\n"
        "    - Op2\n"
        );

    EXPECT_EQ(logic_context_state_done, state());
}

TEST_F(ContextRunParallelTest, success_on_all_f_f) {
    LogicOpMock & op1 = installOp("Op1");
    expect_return(op1, logic_op_exec_result_false);

    LogicOpMock & op2 = installOp("Op2");
    expect_return(op2, logic_op_exec_result_false);

    expect_commit();
    execute(
        "parallel:\n"
        "  policy: SUCCESS_ON_ALL\n"
        "  childs:\n"
        "    - Op1\n"
        "    - Op2\n"
        );

    EXPECT_EQ(logic_context_state_done, state());
}

TEST_F(ContextRunParallelTest, success_on_all_t_null) {
    LogicOpMock & op1 = installOp("Op1");
    expect_return(op1, logic_op_exec_result_true);

    LogicOpMock & op2 = installOp("Op2");
    expect_return(op2, logic_op_exec_result_null);

    expect_commit();
    execute(
        "parallel:\n"
        "  policy: SUCCESS_ON_ALL\n"
        "  childs:\n"
        "    - Op1\n"
        "    - Op2\n"
        );

    EXPECT_EQ(logic_context_state_error, state());
}

TEST_F(ContextRunParallelTest, success_on_all_empty) {
    expect_commit();
    execute(
        "parallel:\n"
        "  policy: SUCCESS_ON_ALL\n"
        );

    EXPECT_EQ(logic_context_state_done, state());
}


TEST_F(ContextRunParallelTest, success_on_one_t_t) {
    LogicOpMock & op1 = installOp("Op1");
    expect_return(op1, logic_op_exec_result_true);

    LogicOpMock & op2 = installOp("Op2");
    expect_return(op2, logic_op_exec_result_true);

    expect_commit();
    execute(
        "parallel:\n"
        "  policy: SUCCESS_ON_ONE\n"
        "  childs:\n"
        "    - Op1\n"
        "    - Op2\n"
        );

    EXPECT_EQ(logic_context_state_done, state());
}

TEST_F(ContextRunParallelTest, success_on_one_t_f) {
    LogicOpMock & op1 = installOp("Op1");
    expect_return(op1, logic_op_exec_result_true);

    LogicOpMock & op2 = installOp("Op2");
    expect_return(op2, logic_op_exec_result_false);

    expect_commit();
    execute(
        "parallel:\n"
        "  policy: SUCCESS_ON_ONE\n"
        "  childs:\n"
        "    - Op1\n"
        "    - Op2\n"
        );

    EXPECT_EQ(logic_context_state_done, state());
}

TEST_F(ContextRunParallelTest, success_on_one_f_t) {
    LogicOpMock & op1 = installOp("Op1");
    expect_return(op1, logic_op_exec_result_false);

    LogicOpMock & op2 = installOp("Op2");
    expect_return(op2, logic_op_exec_result_true);

    expect_commit();
    execute(
        "parallel:\n"
        "  policy: SUCCESS_ON_ONE\n"
        "  childs:\n"
        "    - Op1\n"
        "    - Op2\n"
        );

    EXPECT_EQ(logic_context_state_done, state());
}

TEST_F(ContextRunParallelTest, success_on_one_f_f) {
    LogicOpMock & op1 = installOp("Op1");
    expect_return(op1, logic_op_exec_result_false);

    LogicOpMock & op2 = installOp("Op2");
    expect_return(op2, logic_op_exec_result_false);

    expect_commit();
    execute(
        "parallel:\n"
        "  policy: SUCCESS_ON_ONE\n"
        "  childs:\n"
        "    - Op1\n"
        "    - Op2\n"
        );

    EXPECT_EQ(logic_context_state_done, state());
}

TEST_F(ContextRunParallelTest, success_on_one_empty) {
    expect_commit();

    execute(
        "parallel:\n"
        "  policy: SUCCESS_ON_ONE\n"
        );

    EXPECT_EQ(logic_context_state_done, state());
}

