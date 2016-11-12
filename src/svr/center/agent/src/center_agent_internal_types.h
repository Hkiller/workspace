#ifndef SVR_CENTER_AGENT_INTERNAL_TYPES_H
#define SVR_CENTER_AGENT_INTERNAL_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "svr/center/agent/center_agent_types.h"
#include "protocol/svr/center/svr_center_pro.h"

struct center_agent {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    struct cpe_hash_table m_data_svrs;
    struct cpe_hash_table m_svr_types;

    struct mem_buffer m_dump_buffer;
};

struct center_agent_svr_type {
    center_agent_t m_agent;
    uint16_t m_svr_type_id;
    char * m_svr_type_name;

    LPDRMETA m_pkg_meta;

    struct cpe_hash_entry m_hh;
};

struct center_agent_pkg {
    center_agent_t m_agent;
    dp_req_t m_dp_req;
};

#endif
