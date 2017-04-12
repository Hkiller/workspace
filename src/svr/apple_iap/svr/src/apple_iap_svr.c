#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_cmp.h"
#include "cpe/net/net_manage.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/net_trans/net_trans_group.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "apple_iap_svr_ops.h"

extern char g_metalib_svr_apple_iap_pro[];
static void apple_iap_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_apple_iap_svr = {
    "svr_apple_iap_svr",
    apple_iap_svr_clear
};

#define APPLE_IAP_SVR_LOAD_META(__arg, __name) \
    svr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_apple_iap_pro, __name); \
    assert(svr-> __arg)

apple_iap_svr_t
apple_iap_svr_create(
    gd_app_context_t app,
    const char * name,
    net_trans_manage_t trans_mgr,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct apple_iap_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct apple_iap_svr));
    if (svr_node == NULL) return NULL;

    svr = (apple_iap_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_debug = 0;
    svr->m_is_sandbox = 0;

    APPLE_IAP_SVR_LOAD_META(m_meta_res_validate, "svr_apple_iap_res_validate");
    APPLE_IAP_SVR_LOAD_META(m_meta_res_error, "svr_apple_iap_res_error");

    svr->m_trans_group = net_trans_group_create(trans_mgr, "apple_iap");
    if (svr->m_trans_group == NULL) {
        CPE_ERROR(em, "%s: create: crate trans group fail!", name);
        nm_node_free(svr_node);
        return NULL;
    }

    nm_node_set_type(svr_node, &s_nm_node_type_apple_iap_svr);

    return svr;
}

static void apple_iap_svr_clear(nm_node_t node) {
    apple_iap_svr_t svr;
    svr = (apple_iap_svr_t)nm_node_data(node);

    if (svr->m_trans_group) {
        net_trans_group_free(svr->m_trans_group);
        svr->m_trans_group = NULL;
    }
}

void apple_iap_svr_free(apple_iap_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_apple_iap_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t apple_iap_svr_app(apple_iap_svr_t svr) {
    return svr->m_app;
}

apple_iap_svr_t
apple_iap_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_apple_iap_svr) return NULL;
    return (apple_iap_svr_t)nm_node_data(node);
}

apple_iap_svr_t
apple_iap_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_apple_iap_svr) return NULL;
    return (apple_iap_svr_t)nm_node_data(node);
}

const char * apple_iap_svr_name(apple_iap_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
apple_iap_svr_name_hs(apple_iap_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t apple_iap_svr_cur_time(apple_iap_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

int apple_iap_svr_set_send_to(apple_iap_svr_t svr, const char * send_to) {
    cpe_hash_string_t new_send_to = cpe_hs_create(svr->m_alloc, send_to);
    if (new_send_to == NULL) return -1;

    if (svr->m_send_to) mem_free(svr->m_alloc, svr->m_send_to);
    svr->m_send_to = new_send_to;

    return 0;
}

int apple_iap_svr_request_rsp(dp_req_t req, void * ctx, error_monitor_t em);
int apple_iap_svr_set_request_recv_at(apple_iap_svr_t svr, const char * name) {
    char sp_name_buf[128];

    if (svr->m_recv_at != NULL) dp_rsp_free(svr->m_recv_at);

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.apple_iap.require", apple_iap_svr_name(svr));
    svr->m_recv_at = dp_rsp_create(gd_app_dp_mgr(svr->m_app), sp_name_buf);
    if (svr->m_recv_at == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: apple_iap_svr_set_recv_at: create rsp fail!",
            apple_iap_svr_name(svr));
        return -1;
    }
    dp_rsp_set_processor(svr->m_recv_at, apple_iap_svr_request_rsp, svr);

    if (dp_rsp_bind_string(svr->m_recv_at, name, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: apple_iap_svr_set_recv_at: bind rsp to %s fail!",
            apple_iap_svr_name(svr), name);
        dp_rsp_free(svr->m_recv_at);
        svr->m_recv_at = NULL;
        return -1;
    }

    return 0;
}

dp_req_t apple_iap_svr_pkg_buf(apple_iap_svr_t svr, size_t capacity) {
    if (svr->m_outgoing_pkg && dp_req_capacity(svr->m_outgoing_pkg) < capacity) {
        dp_req_free(svr->m_outgoing_pkg);
        svr->m_outgoing_pkg = NULL;
    }

    if (svr->m_outgoing_pkg == NULL) {
        svr->m_outgoing_pkg = dp_req_create(gd_app_dp_mgr(svr->m_app), capacity);
        if (svr->m_outgoing_pkg == NULL) {
            CPE_ERROR(svr->m_em, "%s: crate outgoing pkg buf fail!", apple_iap_svr_name(svr));
            return NULL;
        }

        if (set_pkg_head_check_create(svr->m_outgoing_pkg) == NULL) {
            CPE_ERROR(svr->m_em, "%s: crate outgoing pkg buf head fail!", apple_iap_svr_name(svr));
            return NULL;
        }
    }

    dp_req_set_meta(svr->m_outgoing_pkg, NULL);

    return svr->m_outgoing_pkg;
}

