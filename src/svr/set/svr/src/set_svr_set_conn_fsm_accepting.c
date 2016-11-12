#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/utils/error.h"
#include "cpe/utils/string_utils.h"
#include "set_svr_set_conn_fsm.h"

void set_svr_set_conn_accepting_rw_cb(EV_P_ ev_io *w, int revents);

static void set_svr_set_conn_fsm_accepting_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_set_conn_t conn = fsm_machine_context(fsm);

    set_svr_set_conn_start_state_timer(conn, 30000);

    ev_io_init(&conn->m_watcher, set_svr_set_conn_accepting_rw_cb, conn->m_fd, EV_READ);
    ev_io_start(conn->m_svr->m_ev_loop, &conn->m_watcher);
}

static void set_svr_set_conn_fsm_accepting_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_set_conn_t conn = fsm_machine_context(fsm);
    set_svr_set_conn_stop_state_timer(conn);

    ev_io_stop(conn->m_svr->m_ev_loop, &conn->m_watcher);
}

static uint32_t set_svr_set_conn_fsm_accepting_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct set_svr_set_conn_fsm_evt * evt = input_evt;
    set_svr_set_conn_t conn = fsm_machine_context(fsm);
    set_svr_t svr = conn->m_svr;

    switch(evt->m_type) {
    case set_svr_set_conn_fsm_evt_timeout:
        CPE_ERROR(svr->m_em, "%s: conn %d: accepting: timeout", set_svr_name(svr), conn->m_fd);
        set_svr_set_conn_free(conn);
        return FSM_DESTORIED_STATE;

    case set_svr_set_conn_fsm_evt_disconnected:
        CPE_ERROR(svr->m_em, "%s: conn %d: accepting: disconnected", set_svr_name(svr), conn->m_fd);
        set_svr_set_conn_free(conn);
        return FSM_DESTORIED_STATE;

    case set_svr_set_conn_fsm_evt_accepted:
        CPE_ERROR(svr->m_em, "%s: conn %d: accepting: accept success", set_svr_name(svr), conn->m_fd);
        return set_svr_set_conn_state_established;

    default:
        return FSM_INVALID_STATE;
    }
}

int set_svr_set_conn_fsm_create_accepting(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "accepting", set_svr_set_conn_state_accepting);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_accepting: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, set_svr_set_conn_fsm_accepting_enter, set_svr_set_conn_fsm_accepting_leave);

    if (fsm_def_state_add_transition(s, set_svr_set_conn_fsm_accepting_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_accepting: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

void set_svr_set_conn_accepting_rw_cb(EV_P_ ev_io *w, int revents) {
    set_svr_set_conn_t conn = w->data;
    set_svr_t svr = conn->m_svr;
    set_svr_set_t set;
    char * buf;
    size_t require_size = sizeof(uint16_t);
    int receive_size;
    uint16_t set_id;
    struct sockaddr_in addr;
    socklen_t addr_len;
    char ip_buf[32];
    const char * from_ip;
    
    assert(!(revents & EV_WRITE));

    if (!(revents & EV_READ)) return;

    addr_len = sizeof(addr);
    if (cpe_getpeername(conn->m_fd, (struct sockaddr *)&addr, &addr_len) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: conn %d: accepting: get peername error, errno=%d (%s)!",
            set_svr_name(svr), conn->m_fd, cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
        return;
    }
    from_ip = cpe_str_format_ipv4(ip_buf, sizeof(ip_buf), addr.sin_addr.s_addr);
    
    if (set_svr_set_conn_read_from_net(conn, require_size) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: conn %d: accepting from %s: read data size fail!",
            set_svr_name(svr), conn->m_fd, from_ip);
        set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
        return;
    }

    receive_size = set_svr_set_conn_r_buf(conn, require_size, (void **)&buf);
    if (receive_size < 0) {
        CPE_ERROR(
            svr->m_em, "%s: conn %d: accepting from %s: data size %d error!",
            set_svr_name(svr), conn->m_fd, from_ip, receive_size);
        set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
        return;
    }

    if (receive_size < require_size) return;

    assert(buf);

    CPE_COPY_NTOH16(&set_id, buf);

    set = set_svr_set_find_by_id(svr, set_id);
    if (set == NULL) {
        CPE_ERROR(
            svr->m_em, "%s: conn %d: accepting: set %d not exist!",
            set_svr_name(svr), conn->m_fd, set_id);
        set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
        return;
    }
    assert(conn->m_set == NULL);

    addr_len = sizeof(addr);
    if (cpe_getpeername(conn->m_fd, (struct sockaddr *)&addr, &addr_len) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: conn %d: accepting: get peername error, errno=%d (%s)!",
            set_svr_name(svr), conn->m_fd, cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
        return;
    }

    if (set->m_conn) {
        assert(set->m_conn != conn);
        
        switch(fsm_machine_curent_state(&set->m_conn->m_fsm)) {
        case set_svr_set_conn_state_connecting:
        case set_svr_set_conn_state_accepting:
        case set_svr_set_conn_state_registing:
            CPE_INFO(
                svr->m_em, "%s: conn %d: accepting from %s: set %s already have conn %d in state %d!",
                set_svr_name(svr), conn->m_fd, from_ip, set_svr_set_name(set), conn->m_fd,
                fsm_machine_curent_state(&set->m_conn->m_fsm));
            set_svr_set_conn_apply_evt(set->m_conn, set_svr_set_conn_fsm_evt_disconnected);
            assert(set->m_conn == NULL);
            break;
        case set_svr_set_conn_state_established:
            CPE_INFO(
                svr->m_em, "%s: conn %d: accepting from %s: set %s already have conn %d!",
                set_svr_name(svr), conn->m_fd, from_ip, set_svr_set_name(set), set->m_conn->m_fd);
            set_svr_set_conn_apply_evt(set->m_conn, set_svr_set_conn_fsm_evt_disconnected);
            assert(set->m_conn == NULL);
            break;
        default:
            CPE_ERROR(
                svr->m_em, "%s: conn %d: accepting from %s: set %s conn state %d unknown!",
                set_svr_name(svr), conn->m_fd, from_ip, set_svr_set_name(set),
                fsm_machine_curent_state(&set->m_conn->m_fsm));
            set_svr_set_conn_apply_evt(set->m_conn, set_svr_set_conn_fsm_evt_disconnected);
            assert(set->m_conn == NULL);
            break;
        }
    }

    set_svr_set_conn_set_set(conn, set);
    set_svr_set_conn_r_erase(conn, require_size);
    
    if (svr->m_debug) {
        CPE_INFO(
            svr->m_em, "%s: conn %d: accepting from %s: bind to set %s!",
            set_svr_name(svr), conn->m_fd, from_ip, set_svr_set_name(set));
    }

    set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_accepted);
}
