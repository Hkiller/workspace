#include <sstream>
#include "cpe/pom_grp/pom_grp_obj.h"
#include "OmGrpObjMgrTest.hpp" 

class OmGrpObjListTest : public OmGrpObjMgrTest {
public:
    struct AttrGroup1 {
        uint32_t a1;
    };

    virtual void SetUp() {
        OmGrpObjMgrTest::SetUp();
        
        install(
            "TestObj:\n"
            "  main-entry: entry1\n"
            "  attributes:\n"
            "    - entry1: { entry-type: list, data-type: AttrGroup1, group-count: 3, capacity: 5 }\n"
            ,
            "<metalib tagsetversion='1' name='net'  version='1'>"
            "    <struct name='AttrGroup1' version='1' align='1'>"
            "	     <entry name='a1' type='uint32' id='1'/>"
            "    </struct>"
            "</metalib>"
            ) ;

        m_obj = pom_grp_obj_alloc(m_mgr);
        ASSERT_TRUE(m_obj);
    }

    virtual void TearDown() {
        pom_grp_obj_free(m_mgr, m_obj);
        OmGrpObjMgrTest::TearDown();
    }

    int append(uint32_t value) {
        AttrGroup1 t;
        t.a1 = value;
        return pom_grp_obj_list_append(m_mgr, m_obj, "entry1", &t);
    }

    void insert(uint32_t pos, uint32_t value) {
        AttrGroup1 t;
        t.a1 = value;
        ASSERT_EQ(0, pom_grp_obj_list_insert(m_mgr, m_obj, "entry1", pos, &t));
    }

    int remove(uint32_t pos) {
        return pom_grp_obj_list_remove(m_mgr, m_obj, "entry1", pos);
    }

    uint32_t at(uint32_t pos) {
        AttrGroup1 * r = (AttrGroup1 *)pom_grp_obj_list_at(m_mgr, m_obj, "entry1", pos);
        EXPECT_TRUE(r) << "value at " << pos << " not exist!";
        return r ? r->a1 : (uint32_t)-1;
    }

    uint16_t count(void) {
        return pom_grp_obj_list_count(m_mgr, m_obj, "entry1");
    }

    const char * dump(void) {
        ::std::ostringstream ss;

        for (uint16_t i = 0; i < count(); ++i) {
            if (i) ss << ":";
            ss << at(i);
        }

        return t_tmp_strdup(ss.str().c_str());
    }

    pom_grp_obj_t m_obj;
};

TEST_F(OmGrpObjListTest, count_empty) {
    EXPECT_EQ(0, count());
}

TEST_F(OmGrpObjListTest, at_overflow_capacity) {
    EXPECT_TRUE(pom_grp_obj_list_at(m_mgr, m_obj, "entry1", 6) == NULL);
}

TEST_F(OmGrpObjListTest, at_overflow_count) {
    EXPECT_TRUE(pom_grp_obj_list_at(m_mgr, m_obj, "entry1", 0) == NULL);
}

TEST_F(OmGrpObjListTest, at_overflow_count_not_empty) {
    append(1);
    EXPECT_TRUE(pom_grp_obj_list_at(m_mgr, m_obj, "entry1", 1) == NULL);
}

TEST_F(OmGrpObjListTest, append_basic) {
    append(1);
    EXPECT_EQ((uint32_t)1, at(0));
}

TEST_F(OmGrpObjListTest, append_all) {
    append(1);
    append(2);
    append(3);
    append(4);
    append(5);

    EXPECT_STREQ("1:2:3:4:5", dump());
    EXPECT_STREQ("0x2000000:0x2000001:", pages(m_obj));
}

TEST_F(OmGrpObjListTest, append_overflow) {
    append(1);
    append(2);
    append(3);
    append(4);
    append(5);

    AttrGroup1 t;
    t.a1 = 6;
    EXPECT_NE(0, pom_grp_obj_list_append(m_mgr, m_obj, "entry1", &t));

    EXPECT_STREQ("1:2:3:4:5", dump());
    EXPECT_STREQ("0x2000000:0x2000001:", pages(m_obj));
}

TEST_F(OmGrpObjListTest, insert_to_empty) {
    AttrGroup1 v1;
    v1.a1 = 1;
    EXPECT_EQ(0, pom_grp_obj_list_insert(m_mgr, m_obj, "entry1", 0, &v1));

    EXPECT_EQ(1, count());
    EXPECT_EQ((uint32_t)1, at(0));
}

TEST_F(OmGrpObjListTest, insert_to_page_last) {
    append(1);
    append(2);
    append(4);

    insert(2, 3);

    EXPECT_STREQ("1:2:3:4", dump());
    EXPECT_STREQ("0x2000000:0x2000001:", pages(m_obj));
}

TEST_F(OmGrpObjListTest, insert_to_page_middle) {
    append(1);
    append(3);
    append(4);

    insert(1, 2);

    EXPECT_STREQ("1:2:3:4", dump());
}

TEST_F(OmGrpObjListTest, insert_to_page_first) {
    append(2);
    append(3);
    append(4);

    insert(0, 1);

    EXPECT_STREQ("1:2:3:4", dump());
}

TEST_F(OmGrpObjListTest, remote_empty) {
    EXPECT_NE(0, pom_grp_obj_list_remove(m_mgr, m_obj, "entry1", 0));
}

TEST_F(OmGrpObjListTest, remote_to_empty) {
    append(1);
    append(2);
    append(3);
    append(4);

    EXPECT_EQ(4, count());

    EXPECT_EQ(0, remove(0));
    EXPECT_EQ(3, count());

    EXPECT_EQ(0, remove(0));
    EXPECT_EQ(2, count());

    EXPECT_EQ(0, remove(0));
    EXPECT_EQ(1, count());

    EXPECT_EQ(0, remove(0));
    EXPECT_EQ(0, count());

    EXPECT_NE(0, remove(0));
    EXPECT_EQ(0, count());
}

TEST_F(OmGrpObjListTest, remote_page_first) {
    append(1);
    append(2);
    append(3);
    append(4);

    remove(0);
    EXPECT_STREQ("2:3:4", dump());
    EXPECT_STREQ("0x2000000:0x0:", pages(m_obj));
}

TEST_F(OmGrpObjListTest, remote_page_middle) {
    append(1);
    append(2);
    append(3);
    append(4);

    remove(1);
    EXPECT_STREQ("1:3:4", dump());
    EXPECT_STREQ("0x2000000:0x0:", pages(m_obj));
}

TEST_F(OmGrpObjListTest, remote_page_last) {
    append(1);
    append(2);
    append(3);
    append(4);

    remove(2);
    EXPECT_STREQ("1:2:4", dump());
    EXPECT_STREQ("0x2000000:0x0:", pages(m_obj));
}

TEST_F(OmGrpObjListTest, remote_last_page) {
    append(1);
    append(2);
    append(3);
    append(4);

    remove(3);
    EXPECT_STREQ("1:2:3", dump());
    EXPECT_STREQ("0x2000000:0x0:", pages(m_obj));
}

TEST_F(OmGrpObjListTest, remote_last_page_left) {
    append(1);
    append(2);
    append(3);
    append(4);
    append(5);

    remove(3);
    EXPECT_STREQ("1:2:3:5", dump());
}

