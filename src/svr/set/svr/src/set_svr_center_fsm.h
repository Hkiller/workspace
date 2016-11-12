#ifndef SVR_SET_SVR_TYPES_CENTER_FSM_H
#define SVR_SET_SVR_TYPES_CENTER_FSM_H
#include "set_svr_center.h"

enum set_svr_center_fsm_evt_type {
    set_svr_center_fsm_evt_pkg
    , set_svr_center_fsm_evt_start
    , set_svr_center_fsm_evt_stop
    , set_svr_center_fsm_evt_timeout
    , set_svr_center_fsm_evt_connected
    , set_svr_center_fsm_evt_disconnected
    , set_svr_center_fsm_evt_wb_update
};

struct set_svr_center_fsm_evt {
    enum set_svr_center_fsm_evt_type m_type;
    SVR_CENTER_PKG * m_pkg;
};

uint32_t set_svr_center_fsm_trans_common(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt);
int set_svr_center_fsm_create_disable(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_center_fsm_create_connecting(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_center_fsm_create_disconnected(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_center_fsm_create_join(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_center_fsm_create_idle(fsm_def_machine_t fsm_def, error_monitor_t em);

void set_svr_center_apply_evt(set_svr_center_t center, enum set_svr_center_fsm_evt_type type);
void set_svr_center_apply_pkg(set_svr_center_t center, SVR_CENTER_PKG * pkg);

#endif

