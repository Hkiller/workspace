#ifndef CPE_AOM_TEST_AOMOBJHASHTEST_H
#define CPE_AOM_TEST_AOMOBJHASHTEST_H
#include "cpe/aom/aom_obj_hash.h"
#include "AomObjManagerTest.hpp"

typedef ::Loki::NullType AomObjHashTestBase;

class AomObjHashTest : public testenv::fixture<AomObjHashTestBase, AomObjManagerTest> {
public:
    virtual void SetUp();
    virtual void TearDown();

    void create_aom_obj_hash(float bucket_ratio);

    aom_obj_hash_table_t m_obj_hash;
};

#endif
