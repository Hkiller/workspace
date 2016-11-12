#include "OmGrpMetaTest.hpp" 

TEST_F(OmGrpMetaTest, from_meta_normal_basic) {
    installFromMeta(
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='AttrGroup1' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='TestObj' version='1'>"
        "        <entry name='entry1' type='AttrGroup1'/>"
        "    </struct>"
        "</metalib>"
        ,
        "TestObj"
        ) ;


    EXPECT_STREQ(
        "pom_grp_meta: name=TestObj, page-size=256, class-id=1, obj-size=4, page-count=1, size-buf-start=4, size-buf-count=0\n"
        "    entry1: entry-type=normal, data=type=AttrGroup1, page-begin=0, page-count=1, class-id=2, page-size=4, obj-align=1"
        ,
        t_pom_grp_meta_dump(m_meta));
}

// TEST_F(OmGrpMetaTest, from_meta_list_basic) {
//     install(
//         "TestObj:\n"
//         "  attributes:\n"
//         "    - entry1: { entry-type: list, data-type: AttrGroup1, group-count: 3, capacity: 3 }\n"
//         ,
//         "<metalib tagsetversion='1' name='net'  version='1'>"
//         "    <struct name='AttrGroup1' version='1' align='1'>"
//         "	     <entry name='a1' type='uint32' id='1'/>"
//         "    </struct>"
//         "</metalib>"
//         ) ;


//     EXPECT_STREQ(
//         "pom_grp_meta: name=TestObj, page-size=256, class-id=1, obj-size=6, page-count=1, size-buf-start=4, size-buf-count=1\n"
//         "    entry1: entry-type=list, data=type=AttrGroup1, capacity=3, size-idx=0, page-begin=0, page-count=1, class-id=2, page-size=12, obj-align=1"
//         ,
//         t_pom_grp_meta_dump(m_meta));
// }

// TEST_F(OmGrpMetaTest, from_meta_list_multi) {
//     install(
//         "TestObj:\n"
//         "  attributes:\n"
//         "    - entry1: { entry-type: list, data-type: AttrGroup1, group-count: 3, capacity: 3 }\n"
//         "    - entry2: { entry-type: list, data-type: AttrGroup1, group-count: 3, capacity: 3 }\n"
//         ,
//         "<metalib tagsetversion='1' name='net'  version='1'>"
//         "    <struct name='AttrGroup1' version='1' align='1'>"
//         "	     <entry name='a1' type='uint32' id='1'/>"
//         "    </struct>"
//         "</metalib>"
//         ) ;


//     EXPECT_STREQ(
//         "pom_grp_meta: name=TestObj, page-size=256, class-id=1, obj-size=12, page-count=2, size-buf-start=8, size-buf-count=2\n"
//         "    entry1: entry-type=list, data=type=AttrGroup1, capacity=3, size-idx=0, page-begin=0, page-count=1, class-id=2, page-size=12, obj-align=1\n"
//         "    entry2: entry-type=list, data=type=AttrGroup1, capacity=3, size-idx=1, page-begin=1, page-count=1, class-id=3, page-size=12, obj-align=1"
//         ,
//         t_pom_grp_meta_dump(m_meta));
// }

// TEST_F(OmGrpMetaTest, from_meta_bitarry_basic) {
//     t_em_set_print();
//     install(
//         "TestObj:\n"
//         "  attributes:\n"
//         "    - entry1: { entry-type: ba, bit-capacity: 15 }\n"
//         ,
//         "<metalib tagsetversion='1' name='net'  version='1'>"
//         "</metalib>"
//         ) ;

//     EXPECT_STREQ(
//         "pom_grp_meta: name=TestObj, page-size=256, class-id=1, obj-size=4, page-count=1, size-buf-start=4, size-buf-count=0\n"
//         "    entry1: entry-type=ba, bit-capacity=15, page-begin=0, page-count=1, class-id=2, page-size=2, obj-align=1"
//         ,
//         t_pom_grp_meta_dump(m_meta));
// }

