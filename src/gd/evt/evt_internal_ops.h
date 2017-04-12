#ifndef GD_EVT_INTERNAL_OPS_H
#define GD_EVT_INTERNAL_OPS_H
#include "evt_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t gd_evt_processor_hash_fun(const struct gd_evt_processor * o);
int gd_evt_processor_cmp_fun(const struct gd_evt_processor * l, const struct gd_evt_processor * r);
void gd_evt_mgr_free_processor_buf(gd_evt_mgr_t mgr);

uint32_t gd_evt_def_hash(const struct gd_evt_def * o);
int gd_evt_def_eq(const struct gd_evt_def * l, const struct gd_evt_def * r);

struct gd_evt_processor * gd_evt_processor_get(gd_evt_mgr_t mgr, evt_processor_id_t processorId);
int gd_evt_processor_alloc(gd_evt_mgr_t mgr, evt_processor_id_t * id);
void gd_evt_processor_free_basic(gd_evt_mgr_t mgr, struct gd_evt_processor * data);
int gd_evt_processor_free_id(gd_evt_mgr_t mgr, evt_processor_id_t processorId);

#ifdef __cplusplus
}
#endif

#endif

