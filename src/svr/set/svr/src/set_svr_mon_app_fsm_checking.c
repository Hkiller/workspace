#include <assert.h>
#include <errno.h>
#include "cpe/utils/error.h"
#include "set_svr_mon_app_fsm.h"

static void set_svr_mon_app_fsm_checking_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_mon_app_t mon_app = fsm_machine_context(fsm);
    set_svr_mon_t mon = mon_app->m_mon;
    set_svr_t svr = mon->m_svr;
    int pid;

    if (set_svr_mon_app_start_state_timer(mon_app, 30000) != 0) {
        CPE_ERROR(
            svr->m_em, "%s: mon app %s: start timer fail!",
            set_svr_name(svr), mon_app->m_bin);
    }

    switch(set_svr_mon_app_get_pid(mon_app, &pid)) {
    case set_svr_mon_app_get_pid_ok:
        if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: mon app %s: read pid %d success",
                set_svr_name(svr), mon_app->m_bin, pid);
        }
        if (mon_app->m_pid != pid) {
            if (mon_app->m_pid) set_svr_mon_app_kill(mon_app, SIGKILL);
            mon_app->m_pid = pid;
        }
        return;
    case set_svr_mon_app_get_pid_not_runing:
        if (svr->m_debug) {
            CPE_INFO(
                svr->m_em, "%s: mon app %s: not runing",
                set_svr_name(svr), mon_app->m_bin);
        }
        set_svr_mon_app_apply_evt(mon_app, set_svr_mon_app_fsm_evt_start);
        return;
    case set_svr_mon_app_get_pid_error:
        CPE_ERROR(
            svr->m_em, "%s: mon app %s: get pid error",
            set_svr_name(svr), mon_app->m_bin);
        return;
    }
}

static void set_svr_mon_app_fsm_checking_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_mon_app_t mon_app = fsm_machine_context(fsm);

    set_svr_mon_app_stop_state_timer(mon_app);
}

static uint32_t set_svr_mon_app_fsm_checking_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct set_svr_mon_app_fsm_evt * evt = input_evt;

    switch(evt->m_type) {
    case set_svr_mon_app_fsm_evt_start:
        return set_svr_mon_app_state_runing;
    case set_svr_mon_app_fsm_evt_timeout:
        return set_svr_mon_app_state_checking;
    case set_svr_mon_app_fsm_evt_stoped:
        return set_svr_mon_app_state_waiting;
    case set_svr_mon_app_fsm_evt_disable:
        return set_svr_mon_app_state_disable;
    default:
        return FSM_INVALID_STATE;
    }
}

int set_svr_mon_app_fsm_create_checking(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "checking", set_svr_mon_app_state_checking);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_checking: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, set_svr_mon_app_fsm_checking_enter, set_svr_mon_app_fsm_checking_leave);

    if (fsm_def_state_add_transition(s, set_svr_mon_app_fsm_checking_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_checking: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}
