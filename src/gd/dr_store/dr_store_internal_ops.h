#ifndef GD_DR_STORE_INTERNAL_OPS_H
#define GD_DR_STORE_INTERNAL_OPS_H
#include "dr_store_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*dr_store ops*/
uint32_t dr_store_hash(const struct dr_store * context);
int dr_store_cmp(const struct dr_store * l, const struct dr_store * r);
void dr_store_free_all(dr_store_manage_t mgr);
void dr_store_add_ref(dr_store_t store);
void dr_store_remove_ref(dr_store_t store);

/*dr ref operations*/
void dr_ref_free_all(dr_store_manage_t mgr);

#ifdef __cplusplus
}
#endif

#endif
