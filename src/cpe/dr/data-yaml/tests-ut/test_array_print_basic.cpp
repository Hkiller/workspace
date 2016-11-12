#include "PrintTest.hpp"

TEST_F(PrintTest, array_print_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='count' type='int16'/>"
        "	     <entry name='data' type='int16' count='16' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    struct {
        int16_t count;
        int16_t data[16];
    } input[] = {
        { 2, {12, 13, 14}  }
        , { 3, {12, 13, 14}  }
    };

    const char * expect =
        "---\n"
        "-   count: !int16 2\n"
        "    data:\n"
        "    - !int16 12\n"
        "    - !int16 13\n"
        "-   count: !int16 3\n"
        "    data:\n"
        "    - !int16 12\n"
        "    - !int16 13\n"
        "    - !int16 14\n"
        "...\n";
    
    EXPECT_EQ(strlen(expect), print_array(&input, sizeof(input), "S2"));
    EXPECT_STREQ(expect, result());
}
