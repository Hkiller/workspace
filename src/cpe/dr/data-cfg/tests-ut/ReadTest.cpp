#include "ReadTest.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_xml.h"

ReadTest::ReadTest() : m_metaLib(0) {
}

void ReadTest::SetUp() {
    Base::SetUp();
    mem_buffer_init(&m_metaLib_buffer, NULL);
    mem_buffer_init(&m_buffer, NULL);
}

void ReadTest::TearDown() {
    mem_buffer_clear(&m_buffer);

    m_metaLib = NULL;
    mem_buffer_clear(&m_metaLib_buffer);
    Base::TearDown();
}

void ReadTest::installMeta(const char * def) {
    m_metaLib = NULL;
    mem_buffer_init(&m_metaLib_buffer, NULL);

    EXPECT_EQ(
        0,
        dr_create_lib_from_xml_ex(&m_metaLib_buffer, def, strlen(def), 0, t_em()))
        << "install meta error";

    m_metaLib = (LPDRMETALIB)mem_buffer_make_exactly(&m_metaLib_buffer);
}

int ReadTest::read(const char * data, const char * typeName, int policy, size_t capacity) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, typeName);
    EXPECT_TRUE(meta) << "get meta " << typeName << " error!";
    if (meta == 0) return -1;

    if (capacity == 0) {
        capacity = dr_meta_size(meta);
    }

    cfg_t input = t_cfg_parse(data);
    EXPECT_TRUE(input) << "parse input to cfg fail!";
    if (input == 0) return -1;

    t_elist_clear();
    mem_buffer_clear(&m_buffer);

    return dr_cfg_read(
        mem_buffer_alloc(&m_buffer, capacity),
        capacity,
        input,
        meta,
        policy,
        t_em());
}

void * ReadTest::result(int startPos) {
    return ((char *)mem_buffer_make_exactly(&m_buffer)) + startPos;
}
