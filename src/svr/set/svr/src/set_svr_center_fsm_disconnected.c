#include <assert.h>
#include "cpe/utils/error.h"
#include "set_svr_center_fsm.h"

static void set_svr_center_fsm_disconnected_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_center_t center = fsm_machine_context(fsm);
    set_svr_center_start_state_timer(center, center->m_reconnect_span_ms);

    if (center->m_rb) {
        ringbuffer_free(center->m_svr->m_ringbuf, center->m_rb);
        center->m_rb = NULL;
    }

    if (center->m_wb) {
        ringbuffer_free(center->m_svr->m_ringbuf, center->m_wb);
        center->m_wb = NULL;
    }

    if (center->m_tb) {
        ringbuffer_free(center->m_svr->m_ringbuf, center->m_tb);
        center->m_tb = NULL;
    }
}

static void set_svr_center_fsm_disconnected_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_center_t center = fsm_machine_context(fsm);
    set_svr_center_stop_state_timer(center);
}

static uint32_t set_svr_center_fsm_disconnected_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct set_svr_center_fsm_evt * evt = input_evt;

    switch(evt->m_type) {
    case set_svr_center_fsm_evt_connected:
        return set_svr_center_state_join;
    case set_svr_center_fsm_evt_timeout:
        return set_svr_center_state_connecting;
    case set_svr_center_fsm_evt_disconnected:
        return FSM_KEEP_STATE;
    default:
        break;
    }

    return FSM_INVALID_STATE;
}

int set_svr_center_fsm_create_disconnected(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "disconnected", set_svr_center_state_disconnected);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_disconnected: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, set_svr_center_fsm_disconnected_enter, set_svr_center_fsm_disconnected_leave);

    if (fsm_def_state_add_transition(s, set_svr_center_fsm_disconnected_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_disconnected: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}
