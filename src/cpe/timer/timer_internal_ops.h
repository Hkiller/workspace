#ifndef CPE_TIMER_INTERNAL_OPS_H
#define CPE_TIMER_INTERNAL_OPS_H
#include "timer_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t cpe_timer_processor_hash_fun(const struct cpe_timer_processor * o);
int cpe_timer_processor_cmp_fun(const struct cpe_timer_processor * l, const struct cpe_timer_processor * r);
void cpe_timer_mgr_free_processor_buf(cpe_timer_mgr_t mgr);

struct cpe_timer_processor * cpe_timer_processor_get(cpe_timer_mgr_t mgr, cpe_timer_id_t processorId);
int cpe_timer_processor_alloc(cpe_timer_mgr_t mgr, cpe_timer_id_t * id);
void cpe_timer_processor_free(cpe_timer_mgr_t mgr, struct cpe_timer_processor * data);

#ifdef CPE_TIMER_DEBUG
uint32_t cpe_debug_info_hash_fun(const struct cpe_timer_alloc_info * o);
int cpe_debug_info_eq_fun(const struct cpe_timer_alloc_info * l, const struct cpe_timer_alloc_info * r);

void cpe_alloc_info_add(cpe_timer_mgr_t mgr, cpe_timer_id_t id);
void cpe_alloc_info_remove(cpe_timer_mgr_t mgr, cpe_timer_id_t id);
#endif

#ifdef __cplusplus
}
#endif

#endif

