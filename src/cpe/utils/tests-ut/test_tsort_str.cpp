#include "../buffer_private.h"
#include "TSortStrTest.hpp"

TEST_F(TSortStrTest, empty) {
    EXPECT_STREQ(
        "",
        sort());
}

TEST_F(TSortStrTest, basic) {
    addDepend("A", "B");

    EXPECT_STREQ(
        "B:A:",
        sort());
}

TEST_F(TSortStrTest, cir_cle) {
    addDepend("A", "C");
    addDepend("A", "B");
    addDepend("B", "A");

    const char * r = NULL;
    EXPECT_NE(0, sort(r));
    EXPECT_STREQ("C:", r);
}
