#include "WriteTest.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "cpe/dr/dr_cfg.h"
#include "cpe/utils/stream_buffer.h"

WriteTest::WriteTest() : m_metaLib(0) {
}

void WriteTest::SetUp() {
    Base::SetUp();
    mem_buffer_init(&m_metaLib_buffer, NULL);
}

void WriteTest::TearDown() {
    m_metaLib = NULL;
    mem_buffer_clear(&m_metaLib_buffer);

    Base::TearDown();
}

void WriteTest::installMeta(const char * def) {
    m_metaLib = NULL;
    mem_buffer_clear(&m_metaLib_buffer);

    t_elist_clear();

    EXPECT_EQ(
        0,
        dr_create_lib_from_xml_ex(&m_metaLib_buffer, def, strlen(def), 0, t_em()))
        << "install meta error";

    m_metaLib = (LPDRMETALIB)mem_buffer_make_exactly(&m_metaLib_buffer);
}

int WriteTest::write(const char * typeName, const char * defs, uint8_t is_open) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, typeName);
    EXPECT_TRUE(meta) << "get meta " << typeName << " error!";

    char buf[1024];

    int r = dr_cfg_read(buf, sizeof(buf), t_cfg_parse(defs), meta, 0, t_em());
    EXPECT_GT(r, 0);

    return write(typeName, buf, r, is_open);
}

int WriteTest::write(const char * typeName, const void * data, size_t data_size, uint8_t is_open) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, typeName);
    EXPECT_TRUE(meta) << "get meta " << typeName << " error!";

    t_elist_clear();

    int r = dr_bson_write(m_buffer, sizeof(m_buffer), is_open, data, data_size, meta, t_em());
    m_bufffer_len = r >= 0 ? (size_t)r : 0;
    return r;
}

const char * WriteTest::result(void) {
    return t_tmp_hexdup(m_buffer, m_bufffer_len);
}
