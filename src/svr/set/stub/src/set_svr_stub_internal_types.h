#ifndef SVR_SET_STUB_INTERNAL_TYPES_H
#define SVR_SET_STUB_INTERNAL_TYPES_H

#ifdef _MSC_VER
#else
#include <sys/file.h>
#endif
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "cpe/net/net_types.h"
#include "cpe/fsm/fsm_def.h"
#include "cpe/fsm/fsm_ins.h"
#include "gd/timer/timer_manage.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "protocol/svr/set/svr_set_pro.h"

typedef TAILQ_HEAD(set_svr_stub_buff_list, set_svr_stub_buff) set_svr_stub_buff_list_t;

struct set_svr_stub {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    char * m_pidfile;
    int m_pidfile_fd;  
    struct flock m_pidfile_lock;  

    set_svr_svr_info_t m_svr_type;

    uint16_t m_svr_id;
    uint32_t m_process_count_per_tick;

    cpe_hash_string_t m_request_dispatch_to;
    cpe_hash_string_t m_response_dispatch_to;
    cpe_hash_string_t m_notify_dispatch_to;

    struct cpe_hash_table m_svr_infos;

    dp_rsp_t m_outgoing_recv_at;

    set_chanel_t m_chanel;

    dp_req_t m_incoming_buf;
    dp_req_t m_outgoing_buf;

    int8_t m_use_shm;
    set_svr_stub_buff_list_t m_buffs;

    struct mem_buffer m_dump_buffer_head;
    struct mem_buffer m_dump_buffer_carry;
    struct mem_buffer m_dump_buffer_body;
};

struct set_svr_cmd_info {
    const char * m_meta_name;
    LPDRMETAENTRY m_entry;
    struct cpe_hash_entry m_hh;
};

struct set_svr_svr_info {
    uint16_t m_svr_type_id;
    char m_svr_type_name[64];
    LPDRMETA m_pkg_meta;
    LPDRMETAENTRY m_pkg_cmd_entry;
    LPDRMETAENTRY m_pkg_data_entry;
    LPDRMETA m_carry_meta;

    LPDRMETA m_error_pkg_meta;
    uint32_t m_error_pkg_cmd;
    LPDRMETAENTRY m_error_pkg_error_entry;

    cpe_hash_string_t m_notify_dispatch_to;
    cpe_hash_string_t m_response_dispatch_to;
    struct cpe_hash_entry m_hh;
    struct cpe_hash_table m_cmds;
};

struct set_svr_stub_buff {
    TAILQ_ENTRY(set_svr_stub_buff) m_next;
    set_svr_stub_buff_type_t m_buff_type;
    void * m_buff;
    int m_shm_id;
};

#endif
