#include <assert.h>
#include "cpe/utils/error.h"
#include "set_svr_center_fsm.h"
#include "set_svr_center_op.h"

static void set_svr_center_fsm_idle_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_center_t center = fsm_machine_context(fsm);
    set_svr_center_start_state_timer(center, center->m_update_span_s * 1000);

    set_svr_center_send_sync_cmd(center);
}

static void set_svr_center_fsm_idle_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_center_t center = fsm_machine_context(fsm);
    set_svr_center_stop_state_timer(center);
}

static uint32_t set_svr_center_fsm_idle_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    set_svr_center_t center = fsm_machine_context(fsm);
    struct set_svr_center_fsm_evt * evt = input_evt;

    switch(evt->m_type) {
    case set_svr_center_fsm_evt_pkg:
        if (evt->m_pkg->cmd == SVR_CENTER_CMD_RES_QUERY) {
            set_svr_center_on_sync_svrs(center, &evt->m_pkg->data.svr_center_res_query);
        }
        else if (evt->m_pkg->cmd == SVR_CENTER_CMD_NTF_LEAVE) {
            set_svr_center_on_ntf_leave(center, &evt->m_pkg->data.svr_center_ntf_leave);
        }
        else if (evt->m_pkg->cmd == SVR_CENTER_CMD_NTF_JOIN) {
            set_svr_center_on_ntf_join(center, &evt->m_pkg->data.svr_center_ntf_join);
        }
        else {
            CPE_ERROR(center->m_svr->m_em, "%s: center: unknown center cmd %d!", set_svr_name(center->m_svr), evt->m_pkg->cmd);
        }
        return FSM_KEEP_STATE;
    case set_svr_center_fsm_evt_timeout:
        set_svr_center_send_sync_cmd(center);
        return FSM_INVALID_STATE;
    case set_svr_center_fsm_evt_disconnected:
        return set_svr_center_state_connecting;
    case set_svr_center_fsm_evt_wb_update:
        ev_io_stop(center->m_svr->m_ev_loop, &center->m_watcher);
        set_svr_center_start_watch(center);
        return FSM_KEEP_STATE;
    default:
        return FSM_INVALID_STATE;
    }
}

int set_svr_center_fsm_create_idle(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "idle", set_svr_center_state_idle);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_idle: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, set_svr_center_fsm_idle_enter, set_svr_center_fsm_idle_leave);

    if (fsm_def_state_add_transition(s, set_svr_center_fsm_idle_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_idle: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}
