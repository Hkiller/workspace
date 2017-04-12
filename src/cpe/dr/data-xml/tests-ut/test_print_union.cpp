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

    EXPECT_EQ(93, print(&input, sizeof(input), "S2"));
    EXPECT_STREQ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<S2><m_s><a1>13</a1><a2>13</a2></m_s><a2>14</a2></S2>\n", result());
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

    EXPECT_GT(print(&input, sizeof(input), "S2"), 0);
    EXPECT_STREQ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<S2><type>1</type><m_s><a1>13</a1></m_s><a2>14</a2></S2>\n", result());
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
    EXPECT_GT(print(&input, sizeof(input), "S2"), 0);
    EXPECT_STREQ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                 "<S2><type><v>1</v></type><m_s><a1>13</a1></m_s><a2>14</a2></S2>\n", result());
}
