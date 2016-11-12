#ifndef SVR_CENTER_SVR_TYPES_H
#define SVR_CENTER_SVR_TYPES_H
#include "ev.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/ringbuffer.h"
#include "cpe/utils/error.h"
#include "gd/app/app_types.h"
#include "cpe/aom/aom_types.h"
#include "protocol/svr/center/svr_center_pro.h"
#include "protocol/svr/center/svr_center_internal.h"

typedef struct center_svr * center_svr_t;
typedef struct center_svr_conn * center_svr_conn_t;
typedef struct center_svr_ins_proxy * center_svr_ins_proxy_t;
typedef struct center_svr_set_proxy * center_svr_set_proxy_t;
typedef struct center_svr_type * center_svr_type_t;

typedef TAILQ_HEAD(center_svr_ins_proxy_list, center_svr_ins_proxy) center_svr_ins_proxy_list_t;
 
struct center_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;
    uint32_t m_set_offline_timeout;

    LPDRMETA m_record_meta;
    LPDRMETA m_pkg_meta;

    struct ev_loop * m_ev_loop;

    uint32_t m_read_block_size;
    uint32_t m_process_count_per_tick;
    tl_time_span_t m_conn_timeout_ms;

    int m_fd;
    struct ev_io m_watcher;

    ringbuffer_t m_ringbuf;

    uint32_t m_max_pkg_size;

    struct cpe_hash_table m_conns;
    struct cpe_hash_table m_svr_types;

    struct cpe_hash_table m_set_proxies;
    
    struct mem_buffer m_mem_data_buf;
    aom_obj_mgr_t m_client_data_mgr;
    struct cpe_hash_table m_ins_proxies;

    struct mem_buffer m_outgoing_pkg_buf;
    struct mem_buffer m_dump_buffer;
};

/*operations of center_svr */
center_svr_t
center_svr_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em);

uint32_t center_svr_cur_time(center_svr_t svr);
ptr_int_t center_svr_tick(void * ctx, ptr_int_t arg, float delta_s);

int center_svr_start(center_svr_t svr, const char * ip, uint16_t port);
void center_svr_stop(center_svr_t svr);

int center_svr_load_svr_config(center_svr_t svr);

void center_svr_free(center_svr_t svr);

center_svr_t center_svr_find(gd_app_context_t app, cpe_hash_string_t name);
center_svr_t center_svr_find_nc(gd_app_context_t app, const char * name);

cpe_hash_string_t center_svr_name_hs(center_svr_t mgr);
const char * center_svr_name(center_svr_t svr);

int center_svr_set_ringbuf_size(center_svr_t svr, size_t capacity);
SVR_CENTER_PKG * center_svr_get_res_pkg_buff(center_svr_t svr, SVR_CENTER_PKG * req, size_t capacity);

int center_svr_init_clients_from_mem(center_svr_t svr, size_t capacity);
int center_svr_init_clients_from_shm(center_svr_t svr, int shmid);

/*protocol process ops*/
typedef void (*center_svr_conn_op_t)(center_svr_conn_t conn, SVR_CENTER_PKG * pkg, size_t pkg_size);
void center_svr_conn_op_join(center_svr_conn_t conn, SVR_CENTER_PKG * pkg, size_t pkg_size);
void center_svr_conn_op_query_by_type(center_svr_conn_t conn, SVR_CENTER_PKG * pkg, size_t pkg_size);

#endif
