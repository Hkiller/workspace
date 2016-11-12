#include "CompaireTest.hpp"

TEST_F(CompaireTest, basic) {
    EXPECT_EQ(0, compaire("1", "1"));
}

TEST_F(CompaireTest, struct_eq_basic) {
    EXPECT_EQ(0, compaire("a: 1", "a: 1"));
}

TEST_F(CompaireTest, struct_eq_multi) {
    EXPECT_EQ(
        0,
        compaire(
            "a: 1\n"
            "b: 2",
            "a: 1\n"
            "b: 2"));
}

TEST_F(CompaireTest, struct_eq_multi_level) {
    EXPECT_EQ(
        0,
        compaire(
            "a:\n"
            "  b: 2",
            "a:\n"
            "  b: 2"));
}

TEST_F(CompaireTest, struct_value_lt) {
    EXPECT_LT(compaire("a: 1", "a: 2"), 0);
}

TEST_F(CompaireTest, struct_name_lt) {
    EXPECT_GT(compaire("a: 1", "b: 1"), 0);
}

TEST_F(CompaireTest, seq_eq_basic) {
    EXPECT_EQ(
        0,
        compaire(
            "a: [1, 2]"
            , "a: [1, 2]"));
}

TEST_F(CompaireTest, seq_eq_null) {
    EXPECT_EQ(
        0,
        compaire(
            "a: []"
            , "a: []"));
}

TEST_F(CompaireTest, seq_struct_eq_basic) {
    EXPECT_EQ(
        0,
        compaire(
            "a: [a: 1, b: 2]"
            , "a: [a: 1, b: 2]"));
}

TEST_F(CompaireTest, struct_eq_policy_l_leak) {
    EXPECT_EQ(
        0,
        compaire(
            "a: 1\n"
            ,
            "a: 1\n"
            "b: 2",
            CFG_CMP_POLICY_L_STRUCT_LEAK));

    EXPECT_GT(
        compaire(
            "a: 1\n"
            "b: 2"
            ,
            "a: 1",
            CFG_CMP_POLICY_L_STRUCT_LEAK),
        0);
}

TEST_F(CompaireTest, struct_eq_policy_r_leak) {
    EXPECT_EQ(
        0,
        compaire(
            "a: 1\n"
            "b: 2"
            ,
            "a: 1",
            CFG_CMP_POLICY_R_STRUCT_LEAK));

    EXPECT_LT(
        compaire(
            "a: 1\n"
            ,
            "a: 1\n"
            "b: 2"
            ,
            CFG_CMP_POLICY_R_STRUCT_LEAK),
        0);
}
