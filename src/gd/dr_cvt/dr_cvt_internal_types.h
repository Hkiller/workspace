#ifndef GD_DR_CVT_INTERNAL_TYPES_H
#define GD_DR_CVT_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "gd/dr_cvt/dr_cvt_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(dr_ref_list, dr_ref) dr_ref_list_t;

struct dr_cvt_manage {
    mem_allocrator_t m_alloc;
    gd_app_context_t m_app;
    mem_allocrator_t m_allc;
    error_monitor_t m_em;

    int m_debug;

    struct cpe_hash_table m_cvt_types;
};

struct dr_cvt_type {
    dr_cvt_manage_t m_mgr;
    const char * m_name;
    dr_cvt_fun_t m_encode;
    dr_cvt_fun_t m_decode;
    void * m_ctx;
    int m_ref_count;

    struct cpe_hash_entry m_hh;
};

typedef TAILQ_HEAD(dr_cvt_type_list, dr_cvt_type) dr_cvt_type_list_t;

struct dr_cvt {
    struct dr_cvt_type * m_type;
};

#ifdef __cplusplus
}
#endif

#endif
