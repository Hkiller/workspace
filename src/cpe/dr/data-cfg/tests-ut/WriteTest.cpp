#include "WriteTest.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_xml.h"

WriteTest::WriteTest() : m_metaLib(0), m_cfg(0) {
}

void WriteTest::SetUp() {
    Base::SetUp();
    mem_buffer_init(&m_metaLib_buffer, NULL);
    m_cfg = cfg_create(t_tmp_allocrator());
}

void WriteTest::TearDown() {
    m_metaLib = NULL;
    mem_buffer_clear(&m_metaLib_buffer);
    cfg_free(m_cfg);
    m_cfg = 0;
    Base::TearDown();
}

void WriteTest::installMeta(const char * def) {
    m_metaLib = NULL;
    mem_buffer_init(&m_metaLib_buffer, NULL);

    EXPECT_EQ(
        0,
        dr_create_lib_from_xml_ex(&m_metaLib_buffer, def, strlen(def), 0, t_em()))
        << "install meta error";

    m_metaLib = (LPDRMETALIB)mem_buffer_make_exactly(&m_metaLib_buffer);
}

int WriteTest::write(const char * data, const char * typeName) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, typeName);
    EXPECT_TRUE(meta) << "get meta " << typeName << " error!";
    if (meta == 0) return -1;

    size_t capacity = 1024;

    cfg_t input = t_cfg_parse(data);
    EXPECT_TRUE(input) << "parse input to cfg fail!";
    if (input == 0) return -1;

    struct mem_buffer m_src_buffer;
    mem_buffer_init(&m_src_buffer, t_tmp_allocrator());
    mem_buffer_alloc(&m_src_buffer, capacity);

    dr_cfg_read(
        mem_buffer_make_continuous(&m_src_buffer, 0),
        capacity,
        input,
        meta,
        0,
        t_em());

    return dr_cfg_write(
        m_cfg,
        mem_buffer_make_continuous(&m_src_buffer, 0),
        meta, t_em());
}
