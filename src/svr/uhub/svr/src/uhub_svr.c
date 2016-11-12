#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_cmp.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "gd/timer/timer_manage.h"
#include "usf/mongo_driver/mongo_pkg.h"
#include "usf/logic_use/logic_op_register.h"
#include "svr/set/share/set_pkg.h"
#include "uhub_svr_ops.h"

extern char g_metalib_svr_uhub_pro[];
static void uhub_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_uhub_svr = {
    "svr_uhub_svr",
    uhub_svr_clear
};

uhub_svr_t
uhub_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    mongo_cli_proxy_t db,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct uhub_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct uhub_svr));
    if (svr_node == NULL) return NULL;

    svr = (uhub_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_db = db;
    svr->m_debug = 0;
    svr->m_send_to = NULL;
    svr->m_recv_at = NULL;

    if (cpe_hash_table_init(
            &svr->m_notify_infos,
            alloc,
            (cpe_hash_fun_t) uhub_svr_notify_info_hash,
            (cpe_hash_eq_t) uhub_svr_notify_info_eq,
            CPE_HASH_OBJ2ENTRY(uhub_svr_notify_info, m_hh),
            -1) != 0)
    {
        nm_node_free(svr_node);
        return NULL;
    }

    TAILQ_INIT(&svr->m_svr_infos);

    nm_node_set_type(svr_node, &s_nm_node_type_uhub_svr);

    return svr;
}

static void uhub_svr_clear(nm_node_t node) {
    uhub_svr_t svr;
    svr = (uhub_svr_t)nm_node_data(node);

    uhub_svr_notify_info_free_all(svr);
    cpe_hash_table_fini(&svr->m_notify_infos);

    while(!TAILQ_EMPTY(&svr->m_svr_infos)) {
        uhub_svr_info_free(svr, TAILQ_FIRST(&svr->m_svr_infos));
    }

    if (svr->m_send_to) {
        mem_free(svr->m_alloc, svr->m_send_to);
        svr->m_send_to = NULL;
    }

    if (svr->m_recv_at != NULL) {
        dp_rsp_free(svr->m_recv_at);
        svr->m_recv_at = NULL;
    }
}

void uhub_svr_free(uhub_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_uhub_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t uhub_svr_app(uhub_svr_t svr) {
    return svr->m_app;
}

uhub_svr_t
uhub_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_uhub_svr) return NULL;
    return (uhub_svr_t)nm_node_data(node);
}

uhub_svr_t
uhub_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_uhub_svr) return NULL;
    return (uhub_svr_t)nm_node_data(node);
}

const char * uhub_svr_name(uhub_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
uhub_svr_name_hs(uhub_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t uhub_svr_cur_time(uhub_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

int uhub_svr_set_send_to(uhub_svr_t svr, const char * send_to) {
    cpe_hash_string_t new_send_to = cpe_hs_create(svr->m_alloc, send_to);
    if (new_send_to == NULL) return -1;

    if (svr->m_send_to) mem_free(svr->m_alloc, svr->m_send_to);
    svr->m_send_to = new_send_to;

    return 0;
}

extern int uhub_svr_rsp(dp_req_t req, void * ctx, error_monitor_t em);
int uhub_svr_set_recv_at(uhub_svr_t svr, const char * name) {
    char sp_name_buf[128];

    if (svr->m_recv_at != NULL) dp_rsp_free(svr->m_recv_at);

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.recv.sp", uhub_svr_name(svr));
    svr->m_recv_at = dp_rsp_create(gd_app_dp_mgr(svr->m_app), sp_name_buf);
    if (svr->m_recv_at == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: uhub_svr_set_recv_at: create rsp fail!",
            uhub_svr_name(svr));
        return -1;
    }
    dp_rsp_set_processor(svr->m_recv_at, uhub_svr_rsp, svr);

    if (dp_rsp_bind_string(svr->m_recv_at, name, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: uhub_svr_set_recv_at: bind rsp to %s fail!",
            uhub_svr_name(svr), name);
        dp_rsp_free(svr->m_recv_at);
        svr->m_recv_at = NULL;
        return -1;
    }

    return 0;
}
