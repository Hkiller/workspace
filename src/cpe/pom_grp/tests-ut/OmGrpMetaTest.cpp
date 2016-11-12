#include "OmGrpMetaTest.hpp"

OmGrpMetaTest::OmGrpMetaTest() : m_meta(NULL) {
}

void OmGrpMetaTest::SetUp() {
    Base::SetUp();
}

void OmGrpMetaTest::TearDown() {
    if (m_meta) {
        pom_grp_meta_free(m_meta);
        m_meta = NULL;
    }

    Base::TearDown();
}

void OmGrpMetaTest::installFromCfg(const char * om_meta, const char * metalib, uint16_t page_size) {
    if (m_meta) {
        pom_grp_meta_free(m_meta);
        m_meta = NULL;
    }

    m_meta = t_pom_grp_meta_create_by_cfg(om_meta, metalib, page_size);
}

void OmGrpMetaTest::installFromMeta(const char * metalib, const char * meta_name, uint16_t page_size) {
    if (m_meta) {
        pom_grp_meta_free(m_meta);
        m_meta = NULL;
    }

    m_meta = t_pom_grp_meta_create_by_meta(metalib, meta_name, page_size);
}
