#include "ErrorTest.hpp"

ErrorTest::ErrorTest() : m_el_1(NULL), m_el_2(NULL) {
}

void ErrorTest::SetUp() {
    m_el_1 = cpe_error_list_create(NULL);
    m_el_2 = cpe_error_list_create(NULL);
}

void ErrorTest::TearDown() {
    cpe_error_list_free(m_el_1);
    cpe_error_list_free(m_el_2);
    m_el_1 = m_el_2 = NULL;
}

