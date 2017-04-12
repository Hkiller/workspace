#ifndef CPE_AOM_INTERNAL_TYPES_H
#define CPE_AOM_INTERNAL_TYPES_H
#include "cpe/utils/hash_string.h"
#include "cpe/utils/range.h"
#include "cpe/dr/dr_types.h"
#include "cpe/aom/aom_types.h"

#define AOM_OBJ_CONTROL_MAGIC ((uint16_t)0x48348u)

struct aom_obj_mgr {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    LPDRMETA m_meta;
    uint32_t m_record_size;
    LPDRMETALIB m_metalib;
    char * m_full_base;
    uint32_t m_full_capacity;
    char * m_obj_base;
    uint32_t m_obj_capacity;
    uint32_t m_allocked_obj_count;
    struct cpe_range_mgr m_allocked_objs;
    uint32_t m_free_obj_count;
    struct cpe_range_mgr m_free_objs;
};

extern cpe_hash_string_t aom_control_class_name;

#endif
