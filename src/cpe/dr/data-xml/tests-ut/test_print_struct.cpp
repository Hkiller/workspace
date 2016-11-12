#include "PrintTest.hpp"

TEST_F(PrintTest, print_struct) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S' version='1'>"
        "	     <entry name='a1' type='int16'/>"
        "    </struct>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='m_s' type='S'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>"
        );

#pragma pack(push,1)
    struct {
        struct {
            int16_t a1;
        } m_s;
        int16_t a2;
    } input = { { 12 }, 14  };
#pragma pack(pop)

    EXPECT_GT(print(&input, sizeof(input), "S2"), 0);
    EXPECT_STREQ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<S2><m_s><a1>12</a1></m_s><a2>14</a2></S2>\n", result());
}

