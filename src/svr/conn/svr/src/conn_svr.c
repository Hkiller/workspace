#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_responser.h"
#include "cpe/net/net_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/net/net_listener.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_cmp.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "gd/timer/timer_manage.h"
#include "svr/set/share/set_pkg.h"
#include "conn_svr_ops.h"

extern void conn_svr_listener_cb(EV_P_ ev_io *w, int revents);
extern char g_metalib_svr_conn_pro[];
static void conn_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_conn_svr = {
    "svr_conn_svr",
    conn_svr_clear
};

conn_svr_t
conn_svr_create(
    gd_app_context_t app,
    const char * name, 
    uint16_t conn_svr_type,
    mem_allocrator_t alloc, error_monitor_t em)
{
    struct conn_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct conn_svr));
    if (svr_node == NULL) return NULL;

    svr = (conn_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_debug = 0;
    svr->m_ev_loop = net_mgr_ev_loop(gd_app_net_mgr(app));
    svr->m_conn_svr_type = conn_svr_type;
    svr->m_outgoing_pkg = NULL;
    svr->m_ss_send_to = NULL;
    svr->m_ss_request_recv_at = NULL;
    svr->m_ss_trans_recv_at = NULL;
    svr->m_read_block_size = 2048;
    svr->m_conn_timeout_s = 300;
    svr->m_max_pkg_size = UINT16_MAX;
    svr->m_conn_max_id = 0;
    svr->m_ringbuf = NULL;
    svr->m_fd = -1;
    svr->m_check_timer_id = GD_TIMER_ID_INVALID;

    TAILQ_INIT(&svr->m_conns_check);

    if (cpe_hash_table_init(
            &svr->m_conns_by_conn_id,
            alloc,
            (cpe_hash_fun_t) conn_svr_conn_conn_id_hash,
            (cpe_hash_eq_t) conn_svr_conn_conn_id_eq,
            CPE_HASH_OBJ2ENTRY(conn_svr_conn, m_hh_for_conn_id),
            -1) != 0)
    {
        nm_node_free(svr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &svr->m_conns_by_user_id,
            alloc,
            (cpe_hash_fun_t) conn_svr_conn_user_id_hash,
            (cpe_hash_eq_t) conn_svr_conn_user_id_eq,
            CPE_HASH_OBJ2ENTRY(conn_svr_conn, m_hh_for_user_id),
            -1) != 0)
    {
        cpe_hash_table_fini(&svr->m_conns_by_conn_id);
        nm_node_free(svr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &svr->m_backends,
            alloc,
            (cpe_hash_fun_t) conn_svr_backend_hash,
            (cpe_hash_eq_t) conn_svr_backend_eq,
            CPE_HASH_OBJ2ENTRY(conn_svr_backend, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&svr->m_conns_by_user_id);
        cpe_hash_table_fini(&svr->m_conns_by_conn_id);
        nm_node_free(svr_node);
        return NULL;
    }

    nm_node_set_type(svr_node, &s_nm_node_type_conn_svr);

    return svr;
}

static void conn_svr_clear(nm_node_t node) {
    conn_svr_t svr;
    svr = (conn_svr_t)nm_node_data(node);

    conn_svr_stop(svr);
    conn_svr_conn_free_all(svr);
    conn_svr_backend_free_all(svr);

    if (svr->m_outgoing_pkg) {
        dp_req_free(svr->m_outgoing_pkg);
        svr->m_outgoing_pkg = NULL;
    }

    if (svr->m_ss_send_to) {
        mem_free(svr->m_alloc, svr->m_ss_send_to);
        svr->m_ss_send_to = NULL;
    }

    if (svr->m_ss_request_recv_at != NULL) {
        dp_rsp_free(svr->m_ss_request_recv_at);
        svr->m_ss_request_recv_at = NULL;
    }

    if (svr->m_ss_trans_recv_at != NULL) {
        dp_rsp_free(svr->m_ss_trans_recv_at);
        svr->m_ss_trans_recv_at = NULL;
    }

    if (svr->m_check_timer_id != GD_TIMER_ID_INVALID) {
        gd_timer_mgr_t timer_mgr = gd_timer_mgr_find(svr->m_app, NULL);
        assert(timer_mgr);
        gd_timer_mgr_unregist_timer_by_id(timer_mgr, svr->m_check_timer_id);
        svr->m_check_timer_id = GD_TIMER_ID_INVALID;
    }

    if (svr->m_ringbuf) {
        ringbuffer_delete(svr->m_ringbuf);
        svr->m_ringbuf = NULL;
    }

    cpe_hash_table_fini(&svr->m_conns_by_conn_id);
    cpe_hash_table_fini(&svr->m_conns_by_user_id);
    cpe_hash_table_fini(&svr->m_backends);
}

void conn_svr_free(conn_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_conn_svr) return;
    nm_node_free(svr_node);
}

gd_app_context_t conn_svr_app(conn_svr_t svr) {
    return svr->m_app;
}

conn_svr_t
conn_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_conn_svr) return NULL;
    return (conn_svr_t)nm_node_data(node);
}

conn_svr_t
conn_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_conn_svr) return NULL;
    return (conn_svr_t)nm_node_data(node);
}

const char * conn_svr_name(conn_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
conn_svr_name_hs(conn_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t conn_svr_cur_time(conn_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

int conn_svr_set_ss_send_to(conn_svr_t svr, const char * send_to) {
    cpe_hash_string_t new_send_to = cpe_hs_create(svr->m_alloc, send_to);
    if (new_send_to == NULL) return -1;

    if (svr->m_ss_send_to) mem_free(svr->m_alloc, svr->m_ss_send_to);
    svr->m_ss_send_to = new_send_to;

    return 0;
}

int conn_svr_set_ringbuf_size(conn_svr_t svr, size_t capacity) {
    conn_svr_conn_free_all(svr);
    
    if (svr->m_ringbuf) {
        ringbuffer_delete(svr->m_ringbuf);
    }
    svr->m_ringbuf = ringbuffer_new(capacity);

    if (svr->m_ringbuf == NULL) return -1;

    return 0;
}

int conn_svr_ss_request_rsp(dp_req_t req, void * ctx, error_monitor_t em);
int conn_svr_set_ss_request_recv_at(conn_svr_t svr, const char * name) {
    char sp_name_buf[128];

    if (svr->m_ss_request_recv_at != NULL) dp_rsp_free(svr->m_ss_request_recv_at);

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.ss.request", conn_svr_name(svr));
    svr->m_ss_request_recv_at = dp_rsp_create(gd_app_dp_mgr(svr->m_app), sp_name_buf);
    if (svr->m_ss_request_recv_at == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: conn_svr_set_ss_request_recv_at: create rsp fail!",
            conn_svr_name(svr));
        return -1;
    }
    dp_rsp_set_processor(svr->m_ss_request_recv_at, conn_svr_ss_request_rsp, svr);

    if (dp_rsp_bind_string(svr->m_ss_request_recv_at, name, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: conn_svr_set_ss_recv_at: bind rsp to %s fail!",
            conn_svr_name(svr), name);
        dp_rsp_free(svr->m_ss_request_recv_at);
        svr->m_ss_request_recv_at = NULL;
        return -1;
    }

    return 0;
}

int conn_svr_ss_trans_rsp(dp_req_t req, void * ctx, error_monitor_t em);
int conn_svr_set_ss_trans_recv_at(conn_svr_t svr, const char * name) {
    char sp_name_buf[128];

    if (svr->m_ss_trans_recv_at != NULL) dp_rsp_free(svr->m_ss_trans_recv_at);

    snprintf(sp_name_buf, sizeof(sp_name_buf), "%s.ss.trans", conn_svr_name(svr));
    svr->m_ss_trans_recv_at = dp_rsp_create(gd_app_dp_mgr(svr->m_app), sp_name_buf);
    if (svr->m_ss_trans_recv_at == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: conn_svr_set_ss_trans_recv_at: create rsp fail!",
            conn_svr_name(svr));
        return -1;
    }
    dp_rsp_set_processor(svr->m_ss_trans_recv_at, conn_svr_ss_trans_rsp, svr);

    if (dp_rsp_bind_string(svr->m_ss_trans_recv_at, name, svr->m_em) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: conn_svr_set_ss_trans_recv_at: bind rsp to %s fail!",
            conn_svr_name(svr), name);
        dp_rsp_free(svr->m_ss_trans_recv_at);
        svr->m_ss_trans_recv_at = NULL;
        return -1;
    }

    return 0;
}

void conn_svr_timer(void * ctx, gd_timer_id_t timer_id, void * arg);
int conn_svr_set_check_span(conn_svr_t svr, uint32_t span_ms) {
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_find(svr->m_app, NULL);

    if (timer_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: set check span: timer_mgr not exist!", conn_svr_name(svr));
        return -1;
    }

    if (svr->m_check_timer_id != GD_TIMER_ID_INVALID) {
        gd_timer_mgr_unregist_timer_by_id(timer_mgr, svr->m_check_timer_id);
        svr->m_check_timer_id = GD_TIMER_ID_INVALID;
    }

    if (gd_timer_mgr_regist_timer(timer_mgr, &svr->m_check_timer_id, conn_svr_timer, svr, NULL, NULL, span_ms, span_ms, -1) != 0) {
        CPE_ERROR(svr->m_em, "%s: set check span: create timer fail!", conn_svr_name(svr));
        return -1;
    }

    return 0;
}

dp_req_t conn_svr_pkg_buf(conn_svr_t svr) {
    dp_req_t head;
    dp_req_t carry;

    if (svr->m_outgoing_pkg == NULL) {
        svr->m_outgoing_pkg = dp_req_create(gd_app_dp_mgr(svr->m_app), 0);
        if (svr->m_outgoing_pkg == NULL) {
            CPE_ERROR(svr->m_em, "%s: crate outgoing pkg buf fail!", conn_svr_name(svr));
            return NULL;
        }

        head = set_pkg_head_check_create(svr->m_outgoing_pkg);
        if (head == NULL) {
            CPE_ERROR(svr->m_em, "%s: crate outgoing pkg buf head fail!", conn_svr_name(svr));
            dp_req_free(svr->m_outgoing_pkg);
            svr->m_outgoing_pkg = NULL;
            return NULL;
        }

        carry = set_pkg_carry_check_create(svr->m_outgoing_pkg, sizeof(CONN_SVR_CONN_INFO));
        if (carry == NULL) {
            CPE_ERROR(svr->m_em, "%s: crate outgoing pkg buf carry fail!", conn_svr_name(svr));
            dp_req_free(svr->m_outgoing_pkg);
            svr->m_outgoing_pkg = NULL;
            return NULL;
        }

        set_pkg_carry_set_size(carry, sizeof(CONN_SVR_CONN_INFO));
    }
    else {
        head = set_pkg_head_find(svr->m_outgoing_pkg);
        assert(head);
    }

    set_pkg_init(head);

    return svr->m_outgoing_pkg;
}

int conn_svr_start(conn_svr_t svr, const char * ip, uint16_t port, int accept_queue_size) {
    struct sockaddr_in addr;

    svr->m_fd = cpe_sock_open(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (svr->m_fd == -1) {
        CPE_ERROR(svr->m_em, "%s: socket call fail, errno=%d (%s)!", conn_svr_name(svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        return -1;
    }

    if (cpe_sock_set_reuseaddr(svr->m_fd, 1) != 0) {
        CPE_ERROR(svr->m_em, "%s: set sock reuseaddr fail, errno=%d (%s)!", conn_svr_name(svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        return -1;
    }


    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = strcmp(ip, "") == 0 ? INADDR_ANY : inet_addr(ip);
    if(cpe_bind(svr->m_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        CPE_ERROR(svr->m_em, "%s: bind error, errno=%d (%s)", conn_svr_name(svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        cpe_sock_close(svr->m_fd);
        svr->m_fd = -1;
        return -1;
    }

    if (accept_queue_size == 0) accept_queue_size = 512;

    if (cpe_listen(svr->m_fd, accept_queue_size) != 0) {
        CPE_ERROR(svr->m_em, "%s: listen error, errno=%d (%s)", conn_svr_name(svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        cpe_sock_close(svr->m_fd);
        svr->m_fd = -1;
        return -1;
    }

    svr->m_watcher.data = svr;
    ev_io_init(&svr->m_watcher, conn_svr_listener_cb, svr->m_fd, EV_READ);
    ev_io_start(svr->m_ev_loop, &svr->m_watcher);

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: listen start", conn_svr_name(svr));
    }

    return 0;
}

void conn_svr_stop(conn_svr_t svr) {
    if (svr->m_fd < 0) return;

    ev_io_stop(svr->m_ev_loop, &svr->m_watcher);
    cpe_sock_close(svr->m_fd);
    svr->m_fd = -1;

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: listen stop", conn_svr_name(svr));
    }
}

int conn_svr_send_pkg(conn_svr_t svr, dp_req_t req) {
    if (dp_dispatch_by_string(svr->m_ss_send_to, dp_req_mgr(req), req, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em, "%s: send pkg fail!", conn_svr_name(svr));
        return -1;
    }

    return 0;
}
