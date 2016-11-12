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

    const char * expect =
        "---\n"
        "m_s:\n"
        "    a1: !int16 12\n"
        "a2: !int16 14\n"
        "...\n";

    EXPECT_EQ(strlen(expect), print(&input, sizeof(input), "S2"));
    EXPECT_STREQ(expect, result());
}

