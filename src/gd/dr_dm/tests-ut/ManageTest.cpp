#include "ManageTest.hpp"

ManageTest::ManageTest() : m_manage(NULL) {
}

void ManageTest::SetUp() {
    Base::SetUp();

    t_id_generator_create("test-id-generator");

    m_manage = t_dr_dm_manage_create("test-dr-dm-manage");
}

void ManageTest::setMetaLib(const char * libdef) {
    t_dr_store_reset("test-dr-lib", libdef);
}

void ManageTest::setMeta(const char * name) {
    dr_dm_manage_set_meta(m_manage, t_meta("test-dr-lib", name), NULL);
}
