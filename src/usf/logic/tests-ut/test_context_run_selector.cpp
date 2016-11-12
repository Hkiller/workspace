#include "usf/logic/logic_require.h"
#include "RunTest.hpp"

class ContextRunSelectorTest : public RunTest {
};

TEST_F(ContextRunSelectorTest, basic_true) {
    LogicOpMock & op1 = installOp("Op1");
    expect_return(op1, logic_op_exec_result_true);

    expect_commit();
    execute(
        "selector:\n"
        "  - Op1");

    EXPECT_EQ(logic_context_state_done, state());
    EXPECT_EQ((int32_t)0, rv());
}

TEST_F(ContextRunSelectorTest, basic_false) {
    LogicOpMock & op1 = installOp("Op1");
    expect_return(op1, logic_op_exec_result_false);

    expect_commit();
    execute(
        "selector:\n"
        "  - Op1");

    EXPECT_EQ(logic_context_state_done, state());
    EXPECT_EQ((int32_t)0, rv());
}

TEST_F(ContextRunSelectorTest, multi) {
    LogicOpMock & op1 = installOp("Op1");
    LogicOpMock & op2 = installOp("Op2");

    do {
        ::testing::InSequence s;
        expect_return(op1, logic_op_exec_result_false);
        expect_return(op2, logic_op_exec_result_true);
    } while(0);

    expect_commit();
    execute(
        "selector:\n"
        "  - Op1\n"
        "  - Op2"
        );

    EXPECT_EQ(logic_context_state_done, state());
    EXPECT_EQ((int32_t)0, rv());
}

TEST_F(ContextRunSelectorTest, multi_success_break) {
    LogicOpMock & op1 = installOp("Op1");
    expect_return(op1, logic_op_exec_result_true);
    installOp("Op2");

    expect_commit();
    execute(
        "selector:\n"
        "  - Op1\n"
        "  - Op2"
        );

    EXPECT_EQ(logic_context_state_done, state());
    EXPECT_EQ((int32_t)0, rv());
}

TEST_F(ContextRunSelectorTest, multi_level_success_break) {
    LogicOpMock & op1 = installOp("Op1");
    installOp("Op2");
    installOp("Op3");
    installOp("Op4");

    expect_return(op1, logic_op_exec_result_true);

    expect_commit();
    execute(
        "selector:\n"
        "  - selector:\n"
        "      - selector:\n"
        "        - Op1\n"
        "        - Op2\n"
        "      - Op3\n"
        "  - Op4"
        );

    EXPECT_EQ(logic_context_state_done, state());
    EXPECT_EQ((int32_t)0, rv());
}

