#include <assert.h>
#include "cpe/pal/pal_stdarg.h"
#include "cpe/pal/pal_string.h"
#include "cpe/dp/dp_responser.h"
#include "dp_internal_ops.h"

dp_rsp_t dp_rsp_create(dp_mgr_t dp, const char * name) {
    size_t nameLen;
    dp_rsp_t r;

    if (dp == NULL || name == NULL) return NULL;

    nameLen = strlen(name);

    r = (dp_rsp_t)mem_alloc(dp->m_alloc, sizeof(struct dp_rsp) + nameLen + 1);
    if (r == NULL) return NULL;

    r->m_dp = dp;
    r->m_name = (char*)(r + 1);
    r->m_name_len = nameLen;
    r->m_bindings = NULL;
    r->m_processor = NULL;
    r->m_type = NULL;
    r->m_context = NULL;
    cpe_hash_entry_init(&r->m_hh);
    memcpy((char*)r->m_name, name, nameLen + 1);

    if (cpe_hash_table_insert_unique(&dp->m_rsps, r) != 0) {
        mem_free(dp->m_alloc, r);
        return NULL;
    }

    return r;
}

void dp_rsp_free_i(dp_rsp_t rsp) {
    dp_pbuf_remove_rsp(rsp->m_dp, rsp);

    while(rsp->m_bindings) {
        dp_binding_free(rsp->m_bindings);
    }

    if (rsp->m_type && rsp->m_type->destruct) {
        rsp->m_type->destruct(rsp);
    }

    mem_free(rsp->m_dp->m_alloc, rsp);
}

void dp_rsp_free(dp_rsp_t rsp) {
    if (rsp == NULL) return;
    cpe_hash_table_remove_by_ins(&rsp->m_dp->m_rsps, rsp);
}

const char * dp_rsp_name(dp_rsp_t rsp) {
    return rsp->m_name;
}

void dp_rsp_set_processor(dp_rsp_t rsp, dp_rsp_process_fun_t process, void * ctx) {
    rsp->m_processor = process;
    rsp->m_context = ctx;
}

void dp_rsp_set_type(dp_rsp_t rsp, dp_rsp_type_t type) {
    rsp->m_type = type;
}

dp_rsp_process_fun_t dp_rsp_processor(dp_rsp_t rsp) {
    return rsp->m_processor;
}

dp_rsp_type_t dp_rsp_type(dp_rsp_t rsp) {
    return rsp->m_type;
}

void * dp_rsp_context(dp_rsp_t rsp) {
    return rsp->m_context;
}

int32_t dp_rsp_hash(const dp_rsp_t rsp) {
    return cpe_hash_str(rsp->m_name, rsp->m_name_len);
}

int dp_rsp_cmp(const dp_rsp_t l, const dp_rsp_t r) {
    return (l->m_name_len == r->m_name_len)
        && (strcmp(l->m_name, r->m_name) == 0);
}

static dp_binding_t dp_rsp_binding_do_next(struct dp_binding_it * it) {
    dp_binding_t r;

    if (it->m_context == NULL) return NULL;

    r = (dp_binding_t)it->m_context;
    it->m_context = r->m_rep_binding_next;

    return r;
}

void dp_rsp_bindings(struct dp_binding_it * it, dp_rsp_t rsp) {
    assert(it);
    it->m_context = rsp->m_bindings;
    it->next = dp_rsp_binding_do_next;
}
