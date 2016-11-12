#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/net/net_manage.h"
#include "net_internal_ops.h"

void net_mgr_break(net_mgr_t nmgr) {
    ev_break(nmgr->m_ev_loop, EVBREAK_ONE);
}

void net_mgr_stop(net_mgr_t nmgr) {
    ev_break(nmgr->m_ev_loop, EVBREAK_ALL);
}

int net_mgr_tick(net_mgr_t nmgr) {
    unsigned int old_value = ev_iteration(nmgr->m_ev_loop);
    ev_run(nmgr->m_ev_loop, EVRUN_NOWAIT);
    return ev_iteration(nmgr->m_ev_loop) - old_value - 1;
}

struct tick_env {
    net_run_tick_fun_t m_tick_fun;
    void * m_tick_ctx;
};

static void net_mgr_run_tick_cb(EV_P_ ev_timer *w, int revents) {
    struct tick_env * tick_env = (struct tick_env *)w->data;

    assert(tick_env);
    assert(tick_env->m_tick_fun);

    tick_env->m_tick_fun(tick_env->m_tick_ctx);
}

int net_mgr_run(net_mgr_t nmgr, int64_t span, net_run_tick_fun_t tick_fun, void * tick_ctx) {
    struct tick_env tick_env;
    struct ev_timer tick_timer;
    ev_tstamp ev_span;

    if (tick_fun) {
        if (span <= 0) return -1;

        tick_env.m_tick_fun = tick_fun;
        tick_env.m_tick_ctx = tick_ctx;

        tick_timer.data = &tick_env;

        ev_span = ((ev_tstamp)span) / 10000.0;
        ev_timer_init (&tick_timer, net_mgr_run_tick_cb, ev_span, ev_span);
        ev_timer_start(nmgr->m_ev_loop, &tick_timer);
    }
    
    ev_run(nmgr->m_ev_loop, 0);

    if (tick_fun) {
        ev_timer_stop(nmgr->m_ev_loop, &tick_timer);
    }

    return 0;
}
