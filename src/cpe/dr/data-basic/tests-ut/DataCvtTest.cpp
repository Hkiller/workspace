#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data_cvt.h"
#include "DataCvtTest.hpp"

void DataCvtTest::cvt(const char * desMetaName, const char * srcMetaName, const char * cfg, size_t capacity) {
    LPDRMETA desMeta = dr_lib_find_meta_by_name(m_metaLib, desMetaName);
    EXPECT_TRUE(desMeta) << "get des meta " << desMetaName << " error!";
    if (desMeta == 0) return;

    LPDRMETA srcMeta = dr_lib_find_meta_by_name(m_metaLib, srcMetaName);
    EXPECT_TRUE(srcMeta) << "get src meta " << srcMetaName << " error!";
    if (srcMeta == 0) return;

    if (capacity == 0) {
        capacity = dr_meta_size(desMeta);
        mem_buffer_clear(&m_buffer);
        void * buf = mem_buffer_alloc(&m_buffer, capacity);
        memset(buf, 0xcc, capacity);
    }

    mem_buffer src_buffer;
    mem_buffer_init(&src_buffer, t_tmp_allocrator());
    mem_buffer_alloc(&src_buffer, dr_meta_size(srcMeta));
    dr_meta_set_defaults(
        mem_buffer_make_continuous(&src_buffer, 0),
        mem_buffer_size(&src_buffer),
        srcMeta,
        0);
    
    t_elist_clear();

    dr_meta_set_defaults(
        mem_buffer_make_continuous(&m_buffer, 0),
        mem_buffer_size(&m_buffer),
        desMeta,
        0);

    dr_data_cvt(
        mem_buffer_make_continuous(&m_buffer, 0),
        mem_buffer_size(&m_buffer),
        desMeta,
        mem_buffer_make_continuous(&src_buffer, 0),
        mem_buffer_size(&src_buffer),
        srcMeta,
        t_cfg_parse(cfg),
        t_em());
}
