#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include "cpe/pal/pal_external.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "set_svr_mon.h"
#include "set_svr_mon_app_fsm.h"

static void set_svr_mon_sig_child_handler(int sig);
static void set_svr_mon_stop_all_apps(set_svr_mon_t mon);
static void set_svr_mon_process_wait_child(void * ctx, gd_timer_id_t timer_id, void * arg);
set_svr_mon_t g_set_svr_mon = NULL;

set_svr_mon_t set_svr_mon_create(set_svr_t svr) {
    set_svr_mon_t mon;
    gd_timer_mgr_t timer_mgr;

    assert(g_set_svr_mon == NULL);

    timer_mgr = gd_timer_mgr_default(svr->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: mon: start wait timer: get default timer manager fail", set_svr_name(svr));
        return NULL;
    }

    mon = mem_alloc(svr->m_alloc, sizeof(struct set_svr_mon));
    if (mon == NULL) {
        CPE_ERROR(svr->m_em, "%s: mon: alloc fail!", set_svr_name(svr));
        return NULL;
    }

    mon->m_svr = svr;
    mon->m_restart_wait_ms = 0;
    mon->m_debug = 0;
    mon->m_stop_apps = 0;
    mon->m_have_stop_apps = 0;
    mon->m_wait_timer_id = 0;

    mon->m_fsm_def = set_svr_mon_app_create_fsm_def(set_svr_name(svr), svr->m_alloc, svr->m_em);
    if (mon->m_fsm_def == NULL) {
        CPE_ERROR(svr->m_em, "%s: mon: create fsm def fail!", set_svr_name(svr));
        mem_free(svr->m_alloc, mon);
        return NULL;
    }

    if (gd_timer_mgr_regist_timer(timer_mgr, &mon->m_wait_timer_id, set_svr_mon_process_wait_child, mon, NULL, NULL, 100, 100, -1) != 0) {
        assert(mon->m_wait_timer_id == GD_TIMER_ID_INVALID);
        CPE_ERROR(svr->m_em, "%s: mon: start wait timer: regist timer fail", set_svr_name(svr));
        fsm_def_machine_free(mon->m_fsm_def);
        mem_free(svr->m_alloc, mon);
        return NULL;
    }


    TAILQ_INIT(&mon->m_mon_apps);

    g_set_svr_mon = mon;
    signal(SIGCHLD, set_svr_mon_sig_child_handler);

    return mon;
}

void set_svr_mon_free(set_svr_mon_t mon) {
    set_svr_t svr = mon->m_svr;
    gd_timer_mgr_t timer_mgr;

    timer_mgr = gd_timer_mgr_default(svr->m_app);
    assert(timer_mgr);
    gd_timer_mgr_unregist_timer_by_id(timer_mgr, mon->m_wait_timer_id);

    assert(g_set_svr_mon == mon);
    g_set_svr_mon = NULL;

    signal(SIGCHLD, NULL);

    if (mon->m_stop_apps) {
        set_svr_mon_stop_all_apps(mon);        
    }
    
    while(!TAILQ_EMPTY(&mon->m_mon_apps)) {
        set_svr_mon_app_free(TAILQ_FIRST(&mon->m_mon_apps));
    }

    fsm_def_machine_free(mon->m_fsm_def);
    mon->m_fsm_def = NULL;
    
    mem_free(svr->m_alloc, mon);
}

static void set_svr_mon_sig_child_handler(int sig) {
    g_set_svr_mon->m_have_stop_apps = 1;
}

static void set_svr_mon_stop_all_apps(set_svr_mon_t mon) {
    set_svr_mon_app_t mon_app;
    uint16_t success_count = 0;
    uint16_t fail_count = 0;
    
    TAILQ_FOREACH(mon_app, &mon->m_mon_apps, m_next) {
        if (mon_app->m_pid) {
            if (kill((pid_t)mon_app->m_pid, SIGUSR1) != 0) {
                fail_count++;
                CPE_ERROR(
                    mon->m_svr->m_em, "set_svr_mon_stop_all_apps: kill app %s(%d) signal SIGUSR1(%d) fail, error=%d (%s)!",
                    mon_app->m_name, mon_app->m_pid, SIGUSR1, errno, strerror(errno));
            }
            else {
                success_count++;
                CPE_INFO(
                    mon->m_svr->m_em, "set_svr_mon_stop_all_apps: kill app %s(%d) signal SIGUSR1(%d) success!",
                    mon_app->m_name, mon_app->m_pid, SIGUSR1);
            }
        }
    }
    
    CPE_INFO(mon->m_svr->m_em, "set_svr_mon_stop_all_apps: kill apps success=%d, fail=%d", success_count, fail_count);
}

#ifndef WCONTINUED
# define WCONTINUED 0
#endif

static void set_svr_mon_process_wait_child(void * ctx, gd_timer_id_t timer_id, void * arg) {
    set_svr_mon_t mon = ctx;
    int pid, status;
    set_svr_t svr;
    set_svr_mon_app_t mon_app;

    assert(mon);
    svr = mon->m_svr;

    while(mon->m_have_stop_apps) {
        /* some systems define WCONTINUED but then fail to support it (linux 2.4) */
        if (0 >= (pid = waitpid (-1, &status, WNOHANG | WUNTRACED | WCONTINUED))) {
            if (errno == ECHILD) {
                mon->m_have_stop_apps = 0;
                break;
            }

            if (!WCONTINUED
                || errno == EINVAL
                || 0 >= (pid = waitpid (-1, &status, WNOHANG | WUNTRACED)))
            {
                if (mon->m_debug) {
                    CPE_INFO(svr->m_em, "%s: sig child: waitpid fail, error=%d (%s)", set_svr_name(svr), errno, strerror(errno));
                }
                return;
            }
        }

        mon_app = set_svr_mon_app_find_by_pid(mon, pid);
        if (mon_app == NULL) {
            CPE_ERROR(svr->m_em, "%s: sig child: no mon app with id %d", set_svr_name(svr), pid);
            return;
        }

        if (svr->m_debug) {
            CPE_INFO(svr->m_em, "%s: sig child: mon app %s: stop, pid=%d, status=%d", set_svr_name(svr), mon_app->m_bin, pid, status);
        }

        set_svr_mon_app_apply_evt(mon_app, set_svr_mon_app_fsm_evt_stoped);
    }
}
