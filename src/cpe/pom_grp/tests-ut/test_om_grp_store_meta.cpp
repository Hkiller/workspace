#include "cpe/dr/dr_metalib_manage.h"
#include "OmGrpStoreTest.hpp" 

TEST_F(OmGrpStoreTest, basic) {
    install(
        "TestObj:\n"
        "  main-entry: entry1\n"
        "  attributes:\n"
        "    - entry1: { entry-type: normal, data-type: AttrGroup1 }\n"
        ,
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='AttrGroup1' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "</metalib>"
        ) ;

    EXPECT_STREQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<metalib tagsetversion=\"1\" name=\"TestObj\" version=\"1\">\n"
        "    <struct name=\"AttrGroup1\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"a1\" type=\"uint32\" id=\"1\"/>\n"
        "    </struct>\n"
        "    <struct name=\"TestObj\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"entry1\" type=\"AttrGroup1\"/>\n"
        "    </struct>\n"
        "    <struct name=\"TestObjList\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"count\" type=\"uint32\"/>\n"
        "        <entry name=\"data\" type=\"TestObj\" count=\"0\" refer=\"count\"/>\n"
        "    </struct>\n"
        "</metalib>\n"
        ,
        str_store_meta());
}

TEST_F(OmGrpStoreTest, multi_normal) {
    install(
        "TestObj:\n"
        "  main-entry: entry1\n"
        "  attributes:\n"
        "    - entry1: { entry-type: normal, data-type: AttrGroup1 }\n"
        "    - entry2: { entry-type: normal, data-type: AttrGroup2 }\n"
        ,
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='AttrGroup1' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='AttrGroup2' version='1' align='1'>"
        "	     <entry name='b1' type='uint32' id='2'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_STREQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<metalib tagsetversion=\"1\" name=\"TestObj\" version=\"1\">\n"
        "    <struct name=\"AttrGroup1\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"a1\" type=\"uint32\" id=\"1\"/>\n"
        "    </struct>\n"
        "    <struct name=\"AttrGroup2\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"b1\" type=\"uint32\" id=\"2\"/>\n"
        "    </struct>\n"
        "    <struct name=\"TestObj\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"entry1\" type=\"AttrGroup1\"/>\n"
        "        <entry name=\"entry2\" type=\"AttrGroup2\"/>\n"
        "    </struct>\n"
        "    <struct name=\"TestObjList\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"count\" type=\"uint32\"/>\n"
        "        <entry name=\"data\" type=\"TestObj\" count=\"0\" refer=\"count\"/>\n"
        "    </struct>\n"
        "</metalib>\n"
        ,
        str_store_meta());
}

TEST_F(OmGrpStoreTest, multi_ba) {
    install(
        "TestObj:\n"
        "  main-entry: entry1\n"
        "  attributes:\n"
        "    - entry1: { entry-type: normal, data-type: AttrGroup1 }\n"
        "    - entry2: { entry-type: ba, bit-capacity: 30, byte-per-page=2 }\n"
        ,
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='AttrGroup1' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_STREQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<metalib tagsetversion=\"1\" name=\"TestObj\" version=\"1\">\n"
        "    <struct name=\"AttrGroup1\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"a1\" type=\"uint32\" id=\"1\"/>\n"
        "    </struct>\n"
        "    <struct name=\"TestObj\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"entry1\" type=\"AttrGroup1\"/>\n"
        "        <entry name=\"entry2\" type=\"uint8\" count=\"4\"/>\n"
        "    </struct>\n"
        "    <struct name=\"TestObjList\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"count\" type=\"uint32\"/>\n"
        "        <entry name=\"data\" type=\"TestObj\" count=\"0\" refer=\"count\"/>\n"
        "    </struct>\n"
        "</metalib>\n"
        ,
        str_store_meta());
}

TEST_F(OmGrpStoreTest, multi_binary) {
    install(
        "TestObj:\n"
        "  main-entry: entry1\n"
        "  attributes:\n"
        "    - entry1: { entry-type: normal, data-type: AttrGroup1 }\n"
        "    - entry2: { entry-type: binary, capacity: 5 }\n"
        ,
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='AttrGroup1' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_STREQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<metalib tagsetversion=\"1\" name=\"TestObj\" version=\"1\">\n"
        "    <struct name=\"AttrGroup1\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"a1\" type=\"uint32\" id=\"1\"/>\n"
        "    </struct>\n"
        "    <struct name=\"TestObj\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"entry1\" type=\"AttrGroup1\"/>\n"
        "        <entry name=\"entry2\" type=\"uint8\" count=\"5\"/>\n"
        "    </struct>\n"
        "    <struct name=\"TestObjList\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"count\" type=\"uint32\"/>\n"
        "        <entry name=\"data\" type=\"TestObj\" count=\"0\" refer=\"count\"/>\n"
        "    </struct>\n"
        "</metalib>\n"
        ,
        str_store_meta());
}

TEST_F(OmGrpStoreTest, multi_list) {
    install(
        "TestObj:\n"
        "  main-entry: entry1\n"
        "  attributes:\n"
        "    - entry1: { entry-type: normal, data-type: AttrGroup1 }\n"
        "    - entry2: { entry-type: list, data-type: AttrGroup2, group-count: 3, capacity: 5 }\n"
        ,
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='AttrGroup1' version='1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='AttrGroup2' version='1' align='1'>"
        "	     <entry name='b1' type='uint32' id='2'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_STREQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<metalib tagsetversion=\"1\" name=\"TestObj\" version=\"1\">\n"
        "    <struct name=\"AttrGroup1\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"a1\" type=\"uint32\" id=\"1\"/>\n"
        "    </struct>\n"
        "    <struct name=\"AttrGroup2\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"b1\" type=\"uint32\" id=\"2\"/>\n"
        "    </struct>\n"
        "    <struct name=\"TestObj\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"entry1\" type=\"AttrGroup1\"/>\n"
        "        <entry name=\"entry2Count\" type=\"uint32\"/>\n"
        "        <entry name=\"entry2\" type=\"AttrGroup2\" count=\"5\" refer=\"entry2Count\"/>\n"
        "    </struct>\n"
        "    <struct name=\"TestObjList\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"count\" type=\"uint32\"/>\n"
        "        <entry name=\"data\" type=\"TestObj\" count=\"0\" refer=\"count\"/>\n"
        "    </struct>\n"
        "</metalib>\n"
        ,
        str_store_meta());

    LPDRMETA data_meta = dr_lib_find_meta_by_name(store_meta(), "TestObj");
    ASSERT_TRUE(data_meta);

    LPDRMETAENTRY data_entry = dr_meta_find_entry_by_name(data_meta, "entry2");
    ASSERT_TRUE(data_entry);
    EXPECT_EQ((size_t)8, dr_entry_data_start_pos(data_entry, 0));
}

TEST_F(OmGrpStoreTest, multi_list_standalone) {
    install(
        "TestObj:\n"
        "  main-entry: entry1\n"
        "  attributes:\n"
        "    - entry1: { entry-type: normal, data-type: AttrGroup1 }\n"
        "    - entry2: { entry-type: list, data-type: AttrGroup2, group-count: 3, capacity: 5, standalone: 1 }\n"
        ,
        "<metalib tagsetversion='1' name='net'  version='1'>"
        "    <struct name='AttrGroup1' version='1' primarykey='a1' align='1'>"
        "	     <entry name='a1' type='uint32' id='1'/>"
        "    </struct>"
        "    <struct name='AttrGroup2' version='1' align='1'>"
        "	     <entry name='b1' type='uint32' id='2'/>"
        "    </struct>"
        "</metalib>"
        );

    EXPECT_STREQ(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<metalib tagsetversion=\"1\" name=\"TestObj\" version=\"1\">\n"
        "    <struct name=\"AttrGroup1\" version=\"1\" primarykey=\"a1\" align=\"1\">\n"
        "        <entry name=\"a1\" type=\"uint32\" id=\"1\"/>\n"
        "    </struct>\n"
        "    <struct name=\"AttrGroup2\" version=\"1\" primarykey=\"a1\" align=\"1\">\n"
        "        <entry name=\"a1\" type=\"uint32\" id=\"1\"/>\n"
        "        <entry name=\"b1\" type=\"uint32\" id=\"2\"/>\n"
        "    </struct>\n"
        "    <struct name=\"TestObj\" version=\"1\" primarykey=\"a1\" align=\"1\">\n"
        "        <entry name=\"a1\" type=\"uint32\" id=\"1\"/>\n"
        "        <entry name=\"entry1\" type=\"AttrGroup1\"/>\n"
        "    </struct>\n"
        "    <struct name=\"TestObjList\" version=\"1\" align=\"1\">\n"
        "        <entry name=\"count\" type=\"uint32\"/>\n"
        "        <entry name=\"data\" type=\"TestObj\" count=\"0\" refer=\"count\"/>\n"
        "    </struct>\n"
        "</metalib>\n"
        ,
        str_store_meta());
}

