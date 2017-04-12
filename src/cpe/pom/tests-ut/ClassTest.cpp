#include "ClassTest.hpp"

void ClassTest::SetUp() {
    Base::SetUp();
    ASSERT_EQ(0, pom_class_mgr_init(&m_classMgr, t_allocrator()));
}

void ClassTest::TearDown() {
    pom_class_mgr_fini(&m_classMgr);
    Base::TearDown();
}

pom_class_id_t
ClassTest::addClass(const char * className, size_t object_size, size_t page_size, size_t align) {
    return pom_class_add(&m_classMgr, className, object_size, page_size, align, t_em());
}



