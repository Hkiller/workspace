#ifndef CPE_DR_TEST_HASHTEST_H
#define CPE_DR_TEST_HASHTEST_H
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/hash.h"

class HashTest : public testenv::fixture<> {
public:
    struct TestObject {
        char m_name[10];
        struct cpe_hash_entry m_he;
    };

    virtual void SetUp();
    virtual void TearDown();

    /*utils function for hstable */
    static uint32_t hash_fun(const void *, void * user_data);
    static int hash_cmp(const void * l, const void * r, void * user_data);
    static void hash_destory(void * obj, void * context);

    struct cpe_hash_table m_hash_table;

    TestObject * createTestObject(const char * name);
    TestObject * createTmpTestObject(const char * name);
};

#endif
