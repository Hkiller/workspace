#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "gd/app/app_context.h"
#include "svr/conn/net_cli/conn_net_cli.h"
#include "conn_net_cli_internal_ops.h"

static void conn_net_cli_fsm_disable_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct conn_net_cli * cli = fsm_machine_context(fsm);

    conn_net_cli_disconnect(cli);
}


static uint32_t conn_net_cli_fsm_disable_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct conn_net_cli_fsm_evt * evt = input_evt;

    switch(evt->m_type) {
    case conn_net_cli_fsm_evt_start:
        return conn_net_cli_state_connecting;
    case conn_net_cli_fsm_evt_stop:
        return FSM_KEEP_STATE;
    default:
        return FSM_INVALID_STATE;
    }
}

int conn_net_cli_fsm_create_disable(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "disable", conn_net_cli_state_disable);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_disable: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, conn_net_cli_fsm_disable_enter, NULL);

    if (fsm_def_state_add_transition(s, conn_net_cli_fsm_disable_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_disable: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}

