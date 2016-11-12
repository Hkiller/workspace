#ifndef USF_MONGO_USE_INTERNAL_TYPES_H
#define USF_MONGO_USE_INTERNAL_TYPES_H
#include "cpe/utils/hash.h"
#include "gd/utils/utils_types.h"
#include "usf/logic_use/logic_require_queue.h"
#include "usf/mongo_use/mongo_use_types.h"

struct mongo_id_info {
    const char * m_name;
    uint64_t m_next_id;
    uint32_t m_left_id_count;

    uint32_t m_id_start;
    uint32_t m_id_inc;

    logic_require_id_t m_processing_require;
    logic_require_queue_t m_waiting_queue;

    struct cpe_hash_entry m_hh;
};

struct mongo_id_generator {
    struct gd_id_generator m_gen;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    mongo_cli_proxy_t m_mongo_cli;

    struct cpe_hash_table m_id_infos;

    int m_debug;
};

#endif
