#include "PrintTest.hpp"

TEST_F(PrintTest, print_union_no_selector) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <union name='S' version='1'>"
        "	     <entry name='a1' type='int16'/>"
        "	     <entry name='a2' type='int32'/>"
        "    </union>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='m_s' type='S'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>"
        );

#pragma pack(push,1)
    struct {
        union {
            int16_t a1;
            int32_t a2;
        } m_s;
        int16_t a2;
    } input = { { 13 }, 14  };
#pragma pack(pop)

    EXPECT_EQ(33, print(&input, sizeof(input), "S2"));
    EXPECT_STREQ("{\"m_s\":{\"a1\":13,\"a2\":13},\"a2\":14}", result());
}

TEST_F(PrintTest, print_union_with_selector) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <union name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16' id='1'/>"
        "	     <entry name='a2' type='int32' id='2'/>"
        "    </union>"
        "    <struct name='S2' version='1' align='1'>"
        "	     <entry name='type' type='int16'/>"
        "	     <entry name='m_s' type='S' select='type'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>"
        );

#pragma pack(push,1)
    struct {
        int16_t type;
        union {
            int16_t a1;
            int32_t a2;
        } m_s;
        int16_t a2;
    } input = { 1, { 13 }, 14  };
#pragma pack(pop)

    EXPECT_EQ(34, print(&input, sizeof(input), "S2"));
    EXPECT_STREQ("{\"type\":1,\"m_s\":{\"a1\":13},\"a2\":14}", result());
}

TEST_F(PrintTest, print_union_with_selector_in_sub) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='Select' version='1' align='1'>"
        "	     <entry name='v' type='int16' id='3'/>"
        "    </struct>"
        "    <union name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16' id='1'/>"
        "	     <entry name='a2' type='int32' id='2'/>"
        "    </union>"
        "    <struct name='S2' version='1' align='1'>"
        "	     <entry name='type' type='Select'/>"
        "	     <entry name='m_s' type='S' select='type.v'/>"
        "	     <entry name='a2' type='int16'/>"
        "    </struct>"
        "</metalib>"
        );

#pragma pack(push,1)
    struct {
        int16_t type;
        union {
            int16_t a1;
            int32_t a2;
        } m_s;
        int16_t a2;
    } input = { 1, { 13 }, 14  };
#pragma pack(pop)

    t_em_set_print();
    EXPECT_EQ(40, print(&input, sizeof(input), "S2"));
    EXPECT_STREQ("{\"type\":{\"v\":1},\"m_s\":{\"a1\":13},\"a2\":14}", result());
}
