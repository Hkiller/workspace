#ifndef SVR_SET_SVR_TYPES_H
#define SVR_SET_SVR_TYPES_H
#include "ev.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "cpe/utils/ringbuffer.h"
#include "cpe/net/net_types.h"
#include "cpe/fsm/fsm_def.h"
#include "cpe/fsm/fsm_ins.h"
#include "gd/timer/timer_manage.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "svr/set/share/set_share_types.h"
#include "protocol/svr/set/svr_set_internal.h"
#include "protocol/svr/center/svr_center_pro.h"

typedef struct set_svr_mon * set_svr_mon_t;
typedef struct set_svr_mon_app * set_svr_mon_app_t;
typedef struct set_svr * set_svr_t;
typedef struct set_svr_listener * set_svr_listener_t;
typedef struct set_svr_set * set_svr_set_t;
typedef struct set_svr_set_conn * set_svr_set_conn_t;
typedef struct set_svr_svr_type * set_svr_svr_type_t;
typedef struct set_svr_center * set_svr_center_t;
typedef struct set_svr_svr_ins * set_svr_svr_ins_t;

typedef TAILQ_HEAD(set_svr_svr_ins_list, set_svr_svr_ins) set_svr_svr_ins_list_t;
typedef TAILQ_HEAD(set_svr_set_conn_list, set_svr_set_conn) set_svr_set_conn_list_t;

typedef enum set_svr_scope {
    set_svr_scope_global,
    set_svr_scope_region,
    set_svr_scope_set,
} set_svr_scope_t;

struct set_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    char m_repository_root[128];
    uint16_t m_set_id;
    uint16_t m_region;

    struct ev_loop * m_ev_loop;
    uint32_t m_max_conn_id;

    set_svr_center_t m_center;
    set_svr_mon_t m_mon;

    /*listener*/
    set_svr_listener_t m_listener;

    /*set*/
    fsm_def_machine_t m_set_conn_fsm_def;
    uint16_t m_set_process_count_per_tick;
    uint32_t m_set_timeout_ms;
    uint32_t m_set_read_block_size;
    uint32_t m_set_max_pkg_size;

    ringbuffer_t m_ringbuf;
    
    uint16_t m_local_svr_count;
    set_svr_svr_ins_list_t m_local_svrs;
    struct cpe_hash_table m_svr_inses;

    struct cpe_hash_table m_svr_types_by_id;
    struct cpe_hash_table m_svr_types_by_name;

    struct cpe_hash_table m_sets_by_id;
    set_svr_set_conn_list_t m_accept_set_conns;

    dp_req_t m_incoming_buf;
    struct mem_buffer m_dump_buffer_head;
    struct mem_buffer m_dump_buffer_carry;
    struct mem_buffer m_dump_buffer_body;
};

set_svr_t
set_svr_create(
    gd_app_context_t app, const char * name, 
    const char * repository_root, uint16_t set_id, uint16_t region,
    mem_allocrator_t alloc, error_monitor_t em);
void set_svr_free(set_svr_t svr);

set_svr_t set_svr_find(gd_app_context_t app, cpe_hash_string_t name);
set_svr_t set_svr_find_nc(gd_app_context_t app, const char * name);

cpe_hash_string_t set_svr_name_hs(set_svr_t mgr);
const char * set_svr_name(set_svr_t svr);

int set_svr_set_ringbuf_size(set_svr_t svr, size_t capacity);

ptr_int_t set_svr_dispatch_tick(void * ctx, ptr_int_t arg, float delta);

ringbuffer_block_t set_svr_ringbuffer_alloc(set_svr_t svr, int size, uint32_t id);

uint32_t set_svr_cur_time(set_svr_t svr);

#endif
