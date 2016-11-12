#include "ParseTest.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_xml.h"

ParseTest::ParseTest() : m_metaLib(0) {
}

void ParseTest::SetUp() {
    Base::SetUp();
    mem_buffer_init(&m_metaLib_buffer, NULL);
    mem_buffer_init(&m_buffer, NULL);
}

void ParseTest::TearDown() {
    mem_buffer_clear(&m_buffer);

    m_metaLib = NULL;
    mem_buffer_clear(&m_metaLib_buffer);
    Base::TearDown();
}

void ParseTest::installMeta(const char * def) {
    m_metaLib = NULL;
    mem_buffer_init(&m_metaLib_buffer, NULL);

    EXPECT_EQ(
        0,
        dr_create_lib_from_xml_ex(&m_metaLib_buffer, def, strlen(def), 0, t_em()))
        << "install meta error";

    m_metaLib = (LPDRMETALIB)mem_buffer_make_exactly(&m_metaLib_buffer);
}

int ParseTest::metaSize(const char * typeName) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, typeName);
    EXPECT_TRUE(meta) << "get meta " << typeName << " error!";

    if (meta == NULL) return -1;

    return (int)dr_meta_size(meta);
}

int ParseTest::read(const char * data, const char * typeName) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, typeName);
    EXPECT_TRUE(meta) << "get meta " << typeName << " error!";

    t_elist_clear();

    return dr_xml_read_to_buffer(&m_buffer, data, meta, t_em());
}

void * ParseTest::result(int startPos) {
    return ((char *)mem_buffer_make_exactly(&m_buffer)) + startPos;
}
