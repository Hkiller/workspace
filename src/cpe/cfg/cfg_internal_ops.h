#ifndef CPE_CFG_IMPL_INTERNAL_OP_H
#define CPE_CFG_IMPL_INTERNAL_OP_H
#include "cfg_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void cfg_fini(cfg_t cfg);

/*struct operations*/
cfg_t cfg_struct_item_create(struct cfg_struct * s, const char * name, int type, size_t capacity, cfg_policy_t policy);
void cfg_struct_item_delete(struct cfg_struct * s, cfg_t cfg);

void cfg_struct_fini(struct cfg_struct * s);
void cfg_struct_init(struct cfg_struct * s);

#define cfg_to_struct_item(__dp) \
    ((struct cfg_struct_item *)                                      \
     (((char*)__dp) - (sizeof(struct cfg_struct_item) - sizeof(struct cfg))))

RB_PROTOTYPE(cfg_struct_item_tree, cfg_struct_item, m_linkage, cfg_struct_item_cmp);

/*sequence operations*/
cfg_t cfg_seq_item_create(struct cfg_seq * s, int type, size_t capacity);
void cfg_seq_item_delete(struct cfg_seq * s, cfg_t cfg);
void cfg_seq_init(struct cfg_seq * s);
void cfg_seq_fini(struct cfg_seq * s);

cfg_t cfg_check_or_create(cfg_t cfg, const char * path, error_monitor_t em, char * buf, size_t buf_capacity);

#ifdef __cplusplus
}
#endif

#endif
