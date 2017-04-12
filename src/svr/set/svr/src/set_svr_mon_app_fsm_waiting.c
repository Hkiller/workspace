#include <assert.h>
#include "cpe/utils/error.h"
#include "set_svr_mon_app_fsm.h"

static void set_svr_mon_app_fsm_waiting_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_mon_app_t mon_app = fsm_machine_context(fsm);
    set_svr_mon_app_start_state_timer(mon_app, mon_app->m_mon->m_restart_wait_ms);
}

static void set_svr_mon_app_fsm_waiting_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_mon_app_t mon_app = fsm_machine_context(fsm);
    set_svr_mon_app_stop_state_timer(mon_app);
}

static uint32_t set_svr_mon_app_fsm_waiting_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct set_svr_mon_app_fsm_evt * evt = input_evt;

    switch(evt->m_type) {
    case set_svr_mon_app_fsm_evt_start:
    case set_svr_mon_app_fsm_evt_timeout:
        return set_svr_mon_app_state_checking;
    case set_svr_mon_app_fsm_evt_disable:
        return set_svr_mon_app_state_disable;
    default:
        return FSM_INVALID_STATE;
    }
}

int set_svr_mon_app_fsm_create_waiting(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "waiting", set_svr_mon_app_state_waiting);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_waiting: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, set_svr_mon_app_fsm_waiting_enter, set_svr_mon_app_fsm_waiting_leave);

    if (fsm_def_state_add_transition(s, set_svr_mon_app_fsm_waiting_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_waiting: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}
