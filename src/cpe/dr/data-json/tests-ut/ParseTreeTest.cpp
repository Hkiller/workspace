#include "ParseTreeTest.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_xml.h"

ParseTreeTest::ParseTreeTest() : m_metaLib(0) {
}

void ParseTreeTest::SetUp() {
    Base::SetUp();
    mem_buffer_init(&m_metaLib_buffer, NULL);
    mem_buffer_init(&m_buffer, NULL);
}

void ParseTreeTest::TearDown() {
    mem_buffer_clear(&m_buffer);

    m_metaLib = NULL;
    mem_buffer_clear(&m_metaLib_buffer);
    Base::TearDown();
}

void ParseTreeTest::installMeta(const char * def) {
    m_metaLib = NULL;
    mem_buffer_init(&m_metaLib_buffer, NULL);

    EXPECT_EQ(
        0,
        dr_create_lib_from_xml_ex(&m_metaLib_buffer, def, strlen(def), 0, t_em()))
        << "install meta error";

    m_metaLib = (LPDRMETALIB)mem_buffer_make_exactly(&m_metaLib_buffer);
}

int ParseTreeTest::metaSize(const char * typeName) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, typeName);
    EXPECT_TRUE(meta) << "get meta " << typeName << " error!";

    if (meta == NULL) return -1;

    return (int)dr_meta_size(meta);
}

int ParseTreeTest::read(const char * data, const char * typeName) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, typeName);
    EXPECT_TRUE(meta) << "get meta " << typeName << " error!";

    t_elist_clear();

    char error_buf[128];
    yajl_val data_tree  = yajl_tree_parse(data, error_buf, sizeof(error_buf));
    EXPECT_TRUE(data_tree) << "parse tree fail: %s" << error_buf;
    if (data_tree == NULL) return -1;

    size_t capacity = dr_meta_size(meta) * 20;
    mem_buffer_clear_data(&m_buffer);

    int rv = dr_json_tree_read(
        mem_buffer_make_continuous(&m_buffer, capacity),
        capacity,
        data_tree, meta, t_em());

    yajl_tree_free(data_tree);
    if (rv >= 0) {
        mem_buffer_set_size(&m_buffer, (size_t)rv);
    }
    
    return rv;
}

void * ParseTreeTest::result(int startPos) {
    return ((char *)mem_buffer_make_exactly(&m_buffer)) + startPos;
}
