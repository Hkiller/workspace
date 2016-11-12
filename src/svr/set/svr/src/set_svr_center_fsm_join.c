#include <assert.h>
#include "cpe/utils/error.h"
#include "cpe/utils/string_utils.h"
#include "set_svr_center.h"
#include "set_svr_center_fsm.h"
#include "set_svr_svr_ins.h"
#include "set_svr_svr_ins.h"
#include "set_svr_listener.h"

static void set_svr_center_fsm_join_enter(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_center_t center = fsm_machine_context(fsm);
    set_svr_t svr = center->m_svr;
    SVR_CENTER_PKG * pkg;
    SVR_CENTER_REQ_JOIN * join;
    size_t pkg_capacity;
    set_svr_svr_ins_t svr_ins;

    set_svr_center_start_state_timer(center, 30000);

    pkg_capacity = sizeof(SVR_CENTER_PKG) + sizeof(SVR_CENTER_SVR_ID) * svr->m_local_svr_count;

    assert(center->m_svr->m_listener);
    
    pkg = set_svr_center_get_pkg_buff(center, pkg_capacity);
    if (pkg == NULL) {
        CPE_ERROR(svr->m_em, "%s: send join: get pkg buf fail!", set_svr_name(svr));
        return;
    }

    join = &pkg->data.svr_center_req_join;

    pkg->cmd = SVR_CENTER_CMD_REQ_JOIN;

    join->set.id = center->m_svr->m_set_id;
    join->set.region = center->m_svr->m_region;
    join->set.port = center->m_svr->m_listener->m_port;
    cpe_str_dup(join->set.ip, sizeof(join->set.ip), center->m_svr->m_listener->m_ip);

    join->svr_count = 0;
    TAILQ_FOREACH(svr_ins, &svr->m_local_svrs, m_next_for_local) {
        SVR_CENTER_SVR_ID * runing_svr = &join->svrs[join->svr_count++];
        runing_svr->svr_type = svr_ins->m_svr_type->m_svr_type_id;
        runing_svr->svr_id = svr_ins->m_svr_id;
    }

    if (set_svr_center_send(center, pkg, pkg_capacity) != 0) {
        CPE_ERROR(svr->m_em, "%s: send join req fail!", set_svr_name(svr));
    }
    else {
        if (svr->m_debug) {
            CPE_INFO(svr->m_em, "%s: send join!", set_svr_name(svr));
        }
    }
}

static void set_svr_center_fsm_join_leave(fsm_machine_t fsm, fsm_def_state_t state, void * event) {
    set_svr_center_t center = fsm_machine_context(fsm);
    set_svr_center_stop_state_timer(center);
}

static uint32_t set_svr_center_fsm_join_trans(fsm_machine_t fsm, fsm_def_state_t state, void * input_evt) {
    struct set_svr_center_fsm_evt * evt = input_evt;
    set_svr_center_t center = fsm_machine_context(fsm);
    set_svr_t svr = center->m_svr;
    SVR_CENTER_RES_JOIN const * join_res;

    switch(evt->m_type) {
    case set_svr_center_fsm_evt_pkg:
        if (evt->m_pkg->cmd == SVR_CENTER_CMD_RES_JOIN) {
            break;
        }
        else {
            return FSM_INVALID_STATE;
        }
    case set_svr_center_fsm_evt_timeout:
        CPE_ERROR(svr->m_em, "%s: recv join: timeout", set_svr_name(svr));
        return set_svr_center_state_join;
    case set_svr_center_fsm_evt_disconnected:
        return set_svr_center_state_connecting;
    case set_svr_center_fsm_evt_wb_update:
        ev_io_stop(center->m_svr->m_ev_loop, &center->m_watcher);
        set_svr_center_start_watch(center);
        return FSM_KEEP_STATE;
    default:
        return FSM_INVALID_STATE;
    }

    join_res = &evt->m_pkg->data.svr_center_res_join;
    if (join_res->result != 0) {
        CPE_ERROR(svr->m_em, "%s: recv join: error, result=%d", set_svr_name(svr), join_res->result);
        return set_svr_center_state_disconnected;
    }

    /* for(svr_i = 0; svr_i < join_res->svr_type_count; ++svr_i) { */
    /*     SVR_CENTER_SVR_TYPE const * svr_type = &join_res->svr_types[svr_i]; */
    /*     set_svr_data_group_t group; */

    /*     group = set_svr_data_group_find(svr, svr_type->svr_type_id); */
    /*     if (group == NULL) { */
    /*         group = set_svr_data_group_create(svr, svr_type->svr_type_id); */
    /*         if (group == NULL) { */
    /*             CPE_ERROR( */
    /*                 svr->m_em, "%s: recv join: create svr_group of type %d fail", */
    /*                 set_svr_name(svr), svr_type->svr_type_id); */
    /*         } */
    /*     } */
    /* } */

    return set_svr_center_state_idle;
}

int set_svr_center_fsm_create_join(fsm_def_machine_t fsm_def, error_monitor_t em) {
    fsm_def_state_t s = fsm_def_state_create_ex(fsm_def, "join", set_svr_center_state_join);
    if (s == NULL) {
        CPE_ERROR(em, "%s: fsm_create_join: create state fail!", fsm_def_machine_name(fsm_def));
        return -1;
    }

    fsm_def_state_set_action(s, set_svr_center_fsm_join_enter, set_svr_center_fsm_join_leave);

    if (fsm_def_state_add_transition(s, set_svr_center_fsm_join_trans) != 0) {
        CPE_ERROR(em, "%s: fsm_create_join: add trans fail!", fsm_def_machine_name(fsm_def));
        fsm_def_state_free(s);
        return -1;
    }

    return 0;
}
