#include "PrintTest.hpp"

TEST_F(PrintTest, array_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='count' type='int16'/>"
        "	     <entry name='data' type='int16' count='16' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

#pragma pack(push,1)
    struct {
        int16_t count;
        int16_t data[16];
    } input = { 2, {12, 13, 14}  };
#pragma pack(pop)

    EXPECT_GT(print(&input, sizeof(input), "S2"), 0);
    EXPECT_STREQ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<S2><count>2</count><data>12</data><data>13</data></S2>\n", result());
}

TEST_F(PrintTest, array_struct_basic) {
    t_em_set_print();

    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='a1' type='int16'/>"
        "    </struct>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='count' type='int16'/>"
        "	     <entry name='data' type='S' count='16' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

#pragma pack(push,1)
    struct {
        int16_t count;
        struct {
            int16_t a1;
        } data[16];
    } input = { 2, { {12}, {13}, {14} }  };
#pragma pack(pop)

    EXPECT_GT(print(&input, sizeof(input), "S2"), 0);
    EXPECT_STREQ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<S2><count>2</count><data><a1>12</a1></data><data><a1>13</a1></data></S2>\n", result());
}
