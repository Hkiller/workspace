#include "LogicTest.hpp"

class ExecutorBuildTest : public LogicTest {
public:
    ExecutorBuildTest() : m_executor(NULL) {}

    virtual void TearDown() { 
        if (m_executor) logic_executor_free(m_executor);
        LogicTest::TearDown();
    }

    logic_executor_t m_executor;
};

TEST_F(ExecutorBuildTest, action_basic) {
    installOp("Op1");

    m_executor =
          t_logic_executor_build("Op1");
    ASSERT_TRUE(m_executor);

    EXPECT_STREQ(
        "Op1",
        t_logic_executor_dump(m_executor));
}

TEST_F(ExecutorBuildTest, action_with_args) {
    installOp("Op1");

    m_executor =
          t_logic_executor_build("Op1: { a1: 1, a2: 2}");
    ASSERT_TRUE(m_executor);

    EXPECT_STREQ(
        "Op1: { a1=1, a2=2 }",
        t_logic_executor_dump(m_executor));
}

TEST_F(ExecutorBuildTest, composite_sequence_basic) {
    installOp("Op1");
    installOp("Op2");

    m_executor =
        t_logic_executor_build(
            "- Op1\n"
            "- Op2: { a1: 1, a2: 2 }\n"
            );
    ASSERT_TRUE(m_executor);

    EXPECT_STREQ(
        "sequence:\n"
        "    Op1\n"
        "    Op2: { a1=1, a2=2 }"
        ,
        t_logic_executor_dump(m_executor));
}

TEST_F(ExecutorBuildTest, condition_basic) {
    installOp("Op1");
    installOp("Op2");
    installOp("Op3");

    m_executor =
        t_logic_executor_build(
            "condition:\n"
            "    if: Op1\n"
            "    do: Op2\n"
            "    else: Op3\n"
            );
    ASSERT_TRUE(m_executor);

    EXPECT_STREQ(
        "condition:\n"
        "    if:\n"
        "        Op1\n"
        "    do:\n"
        "        Op2\n"
        "    else:\n"
        "        Op3"
        ,
        t_logic_executor_dump(m_executor));
}

TEST_F(ExecutorBuildTest, condition_no_else) {
    installOp("Op1");
    installOp("Op2");

    m_executor =
        t_logic_executor_build(
            "condition:\n"
            "    if: Op1\n"
            "    do: Op2"
            );
    ASSERT_TRUE(m_executor);

    EXPECT_STREQ(
        "condition:\n"
        "    if:\n"
        "        Op1\n"
        "    do:\n"
        "        Op2"
        ,
        t_logic_executor_dump(m_executor));
}

TEST_F(ExecutorBuildTest, condition_no_if) {
    installOp("Op1");
    installOp("Op2");
    installOp("Op3");

    m_executor =
        t_logic_executor_build(
            "condition:\n"
            "    do: Op2\n"
            "    else: Op3"
            );
    EXPECT_TRUE(m_executor == NULL);
}

TEST_F(ExecutorBuildTest, condition_no_do) {
    installOp("Op1");
    installOp("Op2");
    installOp("Op3");

    m_executor =
        t_logic_executor_build(
            "condition:\n"
            "    if: Op1\n"
            "    else: Op3"
            );
    ASSERT_TRUE(m_executor == NULL);
}

TEST_F(ExecutorBuildTest, composite_sequence_sequence) {
    installOp("Op1");
    installOp("Op2");

    m_executor =
        t_logic_executor_build(
            "- Op1\n"
            "-\n"
            "    - Op2: { a1: 1, a2: 2 }\n"
            );
    ASSERT_TRUE(m_executor);

    EXPECT_STREQ(
        "sequence:\n"
        "    Op1\n"
        "    sequence:\n"
        "        Op2: { a1=1, a2=2 }"
        ,
        t_logic_executor_dump(m_executor));
}

TEST_F(ExecutorBuildTest, composite_sequence_explict) {
    installOp("Op1");
    installOp("Op2");

    m_executor =
        t_logic_executor_build(
            "- Op1\n"
            "- sequence:\n"
            "    - Op2: { a1: 1, a2: 2 }\n"
            );
    ASSERT_TRUE(m_executor);

    EXPECT_STREQ(
        "sequence:\n"
        "    Op1\n"
        "    sequence:\n"
        "        Op2: { a1=1, a2=2 }"
        ,
        t_logic_executor_dump(m_executor));
}

TEST_F(ExecutorBuildTest, composite_selector_basic) {
    installOp("Op1");
    installOp("Op2");

    m_executor =
        t_logic_executor_build(
            "- Op1\n"
            "- selector:\n"
            "    - Op2: { a1: 1, a2: 2 }\n"
            );
    ASSERT_TRUE(m_executor);

    EXPECT_STREQ(
        "sequence:\n"
        "    Op1\n"
        "    selector:\n"
        "        Op2: { a1=1, a2=2 }"
        ,
        t_logic_executor_dump(m_executor));
}

TEST_F(ExecutorBuildTest, composite_parallel_basic) {
    installOp("Op1");
    installOp("Op2");

    m_executor =
        t_logic_executor_build(
            "- Op1\n"
            "- parallel:\n"
            "    - Op2: { a1: 1, a2: 2 }\n"
            );
    ASSERT_TRUE(m_executor);

    EXPECT_STREQ(
        "sequence:\n"
        "    Op1\n"
        "    parallel:\n"
        "        policy: SUCCESS_ON_ALL\n"
        "        childs:\n"
        "            Op2: { a1=1, a2=2 }"
        ,
        t_logic_executor_dump(m_executor));
}

TEST_F(ExecutorBuildTest, composite_parallel_explict_success_on_all) {
    installOp("Op1");
    installOp("Op2");

    m_executor =
        t_logic_executor_build(
            "- Op1\n"
            "- parallel:\n"
            "    policy: SUCCESS_ON_ALL\n"
            "    childs:\n"
            "      - Op2: { a1: 1, a2: 2 }\n"
            );
    ASSERT_TRUE(m_executor);

    EXPECT_STREQ(
        "sequence:\n"
        "    Op1\n"
        "    parallel:\n"
        "        policy: SUCCESS_ON_ALL\n"
        "        childs:\n"
        "            Op2: { a1=1, a2=2 }"
        ,
        t_logic_executor_dump(m_executor));
}

TEST_F(ExecutorBuildTest, composite_parallel_explict_success_on_one) {
    installOp("Op1");
    installOp("Op2");

    m_executor =
        t_logic_executor_build(
            "- Op1\n"
            "- parallel:\n"
            "    policy: SUCCESS_ON_ONE\n"
            "    childs:\n"
            "      - Op2: { a1: 1, a2: 2 }\n"
            );
    ASSERT_TRUE(m_executor);

    EXPECT_STREQ(
        "sequence:\n"
        "    Op1\n"
        "    parallel:\n"
        "        policy: SUCCESS_ON_ONE\n"
        "        childs:\n"
        "            Op2: { a1=1, a2=2 }"
        ,
        t_logic_executor_dump(m_executor));
}

TEST_F(ExecutorBuildTest, composite_parallel_explict_no_policy) {
    installOp("Op1");
    installOp("Op2");

    m_executor =
        t_logic_executor_build(
            "- Op1\n"
            "- parallel:\n"
            "    childs:\n"
            "      - Op2: { a1: 1, a2: 2 }\n"
            );
    ASSERT_TRUE(m_executor);

    EXPECT_STREQ(
        "sequence:\n"
        "    Op1\n"
        "    parallel:\n"
        "        policy: SUCCESS_ON_ALL\n"
        "        childs:\n"
        "            Op2: { a1=1, a2=2 }"
        ,
        t_logic_executor_dump(m_executor));
}

TEST_F(ExecutorBuildTest, protect_basic) {
    installOp("Op1");

    m_executor =
        t_logic_executor_build(
            "protect: Op1");
    ASSERT_TRUE(m_executor);

    EXPECT_STREQ(
        "protect:\n"
        "    Op1",
        t_logic_executor_dump(m_executor));
}

TEST_F(ExecutorBuildTest, protect_sequence) {
    installOp("Op1");

    m_executor =
        t_logic_executor_build(
            "protect:\n"
            "    - Op1");
    ASSERT_TRUE(m_executor);

    EXPECT_STREQ(
        "protect:\n"
        "    sequence:\n"
        "        Op1"
        ,
        t_logic_executor_dump(m_executor));
}

TEST_F(ExecutorBuildTest, not_basic) {
    installOp("Op1");

    m_executor =
        t_logic_executor_build(
            "not: Op1");
    ASSERT_TRUE(m_executor);

    EXPECT_STREQ(
        "not:\n"
        "    Op1",
        t_logic_executor_dump(m_executor));
}

