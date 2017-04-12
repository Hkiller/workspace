#ifndef SVR_DIR_SVR_MODULE_H
#define SVR_DIR_SVR_MODULE_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "svr/center/agent/center_agent_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "protocol/svr/dir/svr_dir_pro.h"

typedef struct dir_svr * dir_svr_t;

typedef struct dir_svr_region * dir_svr_region_t;
typedef struct dir_svr_server * dir_svr_server_t;

typedef TAILQ_HEAD(dir_svr_region_list, dir_svr_region) dir_svr_region_list_t;
typedef TAILQ_HEAD(dir_svr_server_list, dir_svr_server) dir_svr_server_list_t;

struct dir_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;

    cpe_hash_string_t m_send_to;
    dp_rsp_t m_recv_at;

    LPDRMETA m_meta_res_query_regions;
    LPDRMETA m_meta_res_query_servers;
    LPDRMETA m_meta_res_error;

    uint16_t m_region_count;
    dir_svr_region_list_t m_regions;
};

/*operations of dir_svr */
dir_svr_t
dir_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em);

void dir_svr_free(dir_svr_t svr);

dir_svr_t dir_svr_find(gd_app_context_t app, cpe_hash_string_t name);
dir_svr_t dir_svr_find_nc(gd_app_context_t app, const char * name);
const char * dir_svr_name(dir_svr_t svr);

int dir_svr_set_send_to(dir_svr_t svr, const char * send_to);
int dir_svr_set_recv_at(dir_svr_t svr, const char * name);
int dir_svr_rsp(dp_req_t req, void * ctx, error_monitor_t em);

void dir_svr_send_error_response(dir_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body, int err);

#endif
