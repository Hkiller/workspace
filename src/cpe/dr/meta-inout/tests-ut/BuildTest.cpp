#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_metalib_init.h"
#include "BuildTest.hpp"

BuildTest::BuildTest() : m_metaLib(0), m_builder(NULL) {
}

void BuildTest::SetUp() {
    Base::SetUp();
    mem_buffer_init(&m_buffer, NULL);

    m_builder = dr_inbuild_create_lib();
}

void BuildTest::TearDown() {
    mem_buffer_clear(&m_buffer);

    if (m_builder) {
        dr_inbuild_create_lib();
    }

    m_metaLib = NULL;

    Base::TearDown();
}

int BuildTest::build() {
    mem_buffer_clear(&m_buffer);
    m_metaLib = NULL;

    t_elist_clear();

    int r = dr_inbuild_build_lib(&m_buffer, m_builder, t_em());
    m_metaLib = (LPDRMETALIB)mem_buffer_make_continuous(&m_buffer, 0);

    return r;
}

LPDRMETA
BuildTest::meta(const char * name) {
    LPDRMETA m = dr_lib_find_meta_by_name(m_metaLib, name);

    EXPECT_TRUE(m != NULL) << "get meta " << name << " fail!";

    return m;
}

LPDRMETAENTRY
BuildTest::entry(const char * metaName, const char * entryName) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, metaName);
    EXPECT_TRUE(meta != NULL)
        << "get meta " << metaName << " fail!";

    LPDRMETAENTRY entry = dr_meta_find_entry_by_path(meta, entryName);
    EXPECT_TRUE(entry != NULL)
        << "get entry " << entryName << " of " << metaName << " fail!";

    return entry;
}

