#ifndef GD_DR_DM_ROLE_INTERNAL_TYPES_H
#define GD_DR_DM_ROLE_INTERNAL_TYPES_H
#include "cpe/utils/hash.h"
#include "gd/dr_store/dr_store_types.h"
#include "gd/dr_dm/dr_dm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct dr_dm_data_index;

struct dr_dm_manage {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;

    int m_debug;

    dr_ref_t m_metalib;
    LPDRMETA m_role_meta;
    gd_id_generator_t m_id_generate;

    dr_dm_data_t m_key_buf;
    struct dr_dm_data_index * m_id_index;
    struct cpe_hash_table m_indexes;
};

struct dr_dm_data {
    dr_dm_manage_t m_mgr; 
};

struct dr_dm_data_index {
    int m_id;
    const char * m_name;
    LPDRMETAENTRY m_entry;
    struct cpe_hash_table m_roles;
    int (*m_insert_fun)(cpe_hash_table_t hstable, void * obj);

    struct cpe_hash_entry m_hh;
};

#ifdef __cplusplus
}
#endif

#endif
