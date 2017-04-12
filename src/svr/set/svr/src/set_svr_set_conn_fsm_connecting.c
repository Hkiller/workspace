#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_socket.h"
#include "gd/app/app_context.h"
#include "set_svr_set_conn_fsm.h"

static void set_svr_set_conn_connect_cb(EV_P_ ev_io *w, int revents);

static void set_svr_set_conn_fsm_connecting_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_set_conn_t conn = fsm_machine_context(fsm);
    set_svr_set_t set = conn->m_set;
    set_svr_t svr = conn->m_svr;
    struct sockaddr_in addr;

    if (set_svr_set_conn_start_state_timer(conn, svr->m_set_timeout_ms) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: set %s: connecting: start timer fail!", 
            set_svr_name(svr), set_svr_set_name(conn->m_set));
        set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
        return;
    }

    assert(conn->m_fd == -1);
    assert(set->m_ip[0]);
    assert(set->m_port != 0);
    
    conn->m_fd = cpe_sock_open(AF_INET, SOCK_STREAM, 0);
    if (conn->m_fd == -1) {
        CPE_ERROR(
            svr->m_em, "%s: set %s: connecting: create socket fail, errno=%d (%s)!",
            set_svr_name(svr), set_svr_set_name(set),
            cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
        return;
    }

    if (cpe_sock_set_none_block(conn->m_fd, 1) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: set %s: connecting: set socket none block fail, errno=%d (%s)!",
            set_svr_name(svr), set_svr_set_name(set),
            cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(set->m_port);
    addr.sin_addr.s_addr = inet_addr(set->m_ip);

    if (cpe_connect(conn->m_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        if (cpe_sock_errno() == EINPROGRESS || cpe_sock_errno() == EWOULDBLOCK) {
            CPE_INFO(
                svr->m_em, "%s: set %s: fd %d: connecting: connect started!",
                set_svr_name(svr), set_svr_set_name(set), conn->m_fd);
            ev_io_init(&conn->m_watcher, set_svr_set_conn_connect_cb, conn->m_fd, EV_WRITE);
            ev_io_start(svr->m_ev_loop, &conn->m_watcher);
            return;
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: set %s: fd %d: connecting: connect error, errno=%d (%s)",
                set_svr_name(svr), set_svr_set_name(set), conn->m_fd,
                cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
            cpe_sock_close(conn->m_fd);
            set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
            return;
        }
    }
    else {
        CPE_INFO(
            svr->m_em, "%s: set %s: fd %d: connecting: connect start!",
            set_svr_name(svr), set_svr_set_name(set), conn->m_fd);
        set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_connected);
        return;
    }
}

static void set_svr_set_conn_fsm_connecting_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_set_conn_t set_conn = fsm_machine_context(fsm);
    set_svr_set_conn_stop_state_timer(set_conn);
}

static uint32_t set_svr_set_conn_fsm_connecting_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct set_svr_set_conn_fsm_evt * evt = input_evt;
    set_svr_set_conn_t conn = fsm_machine_context(fsm);
    set_svr_set_t set = conn->m_set;
    set_svr_t svr = set->m_svr;

    switch(evt->m_type) {
    case set_svr_set_conn_fsm_evt_disconnected:
        set_svr_set_conn_free(conn);
        return FSM_DESTORIED_STATE;

    case set_svr_set_conn_fsm_evt_connected:
        return set_svr_set_conn_state_registing;

    case set_svr_set_conn_fsm_evt_timeout:
        CPE_ERROR(
            svr->m_em, "%s: set %s: fd %d: connecting timeout!",
            set_svr_name(svr), set_svr_set_name(set), conn->m_fd);
        set_svr_set_conn_free(conn);
        return FSM_DESTORIED_STATE;

    case set_svr_set_conn_fsm_evt_wb_update:
        return FSM_KEEP_STATE;
        
    default:
        return FSM_INVALID_STATE;
    }
}

int set_svr_set_conn_fsm_create_connecting(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "connecting", set_svr_set_conn_state_connecting);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_connecting: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, set_svr_set_conn_fsm_connecting_enter, set_svr_set_conn_fsm_connecting_leave);

    if (fsm_def_state_add_transition(s, set_svr_set_conn_fsm_connecting_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_connecting: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

static void set_svr_set_conn_connect_cb(EV_P_ ev_io *w, int revents) {
    set_svr_set_conn_t conn = w->data;
    set_svr_set_t set = conn->m_set;
    set_svr_t svr = set->m_svr;
    int err;
    socklen_t err_len;

    err_len = sizeof(err);

    ev_io_stop(svr->m_ev_loop, &conn->m_watcher);

    if (cpe_getsockopt(conn->m_fd, SOL_SOCKET, SO_ERROR, &err, &err_len) == -1) {
        CPE_ERROR(
            svr->m_em,
            "%s: set %s: fd %d: check state, getsockopt error, errno=%d (%s)",
            set_svr_name(svr), set_svr_set_name(set), conn->m_fd,
            cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
    }
    else {
        if (err == 0) {
            CPE_INFO(
                svr->m_em, "%s: set %s: fd %d: connect succeed!",
                set_svr_name(svr), set_svr_set_name(set), conn->m_fd);
            set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_connected);
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: set %s: fd %d: connect error, errno=%d (%s)", 
                set_svr_name(svr), set_svr_set_name(set), conn->m_fd,
                err, cpe_sock_errstr(err));
            set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
        }
    }
}
