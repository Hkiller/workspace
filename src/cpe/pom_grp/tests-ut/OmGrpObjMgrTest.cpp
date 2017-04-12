#include "OmGrpObjMgrTest.hpp"

OmGrpObjMgrTest::OmGrpObjMgrTest() : m_mgr(NULL) {
}

void OmGrpObjMgrTest::SetUp() {
    Base::SetUp();
}

void OmGrpObjMgrTest::TearDown() {
    if (m_mgr) {
        pom_grp_obj_mgr_free(m_mgr);
        m_mgr = NULL;
    }

    Base::TearDown();
}

void OmGrpObjMgrTest::install(const char * om_meta, const char * metalib, size_t capacity, uint16_t page_size) {
    if (m_mgr) {
        pom_grp_obj_mgr_free(m_mgr);
        m_mgr = NULL;
    }

    m_mgr = t_pom_grp_obj_mgr_create_by_cfg(om_meta, metalib, capacity, page_size);
}

void OmGrpObjMgrTest::reload(void) {
    void * data = pom_grp_obj_mgr_data(m_mgr);
    uint32_t capacity = pom_grp_obj_mgr_data_capacity(m_mgr);

    pom_grp_obj_mgr_free(m_mgr);

    m_mgr = pom_grp_obj_mgr_create(t_allocrator(), data, capacity, t_em());
}

const char * OmGrpObjMgrTest::pages(pom_grp_obj_t obj) {
    char buf[256];
    size_t write_pos = 0;

    for(uint16_t i = 0; i < pom_grp_obj_page_capacity(m_mgr, obj); ++i) {
        write_pos += snprintf(buf + write_pos, sizeof(buf) - write_pos, "0x%x:", pom_grp_obj_page_oid(m_mgr, obj, i));
    }

    return t_tmp_strdup(buf);
}
