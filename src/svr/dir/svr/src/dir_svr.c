#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "dir_svr.h"
#include "dir_svr_region.h"

extern char g_metalib_svr_dir_pro[];
static void dir_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_dir_svr = {
    "dir_svr",
    dir_svr_clear
};

#define CHAT_SVR_LOAD_META(__arg, __name) \
    svr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_dir_pro, __name); \
    assert(svr-> __arg)

dir_svr_t
dir_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct dir_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct dir_svr));
    if (svr_node == NULL) return NULL;

    svr = (dir_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_debug = 0;

    svr->m_send_to = NULL;
    svr->m_recv_at = NULL;

    CHAT_SVR_LOAD_META(m_meta_res_query_regions, "svr_dir_res_query_regions");
    CHAT_SVR_LOAD_META(m_meta_res_query_servers, "svr_dir_res_query_servers");
    CHAT_SVR_LOAD_META(m_meta_res_error, "svr_dir_res_error");

    svr->m_region_count = 0;
    TAILQ_INIT(&svr->m_regions);

    nm_node_set_type(svr_node, &s_nm_node_type_dir_svr);

    return svr;
}

static void dir_svr_clear(nm_node_t node) {
    dir_svr_t svr;
    svr = (dir_svr_t)nm_node_data(node);

    while(!TAILQ_EMPTY(&svr->m_regions)) {
        dir_svr_region_free(TAILQ_FIRST(&svr->m_regions));
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

void dir_svr_free(dir_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_dir_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t dir_svr_app(dir_svr_t svr) {
    return svr->m_app;
}

dir_svr_t
dir_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_dir_svr) return NULL;
    return (dir_svr_t)nm_node_data(node);
}

dir_svr_t
dir_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_dir_svr) return NULL;
    return (dir_svr_t)nm_node_data(node);
}

const char * dir_svr_name(dir_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
dir_svr_name_hs(dir_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t dir_svr_cur_time(dir_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

int dir_svr_set_send_to(dir_svr_t svr, const char * send_to) {
    cpe_hash_string_t new_send_to = cpe_hs_create(svr->m_alloc, send_to);
    if (new_send_to == NULL) return -1;

    if (svr->m_send_to) mem_free(svr->m_alloc, svr->m_send_to);
    svr->m_send_to = new_send_to;

    return 0;
}

int dir_svr_set_recv_at(dir_svr_t svr, const char * name) {
    char sp_name_buf[128];

    if (svr->m_recv_at != NULL) dp_rsp_free(svr->m_recv_at);

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.recv.sp", dir_svr_name(svr));
    svr->m_recv_at = dp_rsp_create(gd_app_dp_mgr(svr->m_app), sp_name_buf);
    if (svr->m_recv_at == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: dir_svr_set_recv_at: create rsp fail!",
            dir_svr_name(svr));
        return -1;
    }
    dp_rsp_set_processor(svr->m_recv_at, dir_svr_rsp, svr);

    if (dp_rsp_bind_string(svr->m_recv_at, name, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: dir_svr_set_recv_at: bind rsp to %s fail!",
            dir_svr_name(svr), name);
        dp_rsp_free(svr->m_recv_at);
        svr->m_recv_at = NULL;
        return -1;
    }

    return 0;
}
