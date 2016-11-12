#include "BuilderTest.hpp"

BuilderTest::BuilderTest() : m_builder(0) {
}

void BuilderTest::SetUp() {
    Base::SetUp();

    m_builder = dr_metalib_builder_create(t_allocrator(), t_em());
}

void BuilderTest::TearDown() {
    if (m_builder) {
        dr_metalib_builder_free(m_builder);
        m_builder = 0;
    }

    Base::TearDown();
}


