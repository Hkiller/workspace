#ifndef CPE_OTM_INTERNAL_OPS_H
#define CPE_OTM_INTERNAL_OPS_H
#include "otm_internal_types.h"

uint32_t otm_timer_hash(const struct otm_timer * timer);
int otm_timer_cmp(const struct otm_timer * l, const struct otm_timer * r);
void otm_timer_free_all(otm_manage_t mgr);

int otm_memo_cmp(void const * l, void const * r);

#endif
