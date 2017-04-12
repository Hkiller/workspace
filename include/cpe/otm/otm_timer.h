#ifndef CPE_OTM_TIMER_H
#define CPE_OTM_TIMER_H
#include "otm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

otm_timer_t otm_timer_create(
    otm_manage_t mgr,
    otm_timer_id_t id,
    const char * name,
    uint32_t span_s,
    size_t capacity,
    otm_process_fun_t process);

void otm_timer_free(otm_timer_t timer);

otm_timer_t otm_timer_find(otm_manage_t mgr, otm_timer_id_t id);

otm_timer_id_t otm_timer_id(otm_timer_t timer);
const char * otm_timer_name(otm_timer_t timer);
void * otm_timer_data(otm_timer_t timer);
size_t otm_timer_capacity(otm_timer_t timer);

void otm_timer_set_auto_enable(otm_timer_t timer, int enable_p);
int otm_timer_auto_enable(otm_timer_t timer);

void otm_timer_enable(otm_timer_t timer, uint32_t cur_time_s, uint32_t first_exec_span_s, otm_memo_t memo);
void otm_timer_disable(otm_timer_t timer, otm_memo_t memo);

int otm_timer_is_enable(otm_timer_t timer, otm_memo_t memo);

#ifdef __cplusplus
}
#endif

#endif
