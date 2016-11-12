#include "cpe/dr/dr_metalib_manage.h"
#include "AomObjManagerTest.hpp"

void AomObjManagerTest::SetUp() {
    Base::SetUp();
    m_obj_mgr = NULL;
}

void AomObjManagerTest::TearDown() {
    if (m_obj_mgr) {
        aom_obj_mgr_free(m_obj_mgr);
        m_obj_mgr = NULL;
    }

    Base::TearDown();
}

void AomObjManagerTest::create_aom_obj_mgr(const char * str_meta, uint32_t record_count) {
    if (m_obj_mgr) {
        aom_obj_mgr_free(m_obj_mgr);
        m_obj_mgr = NULL;
    }

    LPDRMETALIB metalib = t_create_metalib(str_meta);
    EXPECT_TRUE(metalib != NULL);
    EXPECT_EQ(dr_lib_meta_num(metalib), 1);

    LPDRMETA meta = dr_lib_meta_at(metalib, 0);
    EXPECT_TRUE(meta != NULL);

    size_t buff_capacity;
    EXPECT_EQ(aom_obj_mgr_buf_calc_capacity(&buff_capacity, meta, record_count, t_em()), 0);

    void * buff = t_tmp_alloc(buff_capacity);
    EXPECT_TRUE(buff != NULL);

    EXPECT_EQ(aom_obj_mgr_buf_init(meta, buff, buff_capacity, t_em()), 0);

    m_obj_mgr = aom_obj_mgr_create(t_allocrator(), buff, buff_capacity, t_em());
    EXPECT_TRUE(m_obj_mgr != NULL);
}
