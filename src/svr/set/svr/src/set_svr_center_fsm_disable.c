#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "gd/app/app_context.h"
#include "set_svr_center_fsm.h"

static void set_svr_center_fsm_disable_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_center_t center = fsm_machine_context(fsm);

    set_svr_center_disconnect(center);
}


static uint32_t set_svr_center_fsm_disable_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct set_svr_center_fsm_evt * evt = input_evt;

    switch(evt->m_type) {
    case set_svr_center_fsm_evt_stop:
        return FSM_KEEP_STATE;
    case set_svr_center_fsm_evt_start:
        return set_svr_center_state_connecting;
    default:
        return FSM_INVALID_STATE;
    }
}

int set_svr_center_fsm_create_disable(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "disable", set_svr_center_state_disable);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_disable: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, set_svr_center_fsm_disable_enter, NULL);

    if (fsm_def_state_add_transition(s, set_svr_center_fsm_disable_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_disable: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

