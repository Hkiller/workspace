#ifndef SVR_SET_SVR_TYPES_MON_APP_FSM_H
#define SVR_SET_SVR_TYPES_MON_APP_FSM_H
#include "set_svr_mon_app.h"

enum set_svr_mon_app_fsm_evt_type {
    set_svr_mon_app_fsm_evt_enable
    , set_svr_mon_app_fsm_evt_disable
    , set_svr_mon_app_fsm_evt_start
    , set_svr_mon_app_fsm_evt_stoped
    , set_svr_mon_app_fsm_evt_timeout
};

struct set_svr_mon_app_fsm_evt {
    enum set_svr_mon_app_fsm_evt_type m_type;
};

/*operations of mon_app fsm*/
void set_svr_mon_app_apply_evt(struct set_svr_mon_app * mon_app, enum set_svr_mon_app_fsm_evt_type type);
int set_svr_mon_app_start_state_timer(struct set_svr_mon_app * mon_app, tl_time_span_t span);
void set_svr_mon_app_stop_state_timer(struct set_svr_mon_app * mon_app);

fsm_def_machine_t set_svr_mon_app_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em);
int set_svr_mon_app_fsm_create_disable(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_mon_app_fsm_create_checking(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_mon_app_fsm_create_runing(fsm_def_machine_t fsm_def, error_monitor_t em);
int set_svr_mon_app_fsm_create_waiting(fsm_def_machine_t fsm_def, error_monitor_t em);

#endif
