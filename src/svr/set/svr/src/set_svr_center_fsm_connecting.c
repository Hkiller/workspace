#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_socket.h"
#include "gd/app/app_context.h"
#include "set_svr_center_fsm.h"

static void set_svr_center_connect_cb(EV_P_ ev_io *w, int revents);

static void set_svr_center_fsm_connecting_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_center_t center = fsm_machine_context(fsm);
    struct set_svr * svr = center->m_svr;
    struct sockaddr_in addr;

    if (set_svr_center_start_state_timer(center, center->m_reconnect_span_ms) != 0) {
        CPE_ERROR(svr->m_em, "%s: center: start timer fail!", set_svr_name(svr));
        set_svr_center_apply_evt(center, set_svr_center_fsm_evt_disconnected);
        return;
    }

    set_svr_center_disconnect(center);
    assert(center->m_fd == -1);

    center->m_fd = cpe_sock_open(AF_INET, SOCK_STREAM, 0);
    if (center->m_fd == -1) {
        CPE_ERROR(
            svr->m_em, "%s: center: create socket fail, errno=%d (%s)!",
            set_svr_name(svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        set_svr_center_apply_evt(center, set_svr_center_fsm_evt_disconnected);
        return;
    }

    if (cpe_sock_set_none_block(center->m_fd, 1) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: center: set socket none block fail, errno=%d (%s)!",
            set_svr_name(svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        set_svr_center_apply_evt(center, set_svr_center_fsm_evt_disconnected);
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(center->m_port);
    addr.sin_addr.s_addr = inet_addr(center->m_ip);

    if (cpe_connect(center->m_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        if (cpe_sock_errno() == EINPROGRESS || cpe_sock_errno() == EWOULDBLOCK) {
            CPE_INFO(svr->m_em, "%s: center: connect started to %s:%d!", set_svr_name(svr), center->m_ip, center->m_port);
            ev_io_init(&center->m_watcher, set_svr_center_connect_cb, center->m_fd, EV_WRITE);
            ev_io_start(svr->m_ev_loop, &center->m_watcher);
            return;
        }
        else {
            CPE_ERROR(
                svr->m_em, "%s: center: connect error, errno=%d (%s)",
                set_svr_name(svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
            cpe_sock_close(center->m_fd);
            set_svr_center_apply_evt(center, set_svr_center_fsm_evt_disconnected);
            return;
        }
    }
    else {
        CPE_INFO(svr->m_em, "%s: center: connect succeed!", set_svr_name(svr));
        set_svr_center_apply_evt(center, set_svr_center_fsm_evt_connected);
        return;
    }
}

static void set_svr_center_fsm_connecting_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_center_t center = fsm_machine_context(fsm);
    set_svr_center_stop_state_timer(center);
}

static uint32_t set_svr_center_fsm_connecting_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct set_svr_center_fsm_evt * evt = input_evt;
    set_svr_center_t center = fsm_machine_context(fsm);
    set_svr_t agent = center->m_svr;

    switch(evt->m_type) {
    case set_svr_center_fsm_evt_stop:
        set_svr_center_disconnect(center);
        return set_svr_center_state_disable;
    case set_svr_center_fsm_evt_disconnected:
        set_svr_center_disconnect(center);
        return set_svr_center_state_disconnected;
    case set_svr_center_fsm_evt_connected:
        return set_svr_center_state_join;
    case set_svr_center_fsm_evt_timeout:
        CPE_ERROR(agent->m_em, "%s: center: connecting timeout!", set_svr_name(agent));
        set_svr_center_disconnect(center);
        return set_svr_center_state_connecting;
    default:
        return FSM_INVALID_STATE;
    }
}

int set_svr_center_fsm_create_connecting(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "connecting", set_svr_center_state_connecting);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_connecting: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, set_svr_center_fsm_connecting_enter, set_svr_center_fsm_connecting_leave);

    if (fsm_def_state_add_transition(s, set_svr_center_fsm_connecting_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_connecting: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

static void set_svr_center_connect_cb(EV_P_ ev_io *w, int revents) {
    set_svr_center_t center = w->data;
    set_svr_t svr = center->m_svr;
    int err;
    socklen_t err_len;

    err_len = sizeof(err);

    ev_io_stop(svr->m_ev_loop, &center->m_watcher);

    if (cpe_getsockopt(center->m_fd, SOL_SOCKET, SO_ERROR, &err, &err_len) == -1) {
        CPE_ERROR(
            svr->m_em,
            "%s: check state, getsockopt error, errno=%d (%s)",
            set_svr_name(svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        set_svr_center_apply_evt(center, set_svr_center_fsm_evt_disconnected);
    }
    else {
        if (err == 0) {
            CPE_INFO(svr->m_em, "%s: connect succeed!", set_svr_name(svr));
            set_svr_center_apply_evt(center, set_svr_center_fsm_evt_connected);
        }
        else {
            CPE_ERROR(svr->m_em, "%s: connect error, errno=%d (%s)", set_svr_name(svr), err, cpe_sock_errstr(err));
            set_svr_center_apply_evt(center, set_svr_center_fsm_evt_disconnected);
        }
    }
}
