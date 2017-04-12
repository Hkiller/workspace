#include "AomObjManagerTest.hpp"

class AomObjManagerBasicTest : public AomObjManagerTest {
public:
    virtual void SetUp() { 
        AomObjManagerTest::SetUp();
        t_em_set_print();
        create_aom_obj_mgr(
            "<metalib version='1' tagsetversion='1'>\n"
            "    <struct name='Record' version='1' primarykey='key'>\n"
            "        <entry name='key' type='uint64'/>\n"
            "        <entry name='value1' type='uint64'/>\n"
            "        <entry name='value2' type='uint64'/>\n"
            "    </struct>\n"
            "</metalib>\n"
            ,
            100);
    }
};

TEST_F(AomObjManagerBasicTest, empty_env) {
    EXPECT_EQ(aom_obj_mgr_free_obj_count(m_obj_mgr), 100);
    EXPECT_EQ(aom_obj_mgr_allocked_obj_count(m_obj_mgr), 0);
}

TEST_F(AomObjManagerBasicTest, alloc_first) {
    void * record = aom_obj_alloc(m_obj_mgr);
    EXPECT_TRUE(record != NULL);

    EXPECT_EQ(aom_obj_mgr_free_obj_count(m_obj_mgr), 99);
    EXPECT_EQ(aom_obj_mgr_allocked_obj_count(m_obj_mgr), 1);
}

TEST_F(AomObjManagerBasicTest, alloc_all) {
    for(int i = 0; i < 100; ++i) {
        void * record = aom_obj_alloc(m_obj_mgr);
        EXPECT_TRUE(record != NULL);
        EXPECT_EQ(i, (int)aom_obj_index(m_obj_mgr, record));
    }


    EXPECT_EQ(aom_obj_mgr_free_obj_count(m_obj_mgr), 0);
    EXPECT_EQ(aom_obj_mgr_allocked_obj_count(m_obj_mgr), 100);
}

TEST_F(AomObjManagerBasicTest, alloc_overflow) {
    for(int i = 0; i < 100; ++i) {
        void * record = aom_obj_alloc(m_obj_mgr);
        EXPECT_TRUE(record != NULL);
    }

    EXPECT_TRUE(aom_obj_alloc(m_obj_mgr) == NULL);
}

TEST_F(AomObjManagerBasicTest, free_basic) {
    void * record = aom_obj_alloc(m_obj_mgr);
    EXPECT_TRUE(record != NULL);

    aom_obj_free(m_obj_mgr, record);

    EXPECT_EQ(aom_obj_mgr_free_obj_count(m_obj_mgr), 100);
    EXPECT_EQ(aom_obj_mgr_allocked_obj_count(m_obj_mgr), 0);
}

TEST_F(AomObjManagerBasicTest, alloc_again) {
    void * record1 = aom_obj_alloc(m_obj_mgr);
    EXPECT_TRUE(record1 != NULL);

    void * record2 = aom_obj_alloc(m_obj_mgr);
    EXPECT_TRUE(record2 != NULL);

    aom_obj_free(m_obj_mgr, record1);

    void * record3 = aom_obj_alloc(m_obj_mgr);
    EXPECT_TRUE(record3 != NULL);

    EXPECT_TRUE(record1 == record3);

    EXPECT_EQ(aom_obj_mgr_free_obj_count(m_obj_mgr), 98);
    EXPECT_EQ(aom_obj_mgr_allocked_obj_count(m_obj_mgr), 2);
}

TEST_F(AomObjManagerBasicTest, obj_to_index_basic) {
    EXPECT_EQ(0, (int)aom_obj_index(m_obj_mgr, aom_obj_alloc(m_obj_mgr)));
    EXPECT_EQ(1, (int)aom_obj_index(m_obj_mgr, aom_obj_alloc(m_obj_mgr)));
    EXPECT_EQ(2, (int)aom_obj_index(m_obj_mgr, aom_obj_alloc(m_obj_mgr)));
}
