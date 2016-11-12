#include "cpe/pom_grp/pom_grp_meta.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "OmGrpStoreTest.hpp"

OmGrpStoreTest::OmGrpStoreTest() : m_meta(NULL), m_store(NULL) {
}

void OmGrpStoreTest::SetUp() {
    Base::SetUp();
}

void OmGrpStoreTest::TearDown() {
    if (m_store) {
        pom_grp_store_free(m_store);
        m_store = NULL;
    }

    if (m_meta) {
        pom_grp_meta_free(m_meta);
        m_meta = NULL;
    }

    Base::TearDown();
}

void OmGrpStoreTest::install(const char * om_meta, const char * metalib) {
    if (m_store) {
        pom_grp_store_free(m_store);
        m_store = NULL;
    }

    if (m_meta) {
        pom_grp_meta_free(m_meta);
        m_meta = NULL;
    }

    t_em_set_print();

    m_input_metalib = t_create_metalib(metalib);
    ASSERT_TRUE(m_input_metalib);
    if (m_input_metalib == NULL) return;

    m_meta = t_pom_grp_meta_create_by_cfg(om_meta, m_input_metalib, 1024);
    ASSERT_TRUE(m_meta);
    if (m_meta == NULL) return;

    pom_grp_entry_meta_t main_entry = pom_grp_meta_main_entry(m_meta);
    ASSERT_TRUE(main_entry);

    LPDRMETA dr_meta = dr_lib_find_meta_by_name(
        m_input_metalib, 
        dr_meta_name(pom_grp_entry_meta_normal_meta(main_entry)));
    ASSERT_TRUE(dr_meta);

    m_store = pom_grp_store_create(t_allocrator(), m_meta, dr_meta, t_em());
    ASSERT_TRUE(m_store);
}

LPDRMETALIB OmGrpStoreTest::store_meta(void) {
    EXPECT_TRUE(m_store);
    if (m_store == NULL) return NULL;

    struct mem_buffer metalib_buffer;
    mem_buffer_init(&metalib_buffer, t_tmp_allocrator());
    EXPECT_EQ(0, pom_grp_meta_build_store_meta(&metalib_buffer, pom_grp_store_meta(m_store), t_em()));

    return (LPDRMETALIB)(mem_buffer_make_continuous(&metalib_buffer, 0));
}

const char *
OmGrpStoreTest::str_store_meta(void) {
    LPDRMETALIB metalib = store_meta();
    if (metalib == NULL) return "store-meta is NULL";

    return t_dump_metalib_xml(metalib);
}
