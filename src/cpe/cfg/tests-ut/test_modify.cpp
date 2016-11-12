#include "ModifyTest.hpp"

TEST_F(ModifyTest, set_value_basic) {
    t_em_set_print();

    EXPECT_STREQ(
        "{ a=13 }"
        ,
        modify("a: 12", "set: { a: 13 }"));
}

TEST_F(ModifyTest, with_basic) {
    t_em_set_print();

    EXPECT_STREQ(
        "{ p1={ p2={ a=13 } } }"
        ,
        modify(
            "p1: { p2: { a: 12 } }",
            "{ with: p1.p2, set: { a: 13 } }"));
}

TEST_F(ModifyTest, set_value_seq) {
    t_em_set_print();

    EXPECT_STREQ(
        "{ a=[ 1\n"
        "    , 3\n"
        "    ] }"
        ,
        modify("a: 12", "set: { a: [ 1, 3 ] }"));
}

