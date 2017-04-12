#include "ReadTest.hpp"
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "cpe/dr/dr_cfg.h"
#include "cpe/utils/stream_buffer.h"

ReadTest::ReadTest() : m_metaLib(0) {
}

void ReadTest::SetUp() {
    Base::SetUp();
    mem_buffer_init(&m_metaLib_buffer, NULL);
}

void ReadTest::TearDown() {
    m_metaLib = NULL;
    mem_buffer_clear(&m_metaLib_buffer);

    Base::TearDown();
}

void ReadTest::installMeta(const char * def) {
    m_metaLib = NULL;
    mem_buffer_clear(&m_metaLib_buffer);

    t_elist_clear();

    EXPECT_EQ(
        0,
        dr_create_lib_from_xml_ex(&m_metaLib_buffer, def, strlen(def), 0, t_em()))
        << "install meta error";

    m_metaLib = (LPDRMETALIB)mem_buffer_make_exactly(&m_metaLib_buffer);
}

int ReadTest::read(const char * encodeTypeName, const char * decodeTypeName, const char * defs) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, encodeTypeName);
    EXPECT_TRUE(meta) << "get meta " << encodeTypeName << " error!";

    char buf[1024];

    int r = dr_cfg_read(buf, sizeof(buf), t_cfg_parse(defs), meta, 0, t_em());
    EXPECT_GT(r, 0);

    return read(decodeTypeName, buf, r);
}

int ReadTest::read(const char * decodeTypeName, const void * data, size_t data_size) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, decodeTypeName);
    EXPECT_TRUE(meta) << "get meta " << decodeTypeName << " error!";

    t_elist_clear();

    char buf[1024];
    int len = dr_pbuf_write(buf, sizeof(buf), data, data_size, meta, t_em());
    EXPECT_GE(len, 0);

    bzero(m_result_buffer, sizeof(m_result_buffer));

    int r = dr_pbuf_read(m_result_buffer, sizeof(m_result_buffer), buf, len, meta, t_em());
    m_result_bufffer_len = r;
    m_result_meta = meta;
    return r;
}

cfg_t ReadTest::result(void) {
    EXPECT_TRUE(m_result_meta) << "result meta not exist";

    cfg_t r = t_cfg_create();

    EXPECT_EQ(
        0,
        dr_cfg_write(
            r,
            m_result_buffer,
            m_result_meta,
            t_em()));

    return r;
}

int ReadTest::metaSize(const char * typeName) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, typeName);
    EXPECT_TRUE(meta) << "get meta " << typeName << " error!";

    if (meta == NULL) return -1;

    return (int)dr_meta_size(meta);
}
