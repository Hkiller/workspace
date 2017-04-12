#include "AomObjHashTest.hpp"

class AomObjHashBasicTest : public AomObjHashTest {
public:
    struct Record {
        uint64_t key;
        uint64_t value1;
        uint64_t value2;
    };
 
    virtual void SetUp() { 
        AomObjHashTest::SetUp();
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

        create_aom_obj_hash(1.5);
    }
};

TEST_F(AomObjHashBasicTest, insert_basic) {
    Record r1 = { 1, 11, 12 };

    ptr_int_t record_id;
    ASSERT_EQ(0, aom_obj_hash_table_insert(m_obj_hash, &r1, &record_id));

    EXPECT_EQ(1, (int)record_id);

    Record * finded_r1 = (Record*)aom_obj_hash_table_find(m_obj_hash, &r1);
    ASSERT_TRUE(finded_r1 != NULL);

    EXPECT_EQ((uint64_t)1, finded_r1->key);
    EXPECT_EQ((uint64_t)11, finded_r1->value1);
    EXPECT_EQ((uint64_t)12, finded_r1->value2);
}

TEST_F(AomObjHashBasicTest, insert_same_key) {
    Record r1 = { 1, 11, 12 };

    ptr_int_t record_id;
    ASSERT_EQ(0, aom_obj_hash_table_insert(m_obj_hash, &r1, &record_id));
    EXPECT_EQ(1, (int)record_id);

    r1.value1 = 21;
    r1.value2 = 22;

    ASSERT_EQ(0, aom_obj_hash_table_insert(m_obj_hash, &r1, &record_id));
    EXPECT_EQ(2, (int)record_id);

    Record * finded_r1 = (Record*)aom_obj_hash_table_find(m_obj_hash, &r1);
    ASSERT_TRUE(finded_r1 != NULL);

    EXPECT_EQ((uint64_t)1, finded_r1->key);
    EXPECT_EQ((uint64_t)11, finded_r1->value1);
    EXPECT_EQ((uint64_t)12, finded_r1->value2);

    Record * finded_r2 = (Record*)aom_obj_hash_table_find_next(m_obj_hash, finded_r1);
    ASSERT_TRUE(finded_r2 != NULL);

    EXPECT_EQ((uint64_t)1, finded_r2->key);
    EXPECT_EQ((uint64_t)21, finded_r2->value1);
    EXPECT_EQ((uint64_t)22, finded_r2->value2);
}

TEST_F(AomObjHashBasicTest, insert_unique_basic) {
    Record r1 = { 1, 11, 12 };

    ptr_int_t record_id;
    ASSERT_EQ(0, aom_obj_hash_table_insert_unique(m_obj_hash, &r1, &record_id));

    EXPECT_EQ(1, (int)record_id);

    Record * finded_r1 = (Record*)aom_obj_hash_table_find(m_obj_hash, &r1);
    ASSERT_TRUE(finded_r1 != NULL);

    EXPECT_EQ((uint64_t)1, finded_r1->key);
    EXPECT_EQ((uint64_t)11, finded_r1->value1);
    EXPECT_EQ((uint64_t)12, finded_r1->value2);
}

TEST_F(AomObjHashBasicTest, insert_unique_duplicate) {
    Record r1 = { 1, 11, 12 };

    ptr_int_t record_id;
    ASSERT_EQ(0, aom_obj_hash_table_insert_unique(m_obj_hash, &r1, &record_id));
    EXPECT_EQ(1, (int)record_id);

    ASSERT_NE(0, aom_obj_hash_table_insert_unique(m_obj_hash, &r1, &record_id));

    Record * finded_r1 = (Record*)aom_obj_hash_table_find(m_obj_hash, &r1);
    ASSERT_TRUE(finded_r1 != NULL);
    EXPECT_TRUE(aom_obj_hash_table_find_next(m_obj_hash, finded_r1) == NULL);

    EXPECT_EQ((uint64_t)1, finded_r1->key);
    EXPECT_EQ((uint64_t)11, finded_r1->value1);
    EXPECT_EQ((uint64_t)12, finded_r1->value2);
}

TEST_F(AomObjHashBasicTest, insert_unique_multi) {
    Record r1 = { 1, 11, 12 };
    ptr_int_t record_id1;
    ASSERT_EQ(0, aom_obj_hash_table_insert_unique(m_obj_hash, &r1, &record_id1));

    Record r2 = { 2, 21, 22 };
    ptr_int_t record_id2;
    ASSERT_EQ(0, aom_obj_hash_table_insert_unique(m_obj_hash, &r2, &record_id2));
    EXPECT_EQ(2, (int)record_id2);

    Record * finded_r2 = (Record*)aom_obj_hash_table_find(m_obj_hash, &r2);
    ASSERT_TRUE(finded_r2 != NULL);
    EXPECT_TRUE(aom_obj_hash_table_find_next(m_obj_hash, finded_r2) == NULL);

    EXPECT_EQ((uint64_t)2, finded_r2->key);
    EXPECT_EQ((uint64_t)21, finded_r2->value1);
    EXPECT_EQ((uint64_t)22, finded_r2->value2);
}

TEST_F(AomObjHashBasicTest, insert_or_update_do_insert) {
    Record r1 = { 1, 11, 12 };

    ptr_int_t record_id;
    ASSERT_EQ(0, aom_obj_hash_table_insert_or_update(m_obj_hash, &r1, &record_id));

    EXPECT_EQ(1, (int)record_id);

    Record * finded_r1 = (Record*)aom_obj_hash_table_find(m_obj_hash, &r1);
    ASSERT_TRUE(finded_r1 != NULL);
    EXPECT_TRUE(aom_obj_hash_table_find_next(m_obj_hash, finded_r1) == NULL);

    EXPECT_EQ((uint64_t)1, finded_r1->key);
    EXPECT_EQ((uint64_t)11, finded_r1->value1);
    EXPECT_EQ((uint64_t)12, finded_r1->value2);
}

TEST_F(AomObjHashBasicTest, insert_or_update_do_update) {
    Record r1 = { 1, 11, 12 };

    ptr_int_t record_id;
    ASSERT_EQ(0, aom_obj_hash_table_insert_or_update(m_obj_hash, &r1, &record_id));
    EXPECT_EQ(1, (int)record_id);

    r1.value1 = 21;
    r1.value2 = 22;

    ASSERT_EQ(0, aom_obj_hash_table_insert_or_update(m_obj_hash, &r1, &record_id));
    EXPECT_EQ(1, (int)record_id);

    Record * finded_r1 = (Record*)aom_obj_hash_table_find(m_obj_hash, &r1);
    ASSERT_TRUE(finded_r1 != NULL);
    EXPECT_TRUE(aom_obj_hash_table_find_next(m_obj_hash, finded_r1) == NULL);

    EXPECT_EQ((uint64_t)1, finded_r1->key);
    EXPECT_EQ((uint64_t)21, finded_r1->value1);
    EXPECT_EQ((uint64_t)22, finded_r1->value2);
}

TEST_F(AomObjHashBasicTest, remove_by_ins_basic) {
    Record r1 = { 1, 11, 12 };
    ASSERT_EQ(0, aom_obj_hash_table_insert_unique(m_obj_hash, &r1, NULL));

    Record * finded_r1 = (Record*)aom_obj_hash_table_find(m_obj_hash, &r1);
    ASSERT_TRUE(finded_r1 != NULL);

    EXPECT_EQ(0, aom_obj_hash_table_remove_by_ins(m_obj_hash, finded_r1));

    EXPECT_TRUE(aom_obj_hash_table_find(m_obj_hash, &r1) == NULL);
}

TEST_F(AomObjHashBasicTest, remove_by_ins_middle) {
    Record r1 = { 1, 11, 12 };

    ASSERT_EQ(0, aom_obj_hash_table_insert(m_obj_hash, &r1, NULL));
    Record * finded_r1 = (Record*)aom_obj_hash_table_find(m_obj_hash, &r1);
    ASSERT_TRUE(finded_r1 != NULL);

    ASSERT_EQ(0, aom_obj_hash_table_insert(m_obj_hash, &r1, NULL));
    Record * finded_r2 = (Record*)aom_obj_hash_table_find_next(m_obj_hash, finded_r1);
    ASSERT_TRUE(finded_r2 != NULL);

    ASSERT_EQ(0, aom_obj_hash_table_insert(m_obj_hash, &r1, NULL));
    Record * finded_r3 = (Record*)aom_obj_hash_table_find_next(m_obj_hash, finded_r2);
    ASSERT_TRUE(finded_r3 != NULL);

    EXPECT_TRUE(NULL == (Record*)aom_obj_hash_table_find_next(m_obj_hash, finded_r3));

    EXPECT_EQ(0, aom_obj_hash_table_remove_by_ins(m_obj_hash, finded_r2));

    EXPECT_TRUE(finded_r1 == (Record*)aom_obj_hash_table_find(m_obj_hash, &r1));
    EXPECT_TRUE(finded_r3 == (Record*)aom_obj_hash_table_find_next(m_obj_hash, finded_r1));
    EXPECT_TRUE(NULL == (Record*)aom_obj_hash_table_find_next(m_obj_hash, finded_r3));
}

TEST_F(AomObjHashBasicTest, remove_by_key_basic) {
    Record r1 = { 1, 11, 12 };

    ASSERT_EQ(0, aom_obj_hash_table_insert(m_obj_hash, &r1, NULL));
    Record * finded_r1 = (Record*)aom_obj_hash_table_find(m_obj_hash, &r1);
    ASSERT_TRUE(finded_r1 != NULL);

    ASSERT_EQ(0, aom_obj_hash_table_insert(m_obj_hash, &r1, NULL));
    Record * finded_r2 = (Record*)aom_obj_hash_table_find_next(m_obj_hash, finded_r1);
    ASSERT_TRUE(finded_r2 != NULL);

    EXPECT_TRUE(NULL == (Record*)aom_obj_hash_table_find_next(m_obj_hash, finded_r2));

    EXPECT_EQ(0, aom_obj_hash_table_remove_by_key(m_obj_hash, &r1));

    EXPECT_TRUE(finded_r2 == (Record*)aom_obj_hash_table_find(m_obj_hash, &r1));
    EXPECT_TRUE(NULL == (Record*)aom_obj_hash_table_find_next(m_obj_hash, finded_r2));
}

TEST_F(AomObjHashBasicTest, remove_by_key_not_exist) {
    Record r1 = { 1, 11, 12 };

    EXPECT_EQ((int)aom_obj_hash_table_error_not_exist, aom_obj_hash_table_remove_by_key(m_obj_hash, &r1));

    EXPECT_TRUE(NULL == (Record*)aom_obj_hash_table_find(m_obj_hash, &r1));
}

TEST_F(AomObjHashBasicTest, remove_all_by_key_basic) {
    Record r1 = { 1, 11, 12 };

    ASSERT_EQ(0, aom_obj_hash_table_insert(m_obj_hash, &r1, NULL));
    ASSERT_EQ(0, aom_obj_hash_table_insert(m_obj_hash, &r1, NULL));
    ASSERT_EQ(0, aom_obj_hash_table_insert(m_obj_hash, &r1, NULL));

    EXPECT_EQ(0, aom_obj_hash_table_remove_all_by_key(m_obj_hash, &r1));

    EXPECT_TRUE(NULL == (Record*)aom_obj_hash_table_find(m_obj_hash, &r1));
}
