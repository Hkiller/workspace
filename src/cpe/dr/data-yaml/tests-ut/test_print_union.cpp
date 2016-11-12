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

    const char * expect =
        "---\n"
        "m_s:\n"
        "    a1: !int16 13\n"
        "    a2: 13\n"
        "a2: !int16 14\n"
        "...\n";
        
    EXPECT_EQ(strlen(expect), print(&input, sizeof(input), "S2"));
    EXPECT_STREQ(expect, result());
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

    const char * expect =
        "---\n"
        "type: !int16 1\n"
        "m_s:\n"
        "    a1: !int16 13\n"
        "a2: !int16 14\n"
        "...\n";
    
    EXPECT_EQ(strlen(expect), print(&input, sizeof(input), "S2"));
    EXPECT_STREQ(expect, result());
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

    const char * expect = 
        "---\n"
        "type:\n"
        "    v: !int16 1\n"
        "m_s:\n"
        "    a1: !int16 13\n"
        "a2: !int16 14\n"
        "...\n";
    
    EXPECT_EQ(strlen(expect), print(&input, sizeof(input), "S2"));
    EXPECT_STREQ(expect, result());
}
