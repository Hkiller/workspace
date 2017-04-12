#include "usf/logic/logic_require.h"
#include "RunTest.hpp"

class ContextRunConditionTest : public RunTest {
};

TEST_F(ContextRunConditionTest, condition_check_null) {
    expect_return(installOp("Op1"), logic_op_exec_result_null);
    installOp("Op2");
    installOp("Op3");

    expect_commit();
    execute(
        "condition:\n"
        "  if: Op1\n"
        "  do: Op2\n"
        "  else: Op3\n"
        );

    EXPECT_EQ(logic_context_state_error, state());
}

TEST_F(ContextRunConditionTest, condition_do_true) {
    expect_return(installOp("Op1"), logic_op_exec_result_true);
    expect_return(installOp("Op2"), logic_op_exec_result_true);
    installOp("Op3");

    expect_commit();
    execute(
        "condition:\n"
        "  if: Op1\n"
        "  do: Op2\n"
        "  else: Op3\n"
        );

    EXPECT_EQ(logic_context_state_done, state());
}

TEST_F(ContextRunConditionTest, condition_do_false) {
    expect_return(installOp("Op1"), logic_op_exec_result_true);
    expect_return(installOp("Op2"), logic_op_exec_result_false);
    installOp("Op3");

    expect_commit();
    execute(
        "condition:\n"
        "  if: Op1\n"
        "  do: Op2\n"
        "  else: Op3\n"
        );

    EXPECT_EQ(logic_context_state_done, state());
}

TEST_F(ContextRunConditionTest, condition_do_null) {
    expect_return(installOp("Op1"), logic_op_exec_result_true);
    expect_return(installOp("Op2"), logic_op_exec_result_null);
    installOp("Op3");

    expect_commit();
    execute(
        "condition:\n"
        "  if: Op1\n"
        "  do: Op2\n"
        "  else: Op3\n"
        );

    EXPECT_EQ(logic_context_state_error, state());
}

TEST_F(ContextRunConditionTest, condition_else_true) {
    expect_return(installOp("Op1"), logic_op_exec_result_false);
    installOp("Op2");
    expect_return(installOp("Op3"), logic_op_exec_result_true);

    expect_commit();
    execute(
        "condition:\n"
        "  if: Op1\n"
        "  do: Op2\n"
        "  else: Op3\n"
        );

    EXPECT_EQ(logic_context_state_done, state());
}

TEST_F(ContextRunConditionTest, condition_else_false) {
    expect_return(installOp("Op1"), logic_op_exec_result_false);
    installOp("Op2");
    expect_return(installOp("Op3"), logic_op_exec_result_false);

    expect_commit();
    execute(
        "condition:\n"
        "  if: Op1\n"
        "  do: Op2\n"
        "  else: Op3\n"
        );

    EXPECT_EQ(logic_context_state_done, state());
}

TEST_F(ContextRunConditionTest, condition_else_null) {
    expect_return(installOp("Op1"), logic_op_exec_result_false);
    installOp("Op2");
    expect_return(installOp("Op3"), logic_op_exec_result_null);

    expect_commit();
    execute(
        "condition:\n"
        "  if: Op1\n"
        "  do: Op2\n"
        "  else: Op3\n"
        );

    EXPECT_EQ(logic_context_state_error, state());
}
