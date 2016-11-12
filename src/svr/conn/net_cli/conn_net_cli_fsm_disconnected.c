#include <assert.h>
#include "cpe/utils/error.h"
#include "svr/conn/net_cli/conn_net_cli.h"
#include "conn_net_cli_internal_ops.h"

static void conn_net_cli_fsm_disconnected_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct conn_net_cli * cli = fsm_machine_context(fsm);
    conn_net_cli_start_state_timer(cli, cli->m_reconnect_span_ms);
}

static void conn_net_cli_fsm_disconnected_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    struct conn_net_cli * cli = fsm_machine_context(fsm);
    conn_net_cli_stop_state_timer(cli);
}

static uint32_t conn_net_cli_fsm_disconnected_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct conn_net_cli_fsm_evt * evt = input_evt;

    switch(evt->m_type) {
    case conn_net_cli_fsm_evt_stop:
        return conn_net_cli_state_disable;
    case conn_net_cli_fsm_evt_timeout:
        return conn_net_cli_state_connecting;
    case conn_net_cli_fsm_evt_disconnected:
        return FSM_KEEP_STATE;
    default:
        break;
    }

    return FSM_INVALID_STATE;
}

int conn_net_cli_fsm_create_disconnected(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "disconnected", conn_net_cli_state_disconnected);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_disconnected: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, conn_net_cli_fsm_disconnected_enter, conn_net_cli_fsm_disconnected_leave);

    if (fsm_def_state_add_transition(s, conn_net_cli_fsm_disconnected_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_disconnected: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}
