#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/stream_error.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/tl/tl_action.h"
#include "cpe/timer/timer_manage.h"
#include "timer_internal_ops.h"

static void cpe_timer_mgr_dispatch_timer(tl_event_t input, void * context);
static void cpe_timer_mgr_destory_timer(tl_event_t event, void * context);

cpe_timer_mgr_t
cpe_timer_mgr_create(tl_manage_t tl_mgr, mem_allocrator_t alloc, error_monitor_t em) {
    cpe_timer_mgr_t mgr;

    mgr = mem_alloc(alloc, sizeof(struct cpe_timer_mgr));
    if (mgr == NULL) {
        CPE_ERROR(em, "cpe_timer_mgr_create: alloc fail!");
        return NULL;
    }

    mgr->m_alloc = alloc;
    mgr->m_em = em;
    mgr->m_debug = 0;
    mgr->m_timer_count_in_page = 2048;
    mgr->m_timer_page_count = 0;
    mgr->m_timer_page_capacity = 0;
    mgr->m_timer_buf = NULL;

    if (cpe_range_mgr_init(&mgr->m_ids, alloc) != 0) {
        CPE_ERROR(em, "cpe_timer_mgr_create: init range mgr!");
        mem_free(alloc, mgr);
        return NULL;
    }

    mgr->m_tl = tl_create(tl_mgr);
    if (mgr->m_tl == NULL) {
        CPE_ERROR(em, "cpe_timer_mgr_create: create tl fail!");
        cpe_range_mgr_fini(&mgr->m_ids);
        mem_free(alloc, mgr);
        return NULL;
    }

    tl_set_opt(mgr->m_tl, tl_set_event_dispatcher, cpe_timer_mgr_dispatch_timer);
    tl_set_opt(mgr->m_tl, tl_set_event_op_context, mgr);
    tl_set_opt(mgr->m_tl, tl_set_event_destory, cpe_timer_mgr_destory_timer);

    if (cpe_hash_table_init(
            &mgr->m_responser_to_processor,
            alloc,
            (cpe_hash_fun_t) cpe_timer_processor_hash_fun,
            (cpe_hash_eq_t) cpe_timer_processor_cmp_fun,
            CPE_HASH_OBJ2ENTRY(cpe_timer_processor, m_hh_for_responser_to_processor),
            -1) != 0)
    {
        CPE_ERROR(em, "cpe_timer_mgr_create: init responser hash table fail!");
        tl_free(mgr->m_tl);
        cpe_range_mgr_fini(&mgr->m_ids);
        mem_free(alloc, mgr);
        return NULL;
    }

#ifdef CPE_TIMER_DEBUG
    cpe_range_set_debug(&mgr->m_ids, 1);

    if (cpe_hash_table_init(
            &mgr->m_alloc_infos,
            alloc,
            (cpe_hash_fun_t) cpe_debug_info_hash_fun,
            (cpe_hash_eq_t) cpe_debug_info_eq_fun,
            CPE_HASH_OBJ2ENTRY(cpe_timer_alloc_info, m_hh),
            -1) != 0)
    {
        CPE_ERROR(em, "cpe_timer_mgr_create: init debug info hash table fail!");
        tl_free(mgr->m_tl);
        cpe_range_mgr_fini(&mgr->m_ids);
        cpe_hash_table_fini(&mgr->m_responser_to_processor);
        mem_free(alloc, mgr);
        return NULL;
    }

#endif

    return mgr;
}

void cpe_timer_mgr_free(cpe_timer_mgr_t mgr) {

    tl_free(mgr->m_tl);

    cpe_timer_mgr_free_processor_buf(mgr);

    cpe_range_mgr_fini(&mgr->m_ids);

    cpe_hash_table_fini(&mgr->m_responser_to_processor);

#ifdef CPE_TIMER_DEBUG
    do {
        struct cpe_hash_it it;
        struct cpe_timer_alloc_info * e;

        cpe_hash_it_init(&it, &mgr->m_alloc_infos);

        e = cpe_hash_it_next(&it);
        while (e) {
            struct cpe_timer_alloc_info * next = cpe_hash_it_next(&it);
            mem_free(mgr->m_alloc, e);
            e = next;
        }

        cpe_hash_table_fini(&mgr->m_alloc_infos);
    } while(0);
#endif

    mem_free(mgr->m_alloc, mgr);
}

tl_t cpe_timer_mgr_tl(cpe_timer_mgr_t mgr) {
    return mgr->m_tl;
}

int cpe_timer_mgr_regist_timer(
    cpe_timer_mgr_t mgr, 
    cpe_timer_id_t * id,
    cpe_timer_process_fun_t fun, void * ctx,
    void * arg, void (*arg_fini)(void *),
    tl_time_span_t delay, tl_time_span_t span, int repeatCount)
{
    cpe_timer_id_t newProcessorId;
    struct cpe_timer_processor * newProcessorData;
    int send_rv;

    if (cpe_timer_processor_alloc(mgr, &newProcessorId) != 0) {
        if (arg && arg_fini) arg_fini(arg);
        CPE_ERROR(mgr->m_em, "cpe_timer_mgr: regist processor: alloc processor fail!");
        return -1;
    }

    newProcessorData = cpe_timer_processor_get(mgr, newProcessorId);
    assert(newProcessorData);
    assert(newProcessorData->m_process_ctx == NULL);
    assert(newProcessorData->m_state == timer_processor_state_NotInResponserHash);

    newProcessorData->m_process_ctx = ctx;
    newProcessorData->m_process_arg = arg;
    newProcessorData->m_process_arg_free = arg_fini;
    newProcessorData->m_process_fun = fun;

    newProcessorData->m_tl_event = tl_event_create(mgr->m_tl, sizeof(cpe_timer_id_t));
    if (newProcessorData->m_tl_event == NULL) {
        cpe_timer_processor_free(mgr, newProcessorData);
        CPE_ERROR(mgr->m_em, "cpe_timer_mgr: regist processor: create tl_event fail!");
        return -1;
    }
    *(cpe_timer_id_t*)tl_event_data(newProcessorData->m_tl_event) = newProcessorId;

    if (cpe_hash_table_insert(&mgr->m_responser_to_processor, newProcessorData) != 0) {
        cpe_timer_processor_free(mgr, newProcessorData);
        CPE_ERROR(mgr->m_em, "cpe_timer_mgr: regist processor: insert to responser processor list fail!");
        return -1;
    }
    newProcessorData->m_state = timer_processor_state_InResponserHash;

    send_rv = tl_event_send_ex(newProcessorData->m_tl_event, delay, span, repeatCount);
    if (send_rv != 0) {
        cpe_timer_processor_free(mgr, newProcessorData);
        CPE_ERROR(mgr->m_em, "cpe_timer_mgr: regist processor: send event to tl fail, rv=%d!", send_rv);
        return -1;
    }

    if (id) *id = newProcessorId;

    return 0;
}

void cpe_timer_mgr_unregist_timer_by_ctx(cpe_timer_mgr_t mgr, void * ctx) {
    struct cpe_timer_processor key;
    struct cpe_timer_processor * node;

    key.m_process_ctx = ctx;

    node = (struct cpe_timer_processor *)cpe_hash_table_find(&mgr->m_responser_to_processor, &key);
    while(node) {
        struct cpe_timer_processor * next = 
            cpe_hash_table_find_next(&mgr->m_responser_to_processor, node);
        assert(node->m_process_ctx);

        cpe_timer_processor_free(mgr, node);

        node = next;
    }
}

void cpe_timer_mgr_unregist_timer_by_id(cpe_timer_mgr_t mgr, cpe_timer_id_t timer_id) {
    struct cpe_timer_processor * timer;
    timer = cpe_timer_processor_get(mgr, timer_id);

    if (timer == NULL) {
        CPE_ERROR(mgr->m_em, "cpe_timer_mgr: unregister timer by id: %d not a valid timer!", timer_id);
    }
    else if (timer->m_process_ctx == NULL) {
        CPE_ERROR(mgr->m_em, "cpe_timer_mgr: unregister timer by id: %d not a allocked timer!", timer_id);
    }
    else {
        assert(timer->m_process_ctx);
        cpe_timer_processor_free(mgr, timer);
    }
}

int cpe_timer_mgr_have_timer(cpe_timer_mgr_t mgr, cpe_timer_id_t timer_id) {
    struct cpe_timer_processor * timerPage;
    int pagePos;

    pagePos = timer_id / mgr->m_timer_count_in_page;
    if (pagePos >= (int)mgr->m_timer_page_count) return 0;

    timerPage = mgr->m_timer_buf[pagePos];

    return timerPage[timer_id % mgr->m_timer_count_in_page].m_process_ctx ? 1 : 0;
}

static void cpe_timer_mgr_destory_timer(tl_event_t event, void * context) {
    cpe_timer_mgr_t mgr;
    cpe_timer_id_t timerId;
    struct cpe_timer_processor * timer;

    mgr = (cpe_timer_mgr_t)context;

    timerId = *(cpe_timer_id_t*)tl_event_data(event);

    timer = cpe_timer_processor_get(mgr, timerId);
    if (timer == NULL) {
        CPE_ERROR(mgr->m_em, "cpe_timer_mgr: destory timer: timer(id=%d) not exist!", timerId);
        return;
    }

    if (timer->m_tl_event != event) {
        CPE_ERROR(mgr->m_em, "cpe_timer_mgr: destory timer: timer(id=%d) tl_event mismatch!", timerId);
        return;
    }

    timer->m_tl_event = NULL;
    cpe_timer_processor_free(mgr, timer);
}

static void cpe_timer_mgr_dispatch_timer(tl_event_t input, void * context) {
    cpe_timer_mgr_t mgr;
    cpe_timer_id_t timerId;
    struct cpe_timer_processor * timer;

    mgr = (cpe_timer_mgr_t)context;
    assert(mgr);

    timerId = *(cpe_timer_id_t*)tl_event_data(input);

    timer = cpe_timer_processor_get(mgr, timerId);
    if (timer == NULL) {
        CPE_ERROR(mgr->m_em, "cpe_timer_mgr: dispatch timer: get timer(id=%d) fail!", timerId);
        return;
    }

    timer->m_process_fun(timer->m_process_ctx, timerId, timer->m_process_arg);
}
