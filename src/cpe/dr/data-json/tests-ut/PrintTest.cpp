#include "PrintTest.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "cpe/utils/stream_buffer.h"

PrintTest::PrintTest() : m_metaLib(0) {
}

void PrintTest::SetUp() {
    Base::SetUp();
    mem_buffer_init(&m_metaLib_buffer, NULL);
    mem_buffer_init(&m_buffer, NULL);
}

void PrintTest::TearDown() {
    mem_buffer_clear(&m_buffer);

    m_metaLib = NULL;
    mem_buffer_clear(&m_metaLib_buffer);

    Base::TearDown();
}

void PrintTest::installMeta(const char * def) {
    m_metaLib = NULL;
    mem_buffer_clear(&m_metaLib_buffer);

    t_elist_clear();

    EXPECT_EQ(
        0,
        dr_create_lib_from_xml_ex(&m_metaLib_buffer, def, strlen(def), 0, t_em()))
        << "install meta error";

    m_metaLib = (LPDRMETALIB)mem_buffer_make_exactly(&m_metaLib_buffer);
}

int PrintTest::print(const void * data, size_t size, const char * typeName) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, typeName);
    EXPECT_TRUE(meta) << "get meta " << typeName << " error!";

    t_elist_clear();

    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&m_buffer);

    int r = dr_json_print((write_stream_t)&stream, data, size, meta, DR_JSON_PRINT_MINIMIZE, t_em());
    stream_putc((write_stream_t)&stream, 0);
    return r;
}

int PrintTest::print_array(const void * data, size_t size, const char * typeName) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, typeName);
    EXPECT_TRUE(meta) << "get meta " << typeName << " error!";

    t_elist_clear();

    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&m_buffer);

    int r = dr_json_print_array((write_stream_t)&stream, data, size, meta, DR_JSON_PRINT_MINIMIZE, t_em());
    stream_putc((write_stream_t)&stream, 0);
    return r;
}

const char * PrintTest::result(void) {
    return (const char *)mem_buffer_make_exactly(&m_buffer);
}
