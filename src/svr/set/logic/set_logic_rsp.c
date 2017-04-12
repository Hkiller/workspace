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
#include "svr/set/logic/set_logic_rsp.h"
#include "svr/set/logic/set_logic_rsp_manage.h"
#include "set_logic_rsp_ops.h"

set_logic_rsp_t set_logic_rsp_create(set_logic_rsp_manage_t mgr, const char * name) {
    char * buf;
    size_t name_len;
    set_logic_rsp_t rsp;
    
    assert(name);

    name_len = strlen(name) + 1;
    CPE_PAL_ALIGN_DFT(name_len);

    buf = mem_alloc(mgr->m_alloc, sizeof(struct set_logic_rsp) + name_len);
    if (buf == NULL) {
        CPE_ERROR(mgr->m_em, "%s: create rsp %s: alloc fail", set_logic_rsp_manage_name(mgr), name) ;
        return NULL;
    }

    cpe_str_dup(buf, name_len, name);

    rsp = (set_logic_rsp_t)(buf + name_len);
    rsp->m_mgr = mgr;
    rsp->m_name = buf;
    rsp->m_queue_info = NULL;
    rsp->m_flags = 0;
    rsp->m_executor_ref = NULL;
    rsp->m_timeout_ms = 0;

    cpe_hash_entry_init(&rsp->m_hh);
    if (cpe_hash_table_insert_unique(&mgr->m_rsps, rsp) != 0) {
        mem_free(mgr->m_alloc, buf);
        return NULL;
    }

    return rsp;
}

void set_logic_rsp_free(set_logic_rsp_t rsp) {
    dp_rsp_t dp_rsp;

    cpe_hash_table_remove_by_ins(&rsp->m_mgr->m_rsps, rsp);

    if (rsp->m_executor_ref) {
        logic_executor_ref_dec(rsp->m_executor_ref);
        rsp->m_executor_ref = NULL;
    }

    dp_rsp = dp_rsp_find_by_name(rsp->m_mgr->m_dp, set_logic_rsp_name(rsp));
    if (dp_rsp) {
        dp_rsp_free(dp_rsp);
    }

    mem_free(rsp->m_mgr->m_alloc, (void*)rsp->m_name);
}

set_logic_rsp_t
set_logic_rsp_find(set_logic_rsp_manage_t mgr, const char * rsp_name) {
    struct set_logic_rsp key;
    key.m_name = rsp_name;
    return (struct set_logic_rsp *)cpe_hash_table_find(&mgr->m_rsps, &key);
}

dp_rsp_t set_logic_rsp_dp(set_logic_rsp_t rsp) {
    return dp_rsp_find_by_name(rsp->m_mgr->m_dp, set_logic_rsp_name(rsp));
}

logic_executor_t set_logic_rsp_executor(set_logic_rsp_t rsp) {
    return rsp->m_executor_ref ? logic_executor_ref_executor(rsp->m_executor_ref) : NULL;
}

void set_logic_rsp_set_executor(set_logic_rsp_t rsp, logic_executor_ref_t executor) {
    if (executor) logic_executor_ref_inc(executor);
    if (rsp->m_executor_ref) logic_executor_ref_dec(rsp->m_executor_ref);
    rsp->m_executor_ref = executor;
}

const char * set_logic_rsp_queue(set_logic_rsp_t rsp) {
    return rsp->m_queue_info ? cpe_hs_data(rsp->m_queue_info->m_name) : NULL;
}

int set_logic_rsp_set_queue(set_logic_rsp_t rsp, const char * queue_name) {
    struct set_logic_rsp_queue_info *  queue_info = NULL;
    
    if (queue_name) {
        char name_buf[128];
        cpe_hs_init((cpe_hash_string_t)name_buf, sizeof(name_buf), queue_name);
        queue_info = set_logic_rsp_queue_info_find(rsp->m_mgr, (cpe_hash_string_t)name_buf);
        if (queue_info == NULL) {
            CPE_ERROR(
                rsp->m_mgr->m_em, "%s: rsp %s: logic queue %s not exist!", 
                set_logic_rsp_manage_name(rsp->m_mgr), set_logic_rsp_name(rsp), queue_name) ;
            return -1;
        }
    }

    rsp->m_queue_info = queue_info;

    return 0;
}


const char * set_logic_rsp_name(set_logic_rsp_t rsp) {
    return rsp->m_name;
}

tl_time_span_t set_logic_rsp_timeout_ms(set_logic_rsp_t rsp) {
    return rsp->m_timeout_ms;
}

void set_logic_rsp_set_timeout_ms(set_logic_rsp_t rsp, tl_time_span_t timeout_ms) {
    rsp->m_timeout_ms = timeout_ms;
}

uint32_t set_logic_rsp_flags(set_logic_rsp_t rsp) {
    return rsp->m_flags;
}

void set_logic_rsp_flags_set(set_logic_rsp_t rsp, uint32_t flag) {
    rsp->m_flags = flag;
}

void set_logic_rsp_flag_enable(set_logic_rsp_t rsp, set_logic_rsp_flag_t flag) {
    rsp->m_flags |= flag;
}

void set_logic_rsp_flag_disable(set_logic_rsp_t rsp, set_logic_rsp_flag_t flag) {
    rsp->m_flags &= ~((uint32_t)flag);
}

int set_logic_rsp_flag_is_enable(set_logic_rsp_t rsp, set_logic_rsp_flag_t flag) {
    return rsp->m_flags & flag;
}

uint32_t set_logic_rsp_hash(const struct set_logic_rsp * rsp) {
    return cpe_hash_str(rsp->m_name, strlen(rsp->m_name));
}

int set_logic_rsp_cmp(const struct set_logic_rsp * l, const struct set_logic_rsp * r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

void set_logic_rsp_free_all(set_logic_rsp_manage_t mgr) {
    struct cpe_hash_it rsp_it;
    struct set_logic_rsp * rsp;

    cpe_hash_it_init(&rsp_it, &mgr->m_rsps);

    rsp = cpe_hash_it_next(&rsp_it);
    while (rsp) {
        struct set_logic_rsp * next = cpe_hash_it_next(&rsp_it);
        set_logic_rsp_free(rsp);
        rsp = next;
    }
}
