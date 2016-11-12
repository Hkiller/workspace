#include "HashTest.hpp"

TEST_F(HashTest, insert_unique_basic) {
    TestObject * testObj = createTestObject("a");
    EXPECT_EQ(
        0,
        cpe_hash_table_insert_unique(
            &m_hash_table, testObj));

    EXPECT_EQ(1, cpe_hash_table_count(&m_hash_table));
        
    EXPECT_TRUE(
        testObj ==
        cpe_hash_table_find(
            &m_hash_table,
            createTmpTestObject("a")));
}

TEST_F(HashTest, insert_unique_duplicate) {
    TestObject * testObj = createTestObject("a");
    EXPECT_EQ(
        0,
        cpe_hash_table_insert_unique(
            &m_hash_table, testObj));

    EXPECT_EQ(1, cpe_hash_table_count(&m_hash_table));

    TestObject * testObj2 = createTestObject("a");
    EXPECT_EQ(
        -1,
        cpe_hash_table_insert_unique(
            &m_hash_table, testObj2));
    mem_free(t_allocrator(), testObj2);

    EXPECT_EQ(1, cpe_hash_table_count(&m_hash_table));

    EXPECT_TRUE(
        testObj ==
        cpe_hash_table_find(
            &m_hash_table,
            createTmpTestObject("a")));
}

TEST_F(HashTest, insert_basic) {
    TestObject * testObj = createTestObject("a");
    EXPECT_EQ(
        0,
        cpe_hash_table_insert(
            &m_hash_table, testObj));
    EXPECT_EQ(1, cpe_hash_table_count(&m_hash_table));

    TestObject * testObj2 = createTestObject("a");
    EXPECT_EQ(
        0,
        cpe_hash_table_insert(
            &m_hash_table, testObj2));

    EXPECT_EQ(2, cpe_hash_table_count(&m_hash_table));

    EXPECT_TRUE(
        testObj2 ==
        cpe_hash_table_find(
            &m_hash_table,
            createTmpTestObject("a")));

    EXPECT_TRUE(
        testObj ==
        cpe_hash_table_find_next(
            &m_hash_table,
            testObj2));
}

TEST_F(HashTest, iterator_basic) {
    TestObject * testObj = createTestObject("a");
    EXPECT_EQ(
        0,
        cpe_hash_table_insert(
            &m_hash_table, testObj));
    EXPECT_EQ(1, cpe_hash_table_count(&m_hash_table));

    TestObject * testObj2 = createTestObject("a");
    EXPECT_EQ(
        0,
        cpe_hash_table_insert(
            &m_hash_table, testObj2));

    struct cpe_hash_it it;
    cpe_hash_it_init(&it, &m_hash_table);

    EXPECT_TRUE(testObj2 == cpe_hash_it_next(&it));
    EXPECT_TRUE(testObj == cpe_hash_it_next(&it));
    EXPECT_TRUE(NULL == cpe_hash_it_next(&it));
}

TEST_F(HashTest, remove_by_ins) {
    TestObject * testObj = createTestObject("a");
    EXPECT_EQ(
        0,
        cpe_hash_table_insert(
            &m_hash_table, testObj));
    EXPECT_EQ(1, cpe_hash_table_count(&m_hash_table));

    EXPECT_EQ(0, cpe_hash_table_remove_by_ins(&m_hash_table, testObj));

    EXPECT_EQ(0, cpe_hash_table_count(&m_hash_table));

    EXPECT_TRUE(
        NULL ==
        cpe_hash_table_find(
            &m_hash_table,
            createTmpTestObject("a")));

}

TEST_F(HashTest, remove_by_key_exist) {
    TestObject * testObj = createTestObject("a");
    EXPECT_EQ(
        0,
        cpe_hash_table_insert(
            &m_hash_table, testObj));
    EXPECT_EQ(1, cpe_hash_table_count(&m_hash_table));

    EXPECT_EQ(1, cpe_hash_table_remove_by_key(&m_hash_table, createTmpTestObject("a")));

    EXPECT_EQ(0, cpe_hash_table_count(&m_hash_table));

    EXPECT_TRUE(
        NULL ==
        cpe_hash_table_find(
            &m_hash_table,
            createTmpTestObject("a")));

}

TEST_F(HashTest, remove_by_key_not_exist) {
    TestObject * testObj = createTestObject("a");
    EXPECT_EQ(
        0,
        cpe_hash_table_insert(
            &m_hash_table, testObj));
    EXPECT_EQ(1, cpe_hash_table_count(&m_hash_table));

    EXPECT_EQ(0, cpe_hash_table_remove_by_key(&m_hash_table, createTmpTestObject("b")));

    EXPECT_EQ(1, cpe_hash_table_count(&m_hash_table));

    EXPECT_TRUE(
        NULL !=
        cpe_hash_table_find(
            &m_hash_table,
            createTmpTestObject("a")));

}

TEST_F(HashTest, remove_all_by_key_exist) {
    TestObject * testObj = createTestObject("a");
    EXPECT_EQ(
        0,
        cpe_hash_table_insert(
            &m_hash_table, testObj));
    EXPECT_EQ(1, cpe_hash_table_count(&m_hash_table));

    EXPECT_EQ(1, cpe_hash_table_remove_all_by_key(&m_hash_table, createTmpTestObject("a")));

    EXPECT_EQ(0, cpe_hash_table_count(&m_hash_table));

    EXPECT_TRUE(
        NULL ==
        cpe_hash_table_find(
            &m_hash_table,
            createTmpTestObject("a")));
}

TEST_F(HashTest, remove_all_by_key_multi) {
    EXPECT_EQ(
        0,
        cpe_hash_table_insert(
            &m_hash_table, createTestObject("a")));
    EXPECT_EQ(
        0,
        cpe_hash_table_insert(
            &m_hash_table, createTestObject("a")));
    EXPECT_EQ(
        0,
        cpe_hash_table_insert(
            &m_hash_table, createTestObject("a")));

    EXPECT_EQ(3, cpe_hash_table_count(&m_hash_table));

    EXPECT_EQ(3, cpe_hash_table_remove_all_by_key(&m_hash_table, createTmpTestObject("a")));

    EXPECT_EQ(0, cpe_hash_table_count(&m_hash_table));

    EXPECT_TRUE(
        NULL ==
        cpe_hash_table_find(
            &m_hash_table,
            createTmpTestObject("a")));

}

TEST_F(HashTest, remove_all_by_key_not_exist) {
    TestObject * testObj = createTestObject("a");
    EXPECT_EQ(
        0,
        cpe_hash_table_insert(
            &m_hash_table, testObj));
    EXPECT_EQ(1, cpe_hash_table_count(&m_hash_table));

    EXPECT_EQ(0, cpe_hash_table_remove_all_by_key(&m_hash_table, createTmpTestObject("b")));

    EXPECT_EQ(1, cpe_hash_table_count(&m_hash_table));

    EXPECT_TRUE(
        NULL !=
        cpe_hash_table_find(
            &m_hash_table,
            createTmpTestObject("a")));

}

