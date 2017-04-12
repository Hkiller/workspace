#include "cpe/utils/string_utils.h"
#include "usf/logic/logic_require.h"
#include "payment_svr_waiting.h"

payment_svr_waiting_t
payment_svr_waiting_create(payment_svr_t svr, const char * trans_id) {
    payment_svr_waiting_t waiting;

    waiting = mem_alloc(svr->m_alloc, sizeof(struct payment_svr_waiting));
    if (waiting == NULL) {
        CPE_ERROR(svr->m_em, "payment_svr_waiting_create: alloc fail!");
        return NULL;
    }

    waiting->m_trade_id = waiting->m_trade_id_buf;
    cpe_str_dup(waiting->m_trade_id_buf, sizeof(waiting->m_trade_id_buf), trans_id);
    waiting->m_require_id = 0;
    
    cpe_hash_entry_init(&waiting->m_hh);
    if (cpe_hash_table_insert_unique(&svr->m_waitings, waiting) != 0) {
        mem_free(svr->m_alloc, waiting);
        return NULL;
    }

    return waiting;
}

void payment_svr_waiting_free(payment_svr_t svr, payment_svr_waiting_t waiting) {
    cpe_hash_table_remove_by_ins(&svr->m_waitings, waiting);
    mem_free(svr->m_alloc, waiting);
}

void payment_svr_waiting_free_all(payment_svr_t svr) {
    struct cpe_hash_it waiting_it;
    payment_svr_waiting_t waiting;

    cpe_hash_it_init(&waiting_it, &svr->m_waitings);
    waiting = cpe_hash_it_next(&waiting_it);
    while (waiting) {
        payment_svr_waiting_t next = cpe_hash_it_next(&waiting_it);
        payment_svr_waiting_free(svr, waiting);
        waiting = next;
    }
}

payment_svr_waiting_t
payment_svr_waiting_find(payment_svr_t svr, const char * trade_id) {
    struct payment_svr_waiting key;
    key.m_trade_id = trade_id;
    return (payment_svr_waiting_t)cpe_hash_table_find(&svr->m_waitings, &key);
}

int payment_svr_waiting_start(payment_svr_t svr, const char * trans_id, logic_require_t require) {
    payment_svr_waiting_t waiting;

    waiting = payment_svr_waiting_find(svr, trans_id);
    if (waiting == NULL) {
        waiting = payment_svr_waiting_create(svr, trans_id);
        if (waiting == NULL) return -1;
    }

    if (waiting->m_require_id) {
        logic_require_t old_require = logic_require_find(logic_require_mgr(require), waiting->m_require_id);
        if (old_require) logic_require_cancel(old_require);
    }

    waiting->m_require_id = logic_require_id(require);
    
    return 0;
}

void payment_svr_waiting_stop(payment_svr_t svr, const char * trans_id, logic_require_t require) {
    payment_svr_waiting_t waiting;

    waiting = payment_svr_waiting_find(svr, trans_id);
    if (waiting == NULL) return;

    if (waiting->m_require_id == logic_require_id(require)) {
        payment_svr_waiting_free(svr, waiting);
    }
}

uint32_t payment_svr_waiting_hash(const payment_svr_waiting_t waiting) {
    return cpe_hash_str(waiting->m_trade_id, strlen(waiting->m_trade_id));
}

int payment_svr_waiting_eq(const payment_svr_waiting_t l, const payment_svr_waiting_t r) {
    return strcmp(l->m_trade_id, r->m_trade_id) == 0;
}

