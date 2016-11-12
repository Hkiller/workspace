#ifndef SVR_VERSION_SVR_TYPES_H
#define SVR_VERSION_SVR_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/hash.h"
#include "gd/dr_cvt/dr_cvt_types.h"
#include "gd/timer/timer_types.h"
#include "svr/center/agent/center_agent_types.h"
#include "svr/set/stub/set_svr_stub_types.h"
#include "protocol/svr/version/svr_version_pro.h"

typedef struct version_svr * version_svr_t;
typedef struct version_svr_version * version_svr_version_t;
typedef TAILQ_HEAD(version_svr_version_list, version_svr_version) version_svr_version_list_t;
typedef struct version_svr_package * version_svr_package_t;
typedef TAILQ_HEAD(version_svr_package_list, version_svr_package) version_svr_package_list_t;

struct version_svr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    set_svr_stub_t m_stub;
    int m_debug;

    cpe_hash_string_t m_send_to;
    dp_rsp_t m_recv_at;

    struct cpe_hash_table m_versions_by_str;
    version_svr_version_list_t m_versions;
    
    LPDRMETA m_meta_res_query;
    LPDRMETA m_meta_res_error;
};

/*operations of version_svr */
version_svr_t
version_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em);

void version_svr_free(version_svr_t svr);

version_svr_t version_svr_find(gd_app_context_t app, cpe_hash_string_t name);
version_svr_t version_svr_find_nc(gd_app_context_t app, const char * name);
const char * version_svr_name(version_svr_t svr);

int version_svr_set_send_to(version_svr_t svr, const char * send_to);
int version_svr_set_recv_at(version_svr_t svr, const char * name);
int version_svr_rsp(dp_req_t req, void * ctx, error_monitor_t em);

void version_svr_send_error_response(version_svr_t svr, dp_req_t pkg_head, dp_req_t pkg_body, int err);

#endif
