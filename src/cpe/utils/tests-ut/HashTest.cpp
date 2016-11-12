#include "cpe/utils/string_utils.h"
#include "HashTest.hpp"

void HashTest::SetUp(void) {
    Base::SetUp();

    EXPECT_EQ(
        0,
        cpe_hash_table_init(
            &m_hash_table,
            t_allocrator(),
            hash_fun,
            hash_cmp,
            CPE_HASH_OBJ2ENTRY(TestObject, m_he),
            -1));

    cpe_hash_table_set_destory_fun(
        &m_hash_table,
        hash_destory,
        t_allocrator());
}

void HashTest::TearDown(void) {
    cpe_hash_table_fini(&m_hash_table);

    Base::TearDown();
}

uint32_t HashTest::hash_fun(const void * o, void * user_data) {
    const char * str = ((TestObject *)o)->m_name;
    return cpe_hash_str(str, strlen(str));
}

int HashTest::hash_cmp(const void * l, const void * r, void * user_data) {
    return 0 == strcmp(((TestObject *)l)->m_name, ((TestObject *)r)->m_name);
}

void HashTest::hash_destory(void * obj, void * context) {
    mem_free((mem_allocrator_t)context, obj);
}

HashTest::TestObject *
HashTest::createTestObject(const char * name) {
    TestObject * p = (TestObject *)t_alloc(sizeof(struct TestObject));
    cpe_str_dup(p->m_name, 9, name);
    cpe_hash_entry_init(&p->m_he);
    return p;
}

HashTest::TestObject *
HashTest::createTmpTestObject(const char * name) {
    TestObject * p = (TestObject *)t_tmp_alloc(sizeof(struct TestObject));
    cpe_str_dup(p->m_name, 9, name);
    cpe_hash_entry_init(&p->m_he);
    return p;
}

