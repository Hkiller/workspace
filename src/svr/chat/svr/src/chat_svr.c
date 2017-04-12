#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/net/net_listener.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_cmp.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "chat_svr_ops.h"

extern char g_metalib_svr_chat_pro[];
static void chat_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_chat_svr = {
    "svr_chat_svr",
    chat_svr_clear
};

#define CHAT_SVR_LOAD_META(__arg, __name) \
    svr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_chat_pro, __name); \
    assert(svr-> __arg)

chat_svr_t
chat_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct chat_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct chat_svr));
    if (svr_node == NULL) return NULL;

    svr = (chat_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_debug = 0;

    svr->m_check_once_process_count = 100;
    svr->m_check_timer_id = GD_TIMER_ID_INVALID;

    svr->m_send_to = NULL;
    svr->m_recv_at = NULL;

    svr->m_chanel_info_count = 0;
    svr->m_chanel_infos = NULL;

    CHAT_SVR_LOAD_META(m_meta_res_query, "svr_chat_res_query_msg");
    CHAT_SVR_LOAD_META(m_meta_res_error, "svr_chat_res_error");

    TAILQ_INIT(&svr->m_chanel_check_queue);

    if (cpe_hash_table_init(
            &svr->m_chanels,
            alloc,
            (cpe_hash_fun_t) chat_svr_chanel_hash,
            (cpe_hash_eq_t) chat_svr_chanel_eq,
            CPE_HASH_OBJ2ENTRY(chat_svr_chanel, m_hh),
            -1) != 0)
    {
        nm_node_free(svr_node);
        return NULL;
    }

    nm_node_set_type(svr_node, &s_nm_node_type_chat_svr);

    return svr;
}

static void chat_svr_clear(nm_node_t node) {
    chat_svr_t svr;
    svr = (chat_svr_t)nm_node_data(node);

    chat_svr_chanel_free_all(svr);
    assert(cpe_hash_table_count(&svr->m_chanels) == 0);
    assert(TAILQ_EMPTY(&svr->m_chanel_check_queue));

    if (svr->m_send_to) {
        mem_free(svr->m_alloc, svr->m_send_to);
        svr->m_send_to = NULL;
    }

    if (svr->m_recv_at != NULL) {
        dp_rsp_free(svr->m_recv_at);
        svr->m_recv_at = NULL;
    }

    if (svr->m_chanel_infos) {
        mem_free(svr->m_alloc, svr->m_chanel_infos);
        svr->m_chanel_infos = NULL;
        svr->m_chanel_info_count = 0;
    }

    if (svr->m_check_timer_id != GD_TIMER_ID_INVALID) {
        gd_timer_mgr_t timer_mgr = gd_timer_mgr_find(svr->m_app, NULL);
        assert(timer_mgr);
        gd_timer_mgr_unregist_timer_by_id(timer_mgr, svr->m_check_timer_id);
        svr->m_check_timer_id = GD_TIMER_ID_INVALID;
    }

    cpe_hash_table_fini(&svr->m_chanels);
}

void chat_svr_free(chat_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_chat_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t chat_svr_app(chat_svr_t svr) {
    return svr->m_app;
}

chat_svr_t
chat_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_chat_svr) return NULL;
    return (chat_svr_t)nm_node_data(node);
}

chat_svr_t
chat_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_chat_svr) return NULL;
    return (chat_svr_t)nm_node_data(node);
}

const char * chat_svr_name(chat_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
chat_svr_name_hs(chat_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t chat_svr_cur_time(chat_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

int chat_svr_set_send_to(chat_svr_t svr, const char * send_to) {
    cpe_hash_string_t new_send_to = cpe_hs_create(svr->m_alloc, send_to);
    if (new_send_to == NULL) return -1;

    if (svr->m_send_to) mem_free(svr->m_alloc, svr->m_send_to);
    svr->m_send_to = new_send_to;

    return 0;
}

int chat_svr_rsp(dp_req_t req, void * ctx, error_monitor_t em);
int chat_svr_set_recv_at(chat_svr_t svr, const char * name) {
    char sp_name_buf[128];

    if (svr->m_recv_at != NULL) dp_rsp_free(svr->m_recv_at);

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.recv.sp", chat_svr_name(svr));
    svr->m_recv_at = dp_rsp_create(gd_app_dp_mgr(svr->m_app), sp_name_buf);
    if (svr->m_recv_at == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: chat_svr_set_recv_at: create rsp fail!",
            chat_svr_name(svr));
        return -1;
    }
    dp_rsp_set_processor(svr->m_recv_at, chat_svr_rsp, svr);

    if (dp_rsp_bind_string(svr->m_recv_at, name, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: chat_svr_set_recv_at: bind rsp to %s fail!",
            chat_svr_name(svr), name);
        dp_rsp_free(svr->m_recv_at);
        svr->m_recv_at = NULL;
        return -1;
    }

    return 0;
}

void chat_svr_timer(void * ctx, gd_timer_id_t timer_id, void * arg);
int chat_svr_set_check_span(chat_svr_t svr, uint32_t span_ms) {
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_find(svr->m_app, NULL);

    if (timer_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: set check span: timer_mgr not exist!", chat_svr_name(svr));
        return -1;
    }

    if (svr->m_check_timer_id != GD_TIMER_ID_INVALID) {
        gd_timer_mgr_unregist_timer_by_id(timer_mgr, svr->m_check_timer_id);
        svr->m_check_timer_id = GD_TIMER_ID_INVALID;
    }

    if (gd_timer_mgr_regist_timer(timer_mgr, &svr->m_check_timer_id, chat_svr_timer, svr, NULL, NULL, span_ms, span_ms, -1) != 0) {
        CPE_ERROR(svr->m_em, "%s: set check span: create timer fail!", chat_svr_name(svr));
        return -1;
    }

    return 0;
}
