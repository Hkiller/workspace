#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/utils/error.h"
#include "set_svr_set_conn_fsm.h"
#include "set_svr_listener.h"

void set_svr_set_conn_registing_rw_cb(EV_P_ ev_io *w, int revents);

static void set_svr_set_conn_fsm_registing_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_set_conn_t conn = fsm_machine_context(fsm);
    set_svr_set_t set = conn->m_set;
    set_svr_t svr;
    ringbuffer_block_t blk;
    char * buf;

    assert(set);
    assert(set->m_svr);

    svr = set->m_svr;
    
    set_svr_set_conn_start_state_timer(conn, 30000);
    
    ev_io_init(&conn->m_watcher, set_svr_set_conn_registing_rw_cb, conn->m_fd, EV_WRITE);
    ev_io_start(svr->m_ev_loop, &conn->m_watcher);

    blk = set_svr_ringbuffer_alloc(svr, sizeof(uint16_t), conn->m_conn_id);
    if (blk == NULL) {
        CPE_ERROR(svr->m_em, "%s: conn %d: registing: alloc ringbuffer fail!", set_svr_name(svr), conn->m_fd);
        set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
        return;
    }

    buf = NULL;
    ringbuffer_data(svr->m_ringbuf, blk, sizeof(uint16_t), 0, (void*)&buf);
    assert(buf);

    assert(svr->m_listener);
    CPE_COPY_HTON16(buf, &svr->m_set_id);

    set_svr_set_conn_link_node_w_head(conn, blk);
}

static void set_svr_set_conn_fsm_registing_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_set_conn_t conn = fsm_machine_context(fsm);
    set_svr_set_conn_stop_state_timer(conn);

    ev_io_stop(conn->m_svr->m_ev_loop, &conn->m_watcher);
}

static uint32_t set_svr_set_conn_fsm_registing_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct set_svr_set_conn_fsm_evt * evt = input_evt;
    set_svr_set_conn_t conn = fsm_machine_context(fsm);
    set_svr_t svr = conn->m_svr;

    switch(evt->m_type) {
    case set_svr_set_conn_fsm_evt_timeout:
        CPE_ERROR(svr->m_em, "%s: conn %d: registing: timeout", set_svr_name(svr), conn->m_fd);
        set_svr_set_conn_free(conn);
        return FSM_DESTORIED_STATE;

    case set_svr_set_conn_fsm_evt_disconnected:
        CPE_ERROR(svr->m_em, "%s: conn %d: registing: disconnected", set_svr_name(svr), conn->m_fd);
        set_svr_set_conn_free(conn);
        return FSM_DESTORIED_STATE;

    case set_svr_set_conn_fsm_evt_registed:
        CPE_INFO(svr->m_em, "%s: conn %d: registing: registe success", set_svr_name(svr), conn->m_fd);
        return set_svr_set_conn_state_established;

    default:
        return FSM_INVALID_STATE;
    }
}

int set_svr_set_conn_fsm_create_registing(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "registing", set_svr_set_conn_state_registing);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_registing: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, set_svr_set_conn_fsm_registing_enter, set_svr_set_conn_fsm_registing_leave);

    if (fsm_def_state_add_transition(s, set_svr_set_conn_fsm_registing_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_registing: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

void set_svr_set_conn_registing_rw_cb(EV_P_ ev_io *w, int revents) {
    set_svr_set_conn_t conn = w->data;

    assert(!(revents & EV_READ));

    if (!(revents & EV_WRITE)) return;

    if (set_svr_set_conn_write_to_net(conn) != 0) {
        set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_disconnected);
    }

    if (conn->m_wb == NULL) {
        set_svr_set_conn_apply_evt(conn, set_svr_set_conn_fsm_evt_registed);
    }
}
