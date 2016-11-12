#ifndef GD_EVT_INTERNAL_TYPES_H
#define GD_EVT_INTERNAL_TYPES_H
#include "cpe/utils/hash.h"
#include "cpe/utils/range.h"
#include "cpe/tl/tl_types.h"
#include "gd/evt/evt_types.h"

enum evt_processor_state {
    evt_processor_state_NotInResponserHash = 0
    , evt_processor_state_InResponserHash
};

struct gd_evt_processor {
    evt_processor_id_t m_id;
    enum evt_processor_state m_state;
    void * m_process_ctx;
    void * m_process_arg;
    void (*m_process_arg_free)(void *);
    gd_evt_process_fun_t m_process_fun;
    struct cpe_hash_entry m_hh_for_responser_to_processor;
};

struct gd_evt_def {
    const char * m_evt_name;
    LPDRMETA m_evt_meta;
    struct cpe_hash_entry m_hh;
};

struct gd_evt_mgr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    tl_t m_tl;

    struct cpe_hash_table m_evt_defs;

    int m_debug;

    LPDRMETA m_carry_meta;
    size_t m_carry_size;
    size_t m_oid_max_len;

    dp_req_t m_req;

    size_t m_processor_count_in_page;
    size_t m_processor_page_count;
    size_t m_processor_page_capacity;
    struct gd_evt_processor ** m_processor_buf;
    struct cpe_hash_table m_responser_to_processor;
    struct cpe_range_mgr m_ids;
};

struct gd_evt {
    size_t m_oid_max_len;
    LPDRMETA m_carry_meta;
    size_t m_carry_capacity;
    LPDRMETA m_meta;
    size_t m_data_capacity;
};

#endif

