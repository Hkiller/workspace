#include "cpe/dr/dr_data.h"
#include "AomObjHashTest.hpp"

void AomObjHashTest::SetUp() {
    Base::SetUp();
    m_obj_hash = NULL;
}

void AomObjHashTest::TearDown() {
    if (m_obj_hash) {
        aom_obj_hash_table_free(m_obj_hash);
        m_obj_hash = NULL;
    }

    Base::TearDown();
}

void AomObjHashTest::create_aom_obj_hash(float bucket_ratio) {
    ASSERT_TRUE(m_obj_mgr != NULL);

    if (m_obj_hash) {
        aom_obj_hash_table_free(m_obj_hash);
        m_obj_hash = NULL;
    }

    size_t buff_capacity = aom_obj_hash_table_buf_calc_capacity(m_obj_mgr, bucket_ratio);

    void * buff = t_tmp_alloc(buff_capacity);
    ASSERT_TRUE(buff != NULL);

    ASSERT_EQ(
        aom_obj_hash_table_buf_init(
            m_obj_mgr, bucket_ratio, dr_meta_key_hash,
            buff, buff_capacity, t_em())
        , 0);
    
    m_obj_hash = aom_obj_hash_table_create(
        t_allocrator(), t_em(),
        m_obj_mgr, dr_meta_key_hash, dr_meta_key_cmp,
        buff, buff_capacity);
    ASSERT_TRUE(m_obj_hash != NULL);
}
