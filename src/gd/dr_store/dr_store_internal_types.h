#ifndef GD_DR_STORE_INTERNAL_TYPES_H
#define GD_DR_STORE_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "gd/dr_store/dr_store_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(dr_ref_list, dr_ref) dr_ref_list_t;

struct dr_store_manage {
    mem_allocrator_t m_alloc;
    gd_app_context_t m_app;
    mem_allocrator_t m_allc;
    error_monitor_t m_em;

    int m_debug;

    struct cpe_hash_table m_stores;
    dr_ref_list_t m_refs;
};

struct dr_store {
    dr_store_manage_t m_mgr;
    const char * m_name; 
    LPDRMETALIB m_lib;
    dr_lib_free_fun_t m_free_fun;
    void * m_free_ctx;
    int m_ref_count;

    struct cpe_hash_entry m_hh;
};

struct dr_ref {
    dr_store_manage_t m_mgr;
    dr_store_t m_store;

    TAILQ_ENTRY(dr_ref) m_next;
};

#ifdef __cplusplus
}
#endif

#endif
