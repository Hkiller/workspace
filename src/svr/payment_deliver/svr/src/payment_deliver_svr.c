#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
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
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/share/set_pkg.h"
#include "payment_deliver_svr_connection.h"
#include "payment_deliver_svr_request.h"
#include "payment_deliver_svr_adapter.h"

extern char g_metalib_svr_payment_pro[];
static void payment_deliver_svr_clear(nm_node_t node);
static ebb_connection * payment_deliver_svr_new_connection(ebb_server *server, struct sockaddr_in *addr);

struct nm_node_type s_nm_node_type_payment_deliver_svr = {
    "svr_payment_deliver_svr",
    payment_deliver_svr_clear
};

#define PAYMENT_DELIVER_LOAD_META(__arg, __name) \
    svr-> __arg  = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_payment_pro, __name); \
    assert(svr-> __arg)

payment_deliver_svr_t
payment_deliver_svr_create(
    gd_app_context_t app,
    const char * name,
    set_svr_stub_t stub,
    set_svr_svr_info_t payment_svr,
    uint16_t port,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct payment_deliver_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct payment_deliver_svr));
    if (svr_node == NULL) return NULL;

    svr = (payment_deliver_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_stub = stub;
    svr->m_payment_svr = payment_svr;
    svr->m_debug = 0;
    svr->m_max_conn_id = 0;
    svr->m_max_request_id = 0;
    svr->m_port = port;
    svr->m_ringbuf = NULL;
    svr->m_response_recv_at = NULL;

    TAILQ_INIT(&svr->m_connections);
    TAILQ_INIT(&svr->m_adapters);

    PAYMENT_DELIVER_LOAD_META(m_meta_req_notify, "svr_payment_req_notify");
    PAYMENT_DELIVER_LOAD_META(m_meta_qihoo_record, "svr_payment_qihoo_record");
    PAYMENT_DELIVER_LOAD_META(m_meta_iapppay_record, "svr_payment_iapppay_record");
    PAYMENT_DELIVER_LOAD_META(m_meta_damai_record, "svr_payment_damai_record");

    ebb_server_init(&svr->m_ebb_svr, net_mgr_ev_loop(gd_app_net_mgr(svr->m_app)));
    svr->m_ebb_svr.new_connection = payment_deliver_svr_new_connection;
    svr->m_ebb_svr.data = svr;
    if (ebb_server_listen_on_port(&svr->m_ebb_svr, port) < 0) {
        CPE_ERROR(
            svr->m_em, "%s: create: ebb listen on port %d fail!",
            payment_deliver_svr_name(svr), port);
        nm_node_free(svr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &svr->m_requests,
            alloc,
            (cpe_hash_fun_t) payment_deliver_request_hash,
            (cpe_hash_eq_t) payment_deliver_request_eq,
            CPE_HASH_OBJ2ENTRY(payment_deliver_request, m_hh_for_svr),
            -1) != 0)
    {
        CPE_ERROR(
            svr->m_em, "%s: create: init request hash table fail!",
            payment_deliver_svr_name(svr));
        ebb_server_unlisten(&svr->m_ebb_svr);
        nm_node_free(svr_node);
        return NULL;
    }

    nm_node_set_type(svr_node, &s_nm_node_type_payment_deliver_svr);

    return svr;
}

static void payment_deliver_svr_clear(nm_node_t node) {
    payment_deliver_svr_t svr;
    svr = (payment_deliver_svr_t)nm_node_data(node);

    ebb_server_unlisten(&svr->m_ebb_svr);

    payment_deliver_connection_free_all(svr);
    assert(TAILQ_EMPTY(&svr->m_connections));
    assert(cpe_hash_table_count(&svr->m_requests) == 0);
    cpe_hash_table_fini(&svr->m_requests);

    if (svr->m_response_recv_at) {
        dp_rsp_free(svr->m_response_recv_at);
        svr->m_response_recv_at = NULL;
    }

    if (svr->m_ringbuf) {
        ringbuffer_delete(svr->m_ringbuf);
        svr->m_ringbuf = NULL;
    }
}

void payment_deliver_svr_free(payment_deliver_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_payment_deliver_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t payment_deliver_svr_app(payment_deliver_svr_t svr) {
    return svr->m_app;
}

payment_deliver_svr_t
payment_deliver_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_payment_deliver_svr) return NULL;
    return (payment_deliver_svr_t)nm_node_data(node);
}

payment_deliver_svr_t
payment_deliver_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_payment_deliver_svr) return NULL;
    return (payment_deliver_svr_t)nm_node_data(node);
}

const char * payment_deliver_svr_name(payment_deliver_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
payment_deliver_svr_name_hs(payment_deliver_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

int payment_deliver_svr_set_ringbuf_size(payment_deliver_svr_t svr, size_t capacity) {
    payment_deliver_connection_free_all(svr);
    
    if (svr->m_ringbuf) {
        ringbuffer_delete(svr->m_ringbuf);
    }
    svr->m_ringbuf = ringbuffer_new(capacity);

    if (svr->m_ringbuf == NULL) return -1;

    return 0;
}

uint32_t payment_deliver_svr_cur_time(payment_deliver_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

int payment_deliver_response_rsp(dp_req_t req, void * ctx, error_monitor_t em);
int payment_deliver_svr_set_response_recv_at(payment_deliver_svr_t svr, const char * name) {
    char sp_name_buf[128];

    if (svr->m_response_recv_at != NULL) dp_rsp_free(svr->m_response_recv_at);

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.payment_deliver.response", payment_deliver_svr_name(svr));
    svr->m_response_recv_at = dp_rsp_create(gd_app_dp_mgr(svr->m_app), sp_name_buf);
    if (svr->m_response_recv_at == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: payment_deliver_svr_set_response_recv_at: create rsp fail!",
            payment_deliver_svr_name(svr));
        return -1;
    }
    dp_rsp_set_processor(svr->m_response_recv_at, payment_deliver_response_rsp, svr);

    if (dp_rsp_bind_string(svr->m_response_recv_at, name, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: payment_deliver_svr_set_response_recv_at: bind rsp to %s fail!",
            payment_deliver_svr_name(svr), name);
        dp_rsp_free(svr->m_response_recv_at);
        svr->m_response_recv_at = NULL;
        return -1;
    }

    return 0;
}

ebb_connection * payment_deliver_svr_new_connection(ebb_server *server, struct sockaddr_in *addr) {
    payment_deliver_svr_t svr = server->data;
    payment_deliver_connection_t connection;

    connection = payment_deliver_connection_create(svr);
    if(connection == NULL) {
        return NULL;
    }

    return &connection->m_ebb_conn;
}

SVR_PAYMENT_REQ_NOTIFY *
payment_deliver_svr_notify_pkg(payment_deliver_svr_t svr, dp_req_t * pkg, uint16_t service, uint8_t device_category, const char * trade_id) {
    SVR_PAYMENT_REQ_NOTIFY * notify;

    assert(pkg);

    *pkg = set_svr_stub_outgoing_pkg_buf(svr->m_stub, sizeof(SVR_PAYMENT_PKG));
    if (*pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: payment_deliver_svr_notify_pkg: get response pkg fail!", payment_deliver_svr_name(svr));
        return NULL;
    }

    notify = set_svr_stub_pkg_to_data(svr->m_stub, *pkg, set_svr_svr_info_svr_type_id(svr->m_payment_svr), svr->m_meta_req_notify, NULL);
    assert(notify);

    notify->service = service;
    notify->device_category = device_category;
    cpe_str_dup(notify->trade_id, sizeof(notify->trade_id), trade_id);
    
    return notify;
}
