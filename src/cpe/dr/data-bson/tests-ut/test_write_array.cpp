#include "WriteTest.hpp"

TEST_F(WriteTest, array_struct_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='b1' type='S1' count='2' id='3'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(46, write("S2", "b1: [ { a1: 1 }, { a1: 2 } ]"));
    
    EXPECT_STREQ(
        "0x2E 0x00 0x00 0x00"   /*S2 begin*/
        " 0x04"
        " 0x62 0x31 0x00"
        " 0x25 0x00 0x00 0x00"  /*array begin*/
        " 0x03"                 /*   [0] begin*/
        " 0x30 0x00"
        " 0x0D 0x00 0x00 0x00"
        " 0x10"                 /*   [0].a1 begin*/
        " 0x61 0x31 0x00"
        " 0x01 0x00 0x00 0x00"
        " 0x00"                 /*   [0] end*/
        " 0x03"                 /*   [1] begin*/
        " 0x31 0x00"
        " 0x0D 0x00 0x00 0x00"
        " 0x10"                 /*   [1].a1 begin*/
        " 0x61 0x31 0x00"
        " 0x02 0x00 0x00 0x00"
        " 0x00"                 /*   [1] end*/
        " 0x00"                 /*array end*/
        " 0x00"                 /*S2 end*/
        , result());
}

TEST_F(WriteTest, array_struct_with_refer) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='S2' version='1'>"
        "        <entry name='count' type='int32'/>"
        "	     <entry name='b1' type='S1' count='4' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(57, write("S2", "b1: [ { a1: 1 }, { a1: 2 } ]"));
    
    EXPECT_STREQ(
        "0x39 0x00 0x00 0x00"   /*S2 begin*/
        " 0x10"                 /*count*/
        " 0x63 0x6F 0x75 0x6E 0x74 0x00"
        " 0x02 0x00 0x00 0x00"
        " 0x04"
        " 0x62 0x31 0x00"
        " 0x25 0x00 0x00 0x00"  /*array begin*/
        " 0x03"                 /*   [0] begin*/
        " 0x30 0x00"
        " 0x0D 0x00 0x00 0x00"
        " 0x10"                 /*   [0].a1 begin*/
        " 0x61 0x31 0x00"
        " 0x01 0x00 0x00 0x00"
        " 0x00"                 /*   [0] end*/
        " 0x03"                 /*   [1] begin*/
        " 0x31 0x00"
        " 0x0D 0x00 0x00 0x00"
        " 0x10"                 /*   [1].a1 begin*/
        " 0x61 0x31 0x00"
        " 0x02 0x00 0x00 0x00"
        " 0x00"                 /*   [1] end*/
        " 0x00"                 /*array end*/
        " 0x00"                 /*S2 end*/
        , result());
}

TEST_F(WriteTest, array_struct_with_refer_empty) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='S2' version='1'>"
        "        <entry name='count' type='int32'/>"
        "	     <entry name='b1' type='S1' count='4' refer='count'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(25, write("S2", "b1: [ ]"));
    
    EXPECT_STREQ(
        "0x19 0x00 0x00 0x00"   /*S2 begin*/
        " 0x10"                 /*count*/
        " 0x63 0x6F 0x75 0x6E 0x74 0x00"
        " 0x00 0x00 0x00 0x00"
        " 0x04"
        " 0x62 0x31 0x00"
        " 0x05 0x00 0x00 0x00"  /*array begin*/
        " 0x00"                 /*array end*/
        " 0x00"                 /*S2 end*/
        , result());
}

TEST_F(WriteTest, array_struct_with_refer_multi) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S1' version='1'>"
        "	     <entry name='a1' type='uint32'/>"
        "    </struct>"
        "    <struct name='S2' version='1'>"
        "        <entry name='count_1' type='int32'/>"
        "	     <entry name='b1' type='S1' count='4' refer='count_1'/>"
        "        <entry name='count_2' type='int32'/>"
        "	     <entry name='b2' type='S1' count='4' refer='count_2'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(
        129,
        write(
            "S2",
            "b1: [ { a1: 1 }, { a1: 2 } ]\n"
            "b2: [ { a1: 3 }, { a1: 4 }, { a1: 5 } ]"
            ));
    
    EXPECT_STREQ(
        "0x81 0x00 0x00 0x00"   /*S2 begin*/
        " 0x10"                 /*count_1*/
        " 0x63 0x6F 0x75 0x6E 0x74 0x5F 0x31 0x00"
        " 0x02 0x00 0x00 0x00"
        " 0x04"
        " 0x62 0x31 0x00"
        " 0x25 0x00 0x00 0x00"  /*array begin*/
        " 0x03"                 /*   [0] begin*/
        " 0x30 0x00"
        " 0x0D 0x00 0x00 0x00"
        " 0x10"                 /*   [0].a1 begin*/
        " 0x61 0x31 0x00"
        " 0x01 0x00 0x00 0x00"
        " 0x00"                 /*   [0] end*/
        " 0x03"                 /*   [1] begin*/
        " 0x31 0x00"
        " 0x0D 0x00 0x00 0x00"
        " 0x10"                 /*   [1].a1 begin*/
        " 0x61 0x31 0x00"
        " 0x02 0x00 0x00 0x00"
        " 0x00"                 /*   [1] end*/
        " 0x00"                 /*array end*/
        " 0x10"                 /*count_2*/
        " 0x63 0x6F 0x75 0x6E 0x74 0x5F 0x32 0x00"
        " 0x03 0x00 0x00 0x00"
        " 0x04"
        " 0x62 0x32 0x00"
        " 0x35 0x00 0x00 0x00"  /*array begin*/
        " 0x03"                 /*   [0] begin*/
        " 0x30 0x00"
        " 0x0D 0x00 0x00 0x00"
        " 0x10"                 /*   [0].a1 begin*/
        " 0x61 0x31 0x00"
        " 0x03 0x00 0x00 0x00"
        " 0x00"                 /*   [0] end*/
        " 0x03"                 /*   [1] begin*/
        " 0x31 0x00"
        " 0x0D 0x00 0x00 0x00"
        " 0x10"                 /*   [1].a1 begin*/
        " 0x61 0x31 0x00"
        " 0x04 0x00 0x00 0x00"
        " 0x00"                 /*   [1] end*/
        " 0x03"                 /*   [2] begin*/
        " 0x32 0x00"
        " 0x0D 0x00 0x00 0x00"
        " 0x10"                 /*   [2].a1 begin*/
        " 0x61 0x31 0x00"
        " 0x05 0x00 0x00 0x00"
        " 0x00"                 /*   [2] end*/
        " 0x00"                 /*array end*/
        " 0x00"                 /*S2 end*/
        , result());
}

TEST_F(WriteTest, array_struct_with_refer_multi_first_empty) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='S1' version='1'>"
        "	     <entry name='a1' type='uint32'/>"
        "    </struct>"
        "    <struct name='S2' version='1'>"
        "        <entry name='count_1' type='int32'/>"
        "	     <entry name='b1' type='S1' count='4' refer='count_1'/>"
        "        <entry name='count_2' type='int32'/>"
        "	     <entry name='b2' type='S1' count='4' refer='count_2'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(
        97,
        write(
            "S2",
            "b1: [ ]\n"
            "b2: [ { a1: 3 }, { a1: 4 }, { a1: 5 } ]"
            ));
    
    EXPECT_STREQ(
        "0x61 0x00 0x00 0x00"   /*S2 begin*/
        " 0x10"                 /*count_1*/
        " 0x63 0x6F 0x75 0x6E 0x74 0x5F 0x31 0x00"
        " 0x00 0x00 0x00 0x00"
        " 0x04"
        " 0x62 0x31 0x00"
        " 0x05 0x00 0x00 0x00"  /*array begin*/
        " 0x00"                 /*array end*/
        " 0x10"                 /*count_2*/
        " 0x63 0x6F 0x75 0x6E 0x74 0x5F 0x32 0x00"
        " 0x03 0x00 0x00 0x00"
        " 0x04"
        " 0x62 0x32 0x00"
        " 0x35 0x00 0x00 0x00"  /*array begin*/
        " 0x03"                 /*   [0] begin*/
        " 0x30 0x00"
        " 0x0D 0x00 0x00 0x00"
        " 0x10"                 /*   [0].a1 begin*/
        " 0x61 0x31 0x00"
        " 0x03 0x00 0x00 0x00"
        " 0x00"                 /*   [0] end*/
        " 0x03"                 /*   [1] begin*/
        " 0x31 0x00"
        " 0x0D 0x00 0x00 0x00"
        " 0x10"                 /*   [1].a1 begin*/
        " 0x61 0x31 0x00"
        " 0x04 0x00 0x00 0x00"
        " 0x00"                 /*   [1] end*/
        " 0x03"                 /*   [2] begin*/
        " 0x32 0x00"
        " 0x0D 0x00 0x00 0x00"
        " 0x10"                 /*   [2].a1 begin*/
        " 0x61 0x31 0x00"
        " 0x05 0x00 0x00 0x00"
        " 0x00"                 /*   [2] end*/
        " 0x00"                 /*array end*/
        " 0x00"                 /*S2 end*/
        , result());
}

TEST_F(WriteTest, array_union_basic) {
    installMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <union name='U1' version='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "	     <entry name='a2' type='uint32' id='2'/>"
        "    </union>"
        "    <struct name='S1' version='1'>"
        "	     <entry name='s' type='int8' id='1'/>"
        "	     <entry name='d' type='U1' id='2' select='s'/>"
        "    </struct>"
        "    <struct name='S2' version='1'>"
        "	     <entry name='b1' type='S1' count='2' id='3'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_EQ(76, write("S2", "b1: [ { d: { a1: 1 } }, { d: { a2: 2 } } ]"));
    
    EXPECT_STREQ(
        "0x4C 0x00 0x00 0x00"   /*S2 begin*/
        " 0x04"
        " 0x62 0x31 0x00"
        " 0x43 0x00 0x00 0x00"  /*array begin*/
        " 0x03"                 /*   [0] begin*/
        " 0x30 0x00"
        " 0x1C 0x00 0x00 0x00"  /*   [0] struct begin*/
        " 0x10"                 /*   [0].s*/
        " 0x73 0x00"
        " 0x01 0x00 0x00 0x00"
        " 0x03"                 /*   [0].d*/
        " 0x64 0x00"
        " 0x0D 0x00 0x00 0x00"
        " 0x10"                 /*   [0].d.a1*/
        " 0x61 0x31 0x00"
        " 0x01 0x00 0x00 0x00"
        " 0x00"                 /*   [0].d end*/
        " 0x00"                 /*   [0] end*/
        " 0x03"                 /*   [1] begin*/
        " 0x31 0x00"
        " 0x1C 0x00 0x00 0x00"  /*   [1] struct begin*/
        " 0x10"                 /*   [1].s*/
        " 0x73 0x00"
        " 0x02 0x00 0x00 0x00"
        " 0x03"                 /*   [1].d*/
        " 0x64 0x00"
        " 0x0D 0x00 0x00 0x00"
        " 0x10"                 /*   [1].d.a2*/
        " 0x61 0x32 0x00"
        " 0x02 0x00 0x00 0x00"
        " 0x00"                 /*   [1].d end*/
        " 0x00"                 /*   [1] end*/
        " 0x00"                 /*array end*/
        " 0x00"                 /*S2 end*/
        , result());
}
