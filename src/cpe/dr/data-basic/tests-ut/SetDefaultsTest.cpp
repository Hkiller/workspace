#include "cpe/dr/dr_metalib_manage.h"
#include "SetDefaultsTest.hpp"

void SetDefaultsTest::set_defaults(const char * metaName, int policy, size_t capacity) {
    LPDRMETA meta = dr_lib_find_meta_by_name(m_metaLib, metaName);
    EXPECT_TRUE(meta) << "get meta " << metaName << " error!";
    if (meta == 0) return;

    if (capacity == 0) {
        capacity = dr_meta_size(meta);
        mem_buffer_clear(&m_buffer);
        void * buf = mem_buffer_alloc(&m_buffer, capacity);
        memset(buf, 0xcc, capacity);
    }

    t_elist_clear();
    dr_meta_set_defaults(
        mem_buffer_make_continuous(&m_buffer, 0),
        mem_buffer_size(&m_buffer),
        meta,
        policy);
}
