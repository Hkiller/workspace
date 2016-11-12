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
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "conn_http_svr_ops.h"

extern char g_metalib_svr_conn_http_pro[];
static void conn_http_svr_clear(nm_node_t node);
static ebb_connection * conn_http_svr_new_connection(ebb_server *server, struct sockaddr_in *addr);

struct nm_node_type s_nm_node_type_conn_http_svr = {
    "svr_conn_http_svr",
    conn_http_svr_clear
};

#define CONN_HTTP_SVR_LOAD_META(__arg, __name) \
    svr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_conn_http_pro, __name); \
    assert(svr-> __arg)

conn_http_svr_t
conn_http_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    uint16_t port,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct conn_http_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct conn_http_svr));
    if (svr_node == NULL) return NULL;

    svr = (conn_http_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_debug = 0;
    svr->m_max_conn_id = 0;
    svr->m_max_request_id = 0;
    svr->m_port = port;
    svr->m_ringbuf = NULL;
    svr->m_request_recv_at = NULL;
    svr->m_response_recv_at = NULL;

    CONN_HTTP_SVR_LOAD_META(m_meta_res_error, "svr_conn_http_res_error");

    TAILQ_INIT(&svr->m_services);
    TAILQ_INIT(&svr->m_connections);

    ebb_server_init(&svr->m_ebb_svr, net_mgr_ev_loop(gd_app_net_mgr(svr->m_app)));
    svr->m_ebb_svr.new_connection = conn_http_svr_new_connection;
    svr->m_ebb_svr.data = svr;
    if (ebb_server_listen_on_port(&svr->m_ebb_svr, port) < 0) {
        CPE_ERROR(
            svr->m_em, "%s: create: ebb listen on port %d fail!",
            conn_http_svr_name(svr), port);
        nm_node_free(svr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &svr->m_requests,
            alloc,
            (cpe_hash_fun_t) conn_http_request_hash,
            (cpe_hash_eq_t) conn_http_request_eq,
            CPE_HASH_OBJ2ENTRY(conn_http_request, m_hh_for_svr),
            -1) != 0)
    {
        CPE_ERROR(
            svr->m_em, "%s: create: init request hash table fail!",
            conn_http_svr_name(svr));
        ebb_server_unlisten(&svr->m_ebb_svr);
        nm_node_free(svr_node);
        return NULL;
    }

    nm_node_set_type(svr_node, &s_nm_node_type_conn_http_svr);

    return svr;
}

static void conn_http_svr_clear(nm_node_t node) {
    conn_http_svr_t svr;
    svr = (conn_http_svr_t)nm_node_data(node);

    ebb_server_unlisten(&svr->m_ebb_svr);

    conn_http_connection_free_all(svr);
    assert(TAILQ_EMPTY(&svr->m_connections));
    assert(cpe_hash_table_count(&svr->m_requests) == 0);
    cpe_hash_table_fini(&svr->m_requests);

    conn_http_service_free_all(svr);
    assert(TAILQ_EMPTY(&svr->m_services));

    if (svr->m_request_recv_at) {
        dp_rsp_free(svr->m_request_recv_at);
        svr->m_request_recv_at = NULL;
    }

    if (svr->m_response_recv_at) {
        dp_rsp_free(svr->m_response_recv_at);
        svr->m_response_recv_at = NULL;
    }

    if (svr->m_ringbuf) {
        ringbuffer_delete(svr->m_ringbuf);
        svr->m_ringbuf = NULL;
    }
}

void conn_http_svr_free(conn_http_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_conn_http_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t conn_http_svr_app(conn_http_svr_t svr) {
    return svr->m_app;
}

conn_http_svr_t
conn_http_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_conn_http_svr) return NULL;
    return (conn_http_svr_t)nm_node_data(node);
}

conn_http_svr_t
conn_http_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_conn_http_svr) return NULL;
    return (conn_http_svr_t)nm_node_data(node);
}

const char * conn_http_svr_name(conn_http_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
conn_http_svr_name_hs(conn_http_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

int conn_http_svr_set_ringbuf_size(conn_http_svr_t svr, size_t capacity) {
    conn_http_connection_free_all(svr);
    
    if (svr->m_ringbuf) {
        ringbuffer_delete(svr->m_ringbuf);
    }
    svr->m_ringbuf = ringbuffer_new(capacity);

    if (svr->m_ringbuf == NULL) return -1;

    return 0;
}

uint32_t conn_http_svr_cur_time(conn_http_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

int conn_http_request_rsp(dp_req_t req, void * ctx, error_monitor_t em);
int conn_http_svr_set_request_recv_at(conn_http_svr_t svr, const char * name) {
    char sp_name_buf[128];

    if (svr->m_request_recv_at != NULL) dp_rsp_free(svr->m_request_recv_at);

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.conn_http.request", conn_http_svr_name(svr));
    svr->m_request_recv_at = dp_rsp_create(gd_app_dp_mgr(svr->m_app), sp_name_buf);
    if (svr->m_request_recv_at == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: conn_http_svr_set_request_recv_at: create rsp fail!",
            conn_http_svr_name(svr));
        return -1;
    }
    dp_rsp_set_processor(svr->m_request_recv_at, conn_http_request_rsp, svr);

    if (dp_rsp_bind_string(svr->m_request_recv_at, name, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: conn_http_svr_set_request_recv_at: bind rsp to %s fail!",
            conn_http_svr_name(svr), name);
        dp_rsp_free(svr->m_request_recv_at);
        svr->m_request_recv_at = NULL;
        return -1;
    }

    return 0;
}

int conn_http_response_rsp(dp_req_t req, void * ctx, error_monitor_t em);
int conn_http_svr_set_response_recv_at(conn_http_svr_t svr, const char * name) {
    char sp_name_buf[128];

    if (svr->m_response_recv_at != NULL) dp_rsp_free(svr->m_response_recv_at);

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.conn_http.response", conn_http_svr_name(svr));
    svr->m_response_recv_at = dp_rsp_create(gd_app_dp_mgr(svr->m_app), sp_name_buf);
    if (svr->m_response_recv_at == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: conn_http_svr_set_response_recv_at: create rsp fail!",
            conn_http_svr_name(svr));
        return -1;
    }
    dp_rsp_set_processor(svr->m_response_recv_at, conn_http_response_rsp, svr);

    if (dp_rsp_bind_string(svr->m_response_recv_at, name, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: conn_http_svr_set_response_recv_at: bind rsp to %s fail!",
            conn_http_svr_name(svr), name);
        dp_rsp_free(svr->m_response_recv_at);
        svr->m_response_recv_at = NULL;
        return -1;
    }

    return 0;
}

ebb_connection * conn_http_svr_new_connection(ebb_server *server, struct sockaddr_in *addr) {
    conn_http_svr_t svr = server->data;
    conn_http_connection_t connection;

    connection = conn_http_connection_create(svr);
    if(connection == NULL) {
        return NULL;
    }

    return &connection->m_ebb_conn;
}

