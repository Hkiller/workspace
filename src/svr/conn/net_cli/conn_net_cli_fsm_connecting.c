#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/net/net_connector.h"
#include "gd/app/app_context.h"
#include "svr/conn/net_cli/conn_net_cli.h"
#include "conn_net_cli_internal_ops.h"

static void conn_net_cli_connect_cb(EV_P_ ev_io *w, int revents);

static void conn_net_cli_fsm_connecting_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct conn_net_cli * cli = fsm_machine_context(fsm);
    struct sockaddr_in addr;

    if (conn_net_cli_start_state_timer(cli, 30 * 1000) != 0) {
        CPE_ERROR(cli->m_em, "%s: cli: start timer fail!", conn_net_cli_name(cli));
        conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);
        return;
    }

    conn_net_cli_disconnect(cli);

    assert(cli->m_fd == -1);

    cli->m_fd = cpe_sock_open(AF_INET, SOCK_STREAM, 0);
    if (cli->m_fd == -1) {
        CPE_ERROR(
            cli->m_em, "%s: create socket fail, errno=%d (%s)!",
            conn_net_cli_name(cli), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);
        return;
    }

    if (cpe_sock_set_none_block(cli->m_fd, 1) != 0) {
        CPE_ERROR(
            cli->m_em, "%s: set socket none block fail, errno=%d (%s)!",
            conn_net_cli_name(cli), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(cli->m_port);
    addr.sin_addr.s_addr = inet_addr(cli->m_ip);

    if (cpe_connect(cli->m_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        if (cpe_sock_errno() == EINPROGRESS || cpe_sock_errno() == EWOULDBLOCK) {
            CPE_INFO(cli->m_em, "%s: connect started!", conn_net_cli_name(cli));
            ev_io_init(&cli->m_watcher, conn_net_cli_connect_cb, cli->m_fd, EV_WRITE);
            ev_io_start(cli->m_ev_loop, &cli->m_watcher);
            return;
        }
        else {
            CPE_ERROR(
                cli->m_em, "%s: connect error, errno=%d (%s)",
                conn_net_cli_name(cli), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
            cpe_sock_close(cli->m_fd);
			cli->m_fd = -1;
            conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);
            return;
        }
    }
    else {
        CPE_INFO(cli->m_em, "%s: connect succeed!", conn_net_cli_name(cli));
        conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_connected);
        return;
    }
}

static void conn_net_cli_fsm_connecting_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct conn_net_cli * cli = fsm_machine_context(fsm);
    conn_net_cli_stop_state_timer(cli);
}

static uint32_t conn_net_cli_fsm_connecting_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct conn_net_cli_fsm_evt * evt = input_evt;
    conn_net_cli_t cli = fsm_machine_context(fsm);

    switch(evt->m_type) {
    case conn_net_cli_fsm_evt_stop:
        conn_net_cli_disconnect(cli);
        return conn_net_cli_state_disable;

    case conn_net_cli_fsm_evt_disconnected:
        conn_net_cli_disconnect(cli);
        return conn_net_cli_state_disconnected;

    case conn_net_cli_fsm_evt_connected:
        return conn_net_cli_state_established;

    case conn_net_cli_fsm_evt_timeout:
        CPE_ERROR(cli->m_em, "%s: connect timeout!", conn_net_cli_name(cli));
        conn_net_cli_disconnect(cli);
        return conn_net_cli_state_connecting;

    default:
        return FSM_KEEP_STATE;
    }
}

int conn_net_cli_fsm_create_connecting(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "connecting", conn_net_cli_state_connecting);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_connecting: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, conn_net_cli_fsm_connecting_enter, conn_net_cli_fsm_connecting_leave);

    if (fsm_def_state_add_transition(s, conn_net_cli_fsm_connecting_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_connecting: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

static void conn_net_cli_connect_cb(EV_P_ ev_io *w, int revents) {
    conn_net_cli_t cli = w->data;
    int err;
    socklen_t err_len;

    err_len = sizeof(err);

    ev_io_stop(cli->m_ev_loop, &cli->m_watcher);

    if (cpe_getsockopt(cli->m_fd, SOL_SOCKET, SO_ERROR, (void*)&err, &err_len) == -1) {
        CPE_ERROR(
            cli->m_em,
            "%s: check state, getsockopt error, errno=%d (%s)",
            conn_net_cli_name(cli), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);
    }
    else {
        if (err == 0) {
            CPE_INFO(cli->m_em, "%s: connect succeed!", conn_net_cli_name(cli));
            conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_connected);
        }
        else {
            CPE_ERROR(cli->m_em, "%s: connect error, errno=%d (%s)", conn_net_cli_name(cli), err, cpe_sock_errstr(err));
            conn_net_cli_apply_evt(cli, conn_net_cli_fsm_evt_disconnected);
        }
    }
}
