#include "DataBasicTest.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_xml.h"

DataBasicTest::DataBasicTest() : m_metaLib(0) {
}

void DataBasicTest::SetUp() {
    Base::SetUp();
    mem_buffer_init(&m_metaLib_buffer, NULL);
    mem_buffer_init(&m_buffer, NULL);
}

void DataBasicTest::TearDown() {
    mem_buffer_clear(&m_buffer);

    m_metaLib = NULL;
    mem_buffer_clear(&m_metaLib_buffer);
    Base::TearDown();
}

void DataBasicTest::installMeta(const char * def) {
    m_metaLib = NULL;
    mem_buffer_init(&m_metaLib_buffer, NULL);

    EXPECT_EQ(
        0,
        dr_create_lib_from_xml_ex(&m_metaLib_buffer, def, strlen(def), 0, t_em()))
        << "install meta error";

    m_metaLib = (LPDRMETALIB)mem_buffer_make_exactly(&m_metaLib_buffer);
}

void * DataBasicTest::result(int startPos) {
    return ((char *)mem_buffer_make_exactly(&m_buffer)) + startPos;
}
