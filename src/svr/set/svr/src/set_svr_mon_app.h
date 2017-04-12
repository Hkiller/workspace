#ifndef SVR_SET_SVR_TYPES_MON_APP_H
#define SVR_SET_SVR_TYPES_MON_APP_H
#include "set_svr_mon.h"

typedef enum set_svr_mon_app_state {
    set_svr_mon_app_state_disable
    , set_svr_mon_app_state_runing
    , set_svr_mon_app_state_waiting
    , set_svr_mon_app_state_checking
} set_svr_mon_app_state_t;

struct set_svr_mon_app {
    set_svr_mon_t m_mon;
    uint8_t m_svr_type_count;
    set_svr_svr_type_t m_svr_types[16];
    char * m_name;
    char * m_bin;
    char * m_pidfile;
    char ** m_args;
    size_t m_arg_count;
    size_t m_arg_capacity;
    uint64_t m_rq_size;
    uint64_t m_wq_size;

    struct fsm_machine m_fsm;
    gd_timer_id_t m_fsm_timer_id;
    pid_t m_pid;
    uint32_t m_last_start_time;

    TAILQ_ENTRY(set_svr_mon_app) m_next;
};

/*operations of set_svr_mon_app*/
set_svr_mon_app_t set_svr_mon_app_create(
    set_svr_mon_t mon,
    const char * name,
    const char * bin, const char * pidfile,
    uint64_t rq_size, uint64_t wq_size);

int set_svr_mon_app_add_svr_type(
    set_svr_mon_app_t set_svr_mon_app_t, set_svr_svr_type_t svr_type);

void set_svr_mon_app_free(set_svr_mon_app_t mon_app);
set_svr_mon_app_t set_svr_mon_app_find_by_pid(set_svr_mon_t mon, int pid);
const char * set_svr_mon_app_pid_file(set_svr_mon_app_t mon_app, char * buf, size_t buf_capacity);
void set_svr_mon_app_start_all(set_svr_mon_t mon_app);
int set_svr_mon_app_add_arg(set_svr_mon_app_t mon_app, const char * arg);

enum set_svr_mon_app_get_pid_result {
    set_svr_mon_app_get_pid_ok
    , set_svr_mon_app_get_pid_not_runing
    , set_svr_mon_app_get_pid_error
};
enum set_svr_mon_app_get_pid_result set_svr_mon_app_get_pid(set_svr_mon_app_t mon_app, int * pid);
int set_svr_mon_app_kill(set_svr_mon_app_t mon_app, int sig);

#endif
