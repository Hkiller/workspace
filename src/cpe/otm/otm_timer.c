#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/otm/otm_timer.h"
#include "otm_internal_ops.h"

uint32_t otm_timer_hash(const struct otm_timer * context) {
    return (uint32_t)context->m_id;
}

int otm_timer_cmp(const struct otm_timer * l, const struct otm_timer * r) {
    return l->m_id == r->m_id;
}

otm_timer_t otm_timer_create(
    otm_manage_t mgr,
    otm_timer_id_t id,
    const char * name,
    uint32_t span_s,
    size_t capacity,
    otm_process_fun_t process)
{
    otm_timer_t timer;
    char * buf;
    size_t name_len = strlen(name) + 1;

    buf = (char *)mem_alloc(mgr->m_alloc, name_len + sizeof(struct otm_timer) + capacity);
    if (buf == NULL) return NULL;

    memcpy(buf, name, name_len);

    timer = (otm_timer_t)(buf + name_len);

    timer->m_mgr = mgr;
    timer->m_id = id;
    timer->m_name = buf;
    timer->m_span_s = span_s;
    timer->m_capacity = capacity;
    timer->m_process = process;
    timer->m_flags = 0;

    cpe_hash_entry_init(&timer->m_hh);
    if (cpe_hash_table_insert_unique(&mgr->m_timers, timer) != 0) {
        mem_free(mgr->m_alloc, buf);
        return NULL;
    }

    return timer;
}

void otm_timer_free(otm_timer_t timer) {
    otm_manage_t mgr;

    assert(timer);

    mgr = timer->m_mgr;

    cpe_hash_table_remove_by_ins(&mgr->m_timers, timer);

    mem_free(mgr->m_alloc, (void*)timer->m_name);
}


void otm_timer_free_all(otm_manage_t mgr) {
    struct cpe_hash_it timer_it;
    otm_timer_t timer;

    cpe_hash_it_init(&timer_it, &mgr->m_timers);

    timer = cpe_hash_it_next(&timer_it);
    while (timer) {
        otm_timer_t next = cpe_hash_it_next(&timer_it);
        otm_timer_free(timer);
        timer = next;
    }
}

otm_timer_t otm_timer_find(otm_manage_t mgr, otm_timer_id_t id) {
    struct otm_timer key;
    key.m_id = id;
    return (otm_timer_t)cpe_hash_table_find(&mgr->m_timers, &key);
}

otm_timer_id_t otm_timer_id(otm_timer_t timer) {
    return timer->m_id;
}

const char * otm_timer_name(otm_timer_t timer) {
    return timer->m_name;
}

void * otm_timer_data(otm_timer_t timer) {
    return timer + 1;
}

size_t otm_timer_capacity(otm_timer_t timer) {
    return timer->m_capacity;
}

void otm_timer_set_auto_enable(otm_timer_t timer, int enable_p) {
    if (enable_p) {
        timer->m_flags &= OTM_TIMER_FLAGS_AUTO_ENABLE;
    }
    else {
        timer->m_flags |= ~OTM_TIMER_FLAGS_AUTO_ENABLE;
    }
}

int otm_timer_auto_enable(otm_timer_t timer) {
    return timer->m_flags | OTM_TIMER_FLAGS_AUTO_ENABLE;
}

void otm_timer_enable(otm_timer_t timer, uint32_t cur_time_s, uint32_t first_exec_span_s, otm_memo_t memo) {
    if (memo->m_last_action_time_s != 0) {
        if (first_exec_span_s != 0) {
            memo->m_next_action_time_s = cur_time_s + first_exec_span_s;
        }
    }
    else {
        if (first_exec_span_s == 0) first_exec_span_s = timer->m_span_s;

        memo->m_last_action_time_s = cur_time_s;
        memo->m_next_action_time_s = cur_time_s + first_exec_span_s;
    }
}

void otm_timer_disable(otm_timer_t timer, otm_memo_t memo) {
    memo->m_last_action_time_s = 0;
    memo->m_next_action_time_s = 0;
}

int otm_timer_is_enable(otm_timer_t timer, otm_memo_t memo)
{
	return memo->m_last_action_time_s != 0 || memo->m_next_action_time_s != 0;
}
