#ifndef SVR_UHUB_SVR_TYPES_H
#define SVR_UHUB_SVR_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "gd/timer/timer_types.h"
#include "usf/mongo_cli/mongo_cli_proxy.h"
#include "svr/center/agent/center_agent_types.h"
#include "svr/set/stub/set_svr_stub_types.h"

typedef struct uhub_svr * uhub_svr_t;
typedef struct uhub_svr_info * uhub_svr_info_t;
typedef struct uhub_svr_notify_info * uhub_svr_notify_info_t;

typedef TAILQ_HEAD(uhub_svr_info_list, uhub_svr_info) uhub_svr_info_list_t;

struct uhub_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;

    mongo_cli_proxy_t m_db;

    cpe_hash_string_t m_send_to;
    dp_rsp_t m_recv_at;

    uhub_svr_info_list_t m_svr_infos;
    struct cpe_hash_table m_notify_infos;
};

struct uhub_svr_notify_info {
    uint16_t m_svr_type;
    uint32_t m_cmd;
    LPDRMETAENTRY m_to_uid_entry;
    uint32_t m_to_uid_start_pos;
    struct cpe_hash_entry m_hh;
};

struct uhub_svr_info {
    uint16_t m_svr_type;
    set_svr_svr_info_t m_svr_info;
    LPDRMETAENTRY m_cmd_entry;
    uint32_t m_data_meta_start_pos;
    LPDRMETA m_data_meta;
    const char * m_to_uid_entry_name;
    TAILQ_ENTRY(uhub_svr_info) m_next;
};

#endif
