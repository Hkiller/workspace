#include "usf/logic/logic_require.h"
#include "RunTest.hpp"

class ContextRunSequenceTest : public RunTest {
};

TEST_F(ContextRunSequenceTest, basic) {
    LogicOpMock & op1 = installOp("Op1");
    expect_return(op1, logic_op_exec_result_true);

    expect_commit();
    execute("- Op1");

    EXPECT_EQ(logic_context_state_done, state());
    EXPECT_EQ((int32_t)0, rv());
}

TEST_F(ContextRunSequenceTest, multi) {
    LogicOpMock & op1 = installOp("Op1");
    LogicOpMock & op2 = installOp("Op2");

    do {
        ::testing::InSequence s;
        expect_return(op1, logic_op_exec_result_true);
        expect_return(op2, logic_op_exec_result_true);
    } while(0);

    expect_commit();
    execute(
        "- Op1\n"
        "- Op2"
        );

    EXPECT_EQ(logic_context_state_done, state());
    EXPECT_EQ((int32_t)0, rv());
}

TEST_F(ContextRunSequenceTest, multi_error_break) {
    LogicOpMock & op1 = installOp("Op1");
    expect_return(op1, logic_op_exec_result_false);
    installOp("Op2");

    expect_commit();
    execute(
        "- Op1\n"
        "- Op2"
        );

    EXPECT_EQ(logic_context_state_done, state());
    EXPECT_EQ((int32_t)0, rv());
}

TEST_F(ContextRunSequenceTest, multi_level_error_break) {
    LogicOpMock & op1 = installOp("Op1");
    installOp("Op2");
    installOp("Op3");
    installOp("Op4");

    expect_return(op1, logic_op_exec_result_false);

    expect_commit();
    execute(
        "-\n"
        "  -\n"
        "    - Op1\n"
        "    - Op2\n"
        "  - Op3\n"
        "- Op4"
        );

    EXPECT_EQ(logic_context_state_done, state());
    EXPECT_EQ((int32_t)0, rv());
}

TEST_F(ContextRunSequenceTest, multi_level_null_break) {
    LogicOpMock & op1 = installOp("Op1");
    installOp("Op2");
    installOp("Op3");
    installOp("Op4");

    expect_return(op1, logic_op_exec_result_null);

    expect_commit();
    execute(
        "-\n"
        "  -\n"
        "    - Op1\n"
        "    - Op2\n"
        "  - Op3\n"
        "- Op4"
        );

    EXPECT_EQ(logic_context_state_error, state());
    EXPECT_EQ((int32_t)-1, rv());
}

