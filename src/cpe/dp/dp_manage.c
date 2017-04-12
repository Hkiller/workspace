#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_responser.h"
#include "dp_internal_ops.h"

dp_mgr_t dp_mgr_create(mem_allocrator_t alloc) {
    dp_mgr_t dpm;
    size_t allocSize = sizeof(struct dp_mgr);

    dpm = (dp_mgr_t)mem_alloc(alloc, allocSize);
    bzero(dpm, allocSize);

    dpm->m_alloc = alloc;

    cpe_hash_table_init(
        &dpm->m_rsps,
        alloc,
        (cpe_hash_fun_t) dp_rsp_hash,
        (cpe_hash_eq_t) dp_rsp_cmp,
        CPE_HASH_OBJ2ENTRY(dp_rsp, m_hh),
        -1);

    cpe_hash_table_set_destory_fun(
        &dpm->m_rsps,
        (cpe_hash_destory_t)dp_rsp_free_i,
        NULL);

    cpe_hash_table_init(
        &dpm->m_cmd_2_rsps,
        alloc,
        (cpe_hash_fun_t) dp_binding_hash,
        (cpe_hash_eq_t) dp_binding_cmp,
        CPE_HASH_OBJ2ENTRY(dp_binding, m_hh),
        -1);

    cpe_hash_table_set_destory_fun(
        &dpm->m_cmd_2_rsps,
        (cpe_hash_destory_t)dp_binding_free_i,
        NULL);

    TAILQ_INIT(&dpm->m_processiong_rsps);

    return dpm;
}

void dp_mgr_free(dp_mgr_t dp) {
    if (dp == NULL) return;

    cpe_hash_table_fini(&dp->m_rsps);

    cpe_hash_table_fini(&dp->m_cmd_2_rsps);

    mem_free(dp->m_alloc, dp);
}

dp_rsp_t dp_rsp_find_by_name(dp_mgr_t dp, const char * name) {
    struct dp_rsp rspBuf;

    assert(dp);

    rspBuf.m_name_len = strlen(name);
    rspBuf.m_name = name;

    return (dp_rsp_t)cpe_hash_table_find(&dp->m_rsps, &rspBuf);
}

static dp_rsp_t dp_rsp_binding_cmd_next(dp_rsp_it_t it) {
    struct dp_binding * r;
    
    if (it->m_context == NULL) return NULL;

    r = (struct dp_binding *)it->m_context;
    it->m_context = r->m_cmd_binding_next;

    return r->m_rsp;
}

void dp_rsp_find_by_numeric(dp_rsp_it_t it, dp_mgr_t dp, int32_t cmd) {
    struct dp_binding_numeric buf;

    assert(it);
    assert(dp);

    buf.m_head.m_kt = dp_key_numeric;
    buf.m_value = cmd;

    it->m_context = (struct dp_binding *)cpe_hash_table_find(&dp->m_cmd_2_rsps, &buf);
    it->m_next_fun = dp_rsp_binding_cmd_next;
}

void dp_rsp_find_by_string(dp_rsp_it_t it, dp_mgr_t dp, const char * cmd) {
    struct dp_binding_string buf;
    buf.m_head.m_kt = dp_key_string;
    buf.m_value = cmd;
    buf.m_value_len = (uint32_t)strlen(cmd);

    it->m_context = (struct dp_binding *)cpe_hash_table_find(&dp->m_cmd_2_rsps, &buf);
    it->m_next_fun = dp_rsp_binding_cmd_next;
}

dp_rsp_t dp_rsp_find_first_by_numeric(dp_mgr_t dp, int32_t cmd) {
    struct dp_rsp_it it;
    dp_rsp_find_by_numeric(&it, dp, cmd);
    return dp_rsp_next(&it);
}

dp_rsp_t dp_rsp_find_first_by_string(dp_mgr_t dp, const char * cmd) {
    struct dp_rsp_it it;
    dp_rsp_find_by_string(&it, dp, cmd);
    return dp_rsp_next(&it);
}

static int dp_do_dispatch_i(struct dp_rsp_it * rsps, dp_mgr_t dm, dp_req_t req, error_monitor_t em) {
    dp_rsp_t rsp;
    int rv;
    int count;
    struct dp_processing_rsp_buf pbuf;

    dp_pbuf_init(dm, &pbuf);

    rv = 0;
    count = 0;

    while((rsp = dp_rsp_next(rsps))) {
        dp_pbuf_add_rsp(&pbuf, rsp, em);
    }

    while((rsp = dp_pbuf_retrieve_first(&pbuf))) {
        ++count;

        if (rsp->m_processor == NULL) {
            CPE_ERROR(em, "responser %s have no processor", rsp->m_name);
            rv = -1;
            continue;
        }

        if (rsp->m_processor(req, rsp->m_context, em) != 0) {
            rv = -1;
            continue;
        }
    }

    dp_pbuf_fini(dm, &pbuf);

    return rv < 0 ? rv : count;
}

int dp_dispatch_by_string(cpe_hash_string_t cmd, dp_mgr_t dm, dp_req_t req, error_monitor_t em) {
    struct dp_rsp_it rspIt;
    int rv;

    dp_rsp_find_by_string(&rspIt, dm, cpe_hs_data(cmd));
    
    rv = dp_do_dispatch_i(&rspIt, dm, req, em);

    if (rv == 0) {
        CPE_INFO(em, "no responser to process cmd %s", cpe_hs_data(cmd));
        rv = -1;
    }

    return rv < 0 ? rv : 0;
}

int dp_dispatch_by_numeric(int32_t cmd, dp_mgr_t dm, dp_req_t req, error_monitor_t em) {
    struct dp_rsp_it rspIt;
    int rv;

    dp_rsp_find_by_numeric(&rspIt, dm, cmd);
    rv = dp_do_dispatch_i(&rspIt, dm, req, em);

    if (rv == 0) {
        CPE_INFO(em, "no responser to process cmd %d", cmd);
        rv = -1;
    }

    return rv < 0 ? rv : 0;
}

int dp_dispatch_by_name(const char * name, dp_mgr_t dm, dp_req_t req, error_monitor_t em) {
    dp_rsp_t rsp;
    rsp = dp_rsp_find_by_name(dm, name);

    if (rsp == 0) {
        CPE_ERROR(em, "no responser name %s", name);
        return -1;
    }
    
    if (rsp->m_processor == NULL) {
        CPE_ERROR(em, "responser %s have no processor", rsp->m_name);
        return -1;
    }

    return rsp->m_processor(req, rsp->m_context, em);
}
