#include "usf/logic/logic_require.h"
#include "RunTest.hpp"

class RequireRunTest : public RunTest {
public:
    logic_require_t require_find() {
        return logic_require_find(t_logic_manage(), 1);
    }

    void cancel_require(void) { logic_require_cancel(require_find()); }
    void set_require_done(void) { logic_require_set_done(require_find()); }
    void set_require_error(void) { logic_require_set_error(require_find()); }
};

TEST_F(RequireRunTest, auto_commit_require_done_keep) {
    LogicOpMock & op1 = installOp("Op1");
    expect_create_require(op1, logic_op_exec_result_true);

    set_execute_immediately();

    execute("- Op1\n");

    /*first execut should block*/
    EXPECT_EQ(logic_context_state_waiting, state());

    expect_return(op1, logic_op_exec_result_true);
    expect_commit();
    set_require_done();

    EXPECT_TRUE(require_find());
}

TEST_F(RequireRunTest, auto_commit_require_error_keep) {
    LogicOpMock & op1 = installOp("Op1");

    expect_create_require(op1, logic_op_exec_result_true);

    set_execute_immediately();

    execute("- Op1\n");
    EXPECT_EQ(logic_context_state_waiting, state());

    expect_return(op1, logic_op_exec_result_true);
    expect_commit();
    set_require_error();

    EXPECT_TRUE(require_find());
}

TEST_F(RequireRunTest, auto_commit_for_free) {
    LogicOpMock & op1 = installOp("Op1");
    expect_create_require(op1, logic_op_exec_result_true);

    set_execute_immediately();

    execute("- Op1\n");
    EXPECT_EQ(logic_context_state_waiting, state());

    expect_return(op1, logic_op_exec_result_true);
    expect_commit();
    logic_require_free(logic_require_find(t_logic_manage(), 1));
}

TEST_F(RequireRunTest, auto_commit_cancel_keep) {
    LogicOpMock & op1 = installOp("Op1");

    expect_create_require(op1, logic_op_exec_result_true);

    set_execute_immediately();

    execute("- Op1\n");
    EXPECT_EQ(logic_context_state_waiting, state());

    expect_return(op1, logic_op_exec_result_true);
    expect_commit();
    cancel_require();

    EXPECT_TRUE(require_find());
}

