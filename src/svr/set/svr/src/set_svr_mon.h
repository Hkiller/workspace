#ifndef SVR_SET_SVR_TYPES_MON_H
#define SVR_SET_SVR_TYPES_MON_H
#include <unistd.h>
#include "set_svr.h"

typedef TAILQ_HEAD(set_svr_mon_app_list, set_svr_mon_app) set_svr_mon_app_list_t;

struct set_svr_mon {
    set_svr_t m_svr;
    int m_debug;
    tl_time_span_t m_restart_wait_ms;

    fsm_def_machine_t m_fsm_def;

    uint8_t m_stop_apps;
    uint8_t m_have_stop_apps;
    gd_timer_id_t m_wait_timer_id;

    set_svr_mon_app_list_t m_mon_apps;
};

/*operations of set_svr_mon*/
set_svr_mon_t set_svr_mon_create(set_svr_t svr);
void set_svr_mon_free(set_svr_mon_t mon);

int set_svr_app_init_mon(set_svr_mon_t mon, cfg_t set_cfg);


#endif
