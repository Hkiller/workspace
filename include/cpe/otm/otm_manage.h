#ifndef CPE_OTM_MANAGER_H
#define CPE_OTM_MANAGER_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "otm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

otm_manage_t
otm_manage_create(
    mem_allocrator_t alloc,
    error_monitor_t em);

void otm_manage_free(otm_manage_t mgr);

int otm_manage_buf_init(otm_manage_t mgr, uint32_t cur_time_s, otm_memo_t memo, size_t memo_capacitiy);

void otm_manage_tick(otm_manage_t mgr, uint32_t cur_time_s, void * obj_ctx, otm_memo_t memo_buf, size_t memo_capacity);

int otm_manage_enable(otm_manage_t mgr, otm_timer_id_t id, uint32_t cur_time_s, uint32_t first_exec_span_s, otm_memo_t memo_buf, size_t memo_capacitiy);
int otm_manage_disable(otm_manage_t mgr, otm_timer_id_t id, otm_memo_t memo_buf, size_t memo_capacitiy);
int otm_manage_perform(otm_manage_t mgr, uint32_t cur_time_s,  otm_timer_id_t id, void * obj_ctx, otm_memo_t memo_buf, size_t memo_capacitiy);

error_monitor_t otm_manage_em(otm_manage_t mgr);
void otm_manage_timers(otm_manage_t mgr, otm_timer_it_t it);

#define otm_timer_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
