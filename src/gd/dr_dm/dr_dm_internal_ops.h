#ifndef GD_DR_DM_ROLE_INTERNAL_OPS_H
#define GD_DR_DM_ROLE_INTERNAL_OPS_H
#include "dr_dm_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*role manage ops*/
#define dr_dm_manage_is_empty(mgr) \
    (mgr->m_id_index && cpe_hash_table_count(&mgr->m_id_index->m_roles) > 0 ? 0 : 1)

dr_dm_data_t dr_dm_manage_key_buf(dr_dm_manage_t mgr);

/*role index ops*/
struct dr_dm_data_index *
dr_dm_data_index_create(dr_dm_manage_t mgr, LPDRMETAENTRY entry, int is_unique);
void dr_dm_data_index_free(dr_dm_manage_t mgr, struct dr_dm_data_index * index);

int dr_dm_data_index_add(struct dr_dm_data_index * index,  dr_dm_data_t role);
void dr_dm_data_index_remove(struct dr_dm_data_index * index,  dr_dm_data_t role);

const char * dr_dm_data_index_name(struct dr_dm_data_index * index);

uint32_t dr_dm_data_index_hash(const struct dr_dm_data_index * idx);
int dr_dm_data_index_cmp(const struct dr_dm_data_index * l, const struct dr_dm_data_index * r);
void dr_dm_data_index_free_all(dr_dm_manage_t mgr);

#ifdef __cplusplus
}
#endif

#endif
