#include "cpe/dr/dr_metalib_manage.h"
#include "ParseTest.hpp"

TEST_F(ParseTest, type_union_no_select_use_large) {
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
    struct T {
        union {
            int16_t a1;
            int32_t a2;
        } m_s;
        int16_t a2;
    };
#pragma pack(pop)

    ASSERT_EQ(
        metaSize("S2"),
        read("<Data><m_s><a2>12</a2></m_s><a2>14</a2></Data>", "S2"));

    struct T * r = (struct T*)result();
    ASSERT_TRUE(r);
    EXPECT_EQ(12, r->m_s.a1);
    EXPECT_EQ(14, r->a2);
}

TEST_F(ParseTest, type_union_no_select_use_small) {
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
    struct T {
        union {
            int16_t a1;
            int32_t a2;
        } m_s;
        int16_t a2;
    };
#pragma pack(pop)

    EXPECT_EQ(
        metaSize("S2"),
        read("<Data><m_s><a1>12</a1></m_s><a2>14</a2></Data>", "S2"));

    struct T * r = (struct T*)result();
    ASSERT_TRUE(r);
    EXPECT_EQ(12, r->m_s.a1);
    EXPECT_EQ(14, r->a2);
}

TEST_F(ParseTest, type_union_no_select_multi_entry) {
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
    } expect = { { 13 }, 14  };
#pragma pack(pop)

    ASSERT_EQ(
        metaSize("S2"),
        read("<Data><m_s><a1>12</a1><a2>13</a2></m_s><a2>14</a2></Data>", "S2"));

    ASSERT_XML_READ_RESULT(expect);
}

TEST_F(ParseTest, type_union_root) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <union name='S' version='1'>"
        "	     <entry name='a1' type='int16'/>"
        "	     <entry name='a2' type='int32'/>"
        "    </union>"
        "</metalib>"
        );

#pragma pack(push,1)
    union {
        int16_t a1;
        int32_t a2;
    } expect = { 12 };
#pragma pack(pop)

    ASSERT_EQ(metaSize("S"), read("<Data><a2>12</a2></Data>", "S"));

    ASSERT_XML_READ_RESULT(expect);
}

TEST_F(ParseTest, type_union_selector_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <union name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16' id='3'/>"
        "	     <entry name='a2' type='int32' id='4'/>"
        "    </union>"
        "    <struct name='S2' version='1' align='1'>"
        "	     <entry name='s' type='int16'/>"
        "	     <entry name='u' type='S' select='s'/>"
        "    </struct>"
        "</metalib>"
        );

#pragma pack(push,1)
    struct T {
        int16_t s;
        union {
            int16_t a1;
            int32_t a2;
        } u;
    };
#pragma pack(pop)

    ASSERT_EQ(
        sizeof(T),
        read("<Data><s>3</s><u><a1>12</a1><a2>13</a2></u></Data>", "S2"));

    struct T * r = (struct T*)result();
    ASSERT_TRUE(r);
    EXPECT_EQ(3, r->s);
    EXPECT_EQ(12, r->u.a1);

}

TEST_F(ParseTest, type_union_selector_in_sub) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='Select' version='1' align='1'>"
        "	     <entry name='v' type='int16' id='3'/>"
        "    </struct>"
        "    <union name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16' id='3'/>"
        "	     <entry name='a2' type='int32' id='4'/>"
        "    </union>"
        "    <struct name='S2' version='1' align='1'>"
        "	     <entry name='s' type='Select'/>"
        "	     <entry name='u' type='S' select='s.v'/>"
        "    </struct>"
        "</metalib>"
        );

#pragma pack(push,1)
    struct T {
        int16_t s;
        union {
            int16_t a1;
            int32_t a2;
        } u;
    };
#pragma pack(pop)

    ASSERT_EQ(
        sizeof(T),
        read("<Data><s><v>3</v></s><u><a1>12</a1><a2>13</a2></u></Data>", "S2"));

    struct T * r = (struct T*)result();
    ASSERT_TRUE(r);
    EXPECT_EQ(3, r->s);
    EXPECT_EQ(12, r->u.a1);

}

TEST_F(ParseTest, type_union_array_in_union) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='SS' version='1' align='1'>"
        "	     <entry name='count' type='int16'/>"
        "	     <entry name='d' type='int32' count='5' refer='count'/>"
        "    </struct>"
        "    <union name='S' version='1' align='1'>"
        "	     <entry name='a1' type='int16' id='3'/>"
        "	     <entry name='a2' type='SS' id='4'/>"
        "    </union>"
        "    <struct name='S2' version='1' align='1'>"
        "	     <entry name='s' type='int32'/>"
        "	     <entry name='u' type='S' select='s'/>"
        "    </struct>"
        "</metalib>"
        );

#pragma pack(push,1)
    struct T {
        int32_t s;
        union {
            int16_t a1;
            struct {
                int16_t count;
                int32_t d[5];
            } a2;
        } u;
    };
#pragma pack(pop)
    t_em_set_print();
    ASSERT_EQ(
        sizeof(T),
        read("<Data><s>4</s><u><a1>1</a1><a2><count>2</count><d>5</d><d>9</d></a2></u></Data>", "S2"));

    struct T * r = (struct T*)result();
    ASSERT_TRUE(r);
    EXPECT_EQ(4, r->s);
    EXPECT_EQ((int16_t)2, r->u.a2.count);
    EXPECT_EQ((int32_t)5, r->u.a2.d[0]);
    EXPECT_EQ((int32_t)9, r->u.a2.d[1]);
}

