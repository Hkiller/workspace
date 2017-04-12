#include <assert.h>
#include <errno.h>
#include "cpe/pal/pal_types.h"
#include "cpe/pal/pal_signal.h"
#include "cpe/utils/error.h"
#include "set_svr_mon_app_fsm.h"

static void set_svr_mon_app_fsm_disable_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct set_svr_mon_app * mon_app = fsm_machine_context(fsm);

    if (mon_app->m_pid) {
        set_svr_mon_app_kill(mon_app, SIGKILL);
        mon_app->m_pid = 0;
    }
}

static void set_svr_mon_app_fsm_disable_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
}

static uint32_t set_svr_mon_app_fsm_disable_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct set_svr_mon_app_fsm_evt * evt = input_evt;

    switch(evt->m_type) {
    case set_svr_mon_app_fsm_evt_enable:
        return set_svr_mon_app_state_checking;
    default:
        return FSM_INVALID_STATE;
    }
}

int set_svr_mon_app_fsm_create_disable(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "disable", set_svr_mon_app_state_disable);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_disable: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, set_svr_mon_app_fsm_disable_enter, set_svr_mon_app_fsm_disable_leave);

    if (fsm_def_state_add_transition(s, set_svr_mon_app_fsm_disable_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_disable: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}
