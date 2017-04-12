#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_responser.h"
#include "usf/logic/logic_executor.h"
#include "usf/logic/logic_executor_ref.h"
#include "usf/logic/logic_executor_build.h"
#include "usf/bpg_rsp/bpg_rsp.h"
#include "usf/bpg_rsp/bpg_rsp_manage.h"
#include "bpg_rsp_internal_ops.h"

bpg_rsp_t bpg_rsp_create(bpg_rsp_manage_t mgr, const char * name) {
    char * buf;
    size_t name_len;
    bpg_rsp_t rsp;
    
    assert(name);

    name_len = strlen(name) + 1;
    CPE_PAL_ALIGN_DFT(name_len);

    buf = mem_alloc(mgr->m_alloc, sizeof(struct bpg_rsp) + name_len);
    if (buf == NULL) {
        CPE_ERROR(mgr->m_em, "%s: create rsp %s: alloc fail", bpg_rsp_manage_name(mgr), name) ;
        return NULL;
    }

    cpe_str_dup(buf, name_len, name);

    rsp = (bpg_rsp_t)(buf + name_len);
    rsp->m_mgr = mgr;
    rsp->m_name = buf;
    rsp->m_queue_info = NULL;
    rsp->m_flags = 0;
    rsp->m_executor_ref = NULL;
    rsp->m_timeout_ms = 0;

    TAILQ_INIT(&rsp->m_ctx_to_pdu);

    cpe_hash_entry_init(&rsp->m_hh);
    if (cpe_hash_table_insert_unique(&mgr->m_rsps, rsp) != 0) {
        mem_free(mgr->m_alloc, buf);
        return NULL;
    }

    return rsp;
}

void bpg_rsp_free(bpg_rsp_t rsp) {
    dp_rsp_t dp_rsp;

    cpe_hash_table_remove_by_ins(&rsp->m_mgr->m_rsps, rsp);

    bpg_rsp_copy_info_clear(rsp);

    if (rsp->m_executor_ref) {
        logic_executor_ref_dec(rsp->m_executor_ref);
        rsp->m_executor_ref = NULL;
    }

    dp_rsp = dp_rsp_find_by_name(rsp->m_mgr->m_dp, bpg_rsp_name(rsp));
    if (dp_rsp) {
        dp_rsp_free(dp_rsp);
    }

    mem_free(rsp->m_mgr->m_alloc, (void*)rsp->m_name);
}

bpg_rsp_t
bpg_rsp_find(bpg_rsp_manage_t mgr, const char * rsp_name) {
    struct bpg_rsp key;
    key.m_name = rsp_name;
    return (struct bpg_rsp *)cpe_hash_table_find(&mgr->m_rsps, &key);
}

dp_rsp_t bpg_rsp_dp(bpg_rsp_t rsp) {
    return dp_rsp_find_by_name(rsp->m_mgr->m_dp, bpg_rsp_name(rsp));
}

logic_executor_t bpg_rsp_executor(bpg_rsp_t rsp) {
    return rsp->m_executor_ref ? logic_executor_ref_executor(rsp->m_executor_ref) : NULL;
}

void bpg_rsp_set_executor(bpg_rsp_t rsp, logic_executor_ref_t executor) {
    if (executor) logic_executor_ref_inc(executor);
    if (rsp->m_executor_ref) logic_executor_ref_dec(rsp->m_executor_ref);
    rsp->m_executor_ref = executor;
}

const char * bpg_rsp_queue(bpg_rsp_t rsp) {
    return rsp->m_queue_info ? cpe_hs_data(rsp->m_queue_info->m_name) : NULL;
}

int bpg_rsp_set_queue(bpg_rsp_t rsp, const char * queue_name) {
    struct bpg_rsp_queue_info *  queue_info = NULL;
    
    if (queue_name) {
        char name_buf[128];
        cpe_hs_init((cpe_hash_string_t)name_buf, sizeof(name_buf), queue_name);
        queue_info = bpg_rsp_queue_info_find(rsp->m_mgr, (cpe_hash_string_t)name_buf);
        if (queue_info == NULL) {
            CPE_ERROR(
                rsp->m_mgr->m_em, "%s: rsp %s: logic queue %s not exist!", 
                bpg_rsp_manage_name(rsp->m_mgr), bpg_rsp_name(rsp), queue_name) ;
            return -1;
        }
    }

    rsp->m_queue_info = queue_info;

    return 0;
}


const char * bpg_rsp_name(bpg_rsp_t rsp) {
    return rsp->m_name;
}

tl_time_span_t bpg_rsp_timeout_ms(bpg_rsp_t rsp) {
    return rsp->m_timeout_ms;
}

void bpg_rsp_set_timeout_ms(bpg_rsp_t rsp, tl_time_span_t timeout_ms) {
    rsp->m_timeout_ms = timeout_ms;
}

uint32_t bpg_rsp_flags(bpg_rsp_t rsp) {
    return rsp->m_flags;
}

void bpg_rsp_flags_set(bpg_rsp_t rsp, uint32_t flag) {
    rsp->m_flags = flag;
}

void bpg_rsp_flag_enable(bpg_rsp_t rsp, bpg_rsp_flag_t flag) {
    rsp->m_flags |= flag;
}

void bpg_rsp_flag_disable(bpg_rsp_t rsp, bpg_rsp_flag_t flag) {
    rsp->m_flags &= ~((uint32_t)flag);
}

int bpg_rsp_flag_is_enable(bpg_rsp_t rsp, bpg_rsp_flag_t flag) {
    return rsp->m_flags & flag;
}

uint32_t bpg_rsp_hash(const struct bpg_rsp * rsp) {
    return cpe_hash_str(rsp->m_name, strlen(rsp->m_name));
}

int bpg_rsp_cmp(const struct bpg_rsp * l, const struct bpg_rsp * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

void bpg_rsp_free_all(bpg_rsp_manage_t mgr) {
    struct cpe_hash_it rsp_it;
    struct bpg_rsp * rsp;

    cpe_hash_it_init(&rsp_it, &mgr->m_rsps);

    rsp = cpe_hash_it_next(&rsp_it);
    while (rsp) {
        struct bpg_rsp * next = cpe_hash_it_next(&rsp_it);
        bpg_rsp_free(rsp);
        rsp = next;
    }
}
