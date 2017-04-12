#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_metalib_init.h"
#include "BuildFromXmlTest.hpp"

BuildFromXmlTest::BuildFromXmlTest() : m_metaLib(0) {
}

void BuildFromXmlTest::SetUp() {
    Base::SetUp();
    mem_buffer_init(&m_buffer, NULL);
}

void BuildFromXmlTest::TearDown() {
    mem_buffer_clear(&m_buffer);

    m_metaLib = NULL;

    Base::TearDown();
}

int BuildFromXmlTest::parseMeta(const char * def, uint8_t dft_align) {
    mem_buffer_clear(&m_buffer);
    m_metaLib = NULL;

    t_elist_clear();

    int r = dr_create_lib_from_xml_ex(&m_buffer, def, strlen(def), dft_align, t_em());
    m_metaLib = (LPDRMETALIB)mem_buffer_make_continuous(&m_buffer, 0);

    return r;
}

LPDRMETA
BuildFromXmlTest::meta(const char * name) {
    LPDRMETA m = dr_lib_find_meta_by_name(m_metaLib, name);

    EXPECT_TRUE(m != NULL) << "get meta " << name << " fail!";

    return m;
}

LPDRMETAENTRY
BuildFromXmlTest::entry(const char * metaName, const char * entryName) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, metaName);
    EXPECT_TRUE(meta != NULL)
        << "get meta " << metaName << " fail!";

    LPDRMETAENTRY entry = dr_meta_find_entry_by_path(meta, entryName);
    EXPECT_TRUE(entry != NULL)
        << "get entry " << entryName << " of " << metaName << " fail!";

    return entry;
}

int32_t BuildFromXmlTest::address_to_pos(void * p) {
    return (int32_t)((char *)p - (char *)(m_metaLib + 1));
}

