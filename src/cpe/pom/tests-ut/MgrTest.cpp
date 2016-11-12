#include "cpe/pom/pom_debuger.h"
#include "MgrTest.hpp"

MgrTest::MgrTest() : m_omm(NULL) {
}

void MgrTest::SetUp() {
    Base::SetUp();

    m_omm = pom_mgr_create(t_allocrator(), 256, 1024);
    ASSERT_TRUE(m_omm);

    pom_debuger_enable(m_omm, 5, t_em());

    pom_mgr_set_backend_memory(m_omm, t_allocrator());

    pom_mgr_set_auto_validate(m_omm, 1);
}

void MgrTest::TearDown() {
    if (m_omm) {
        pom_mgr_free(m_omm);
        m_omm = NULL;
    }

    Base::TearDown();
}

pom_buffer_id_t
MgrTest::buf_alloc(size_t size, void * context) {
    MgrTest * t = reinterpret_cast<MgrTest *>(context);
    return (pom_buffer_id_t)mem_alloc(t->t_allocrator(), size);
}

pom_class_id_t
MgrTest::addClass(const char * className, size_t object_size) {
    return pom_mgr_add_class(m_omm, className, object_size, 4, t_em());
}

pom_oid_t
MgrTest::obj_alloc(cpe_hash_string_t className) {
    return pom_obj_alloc(m_omm, className, t_em());
}



