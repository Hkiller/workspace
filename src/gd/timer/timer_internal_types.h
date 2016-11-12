#ifndef GD_TIMER_INTERNAL_TYPES_H
#define GD_TIMER_INTERNAL_TYPES_H
#include "cpe/timer/timer_types.h"
#include "gd/timer/timer_types.h"

struct gd_timer_mgr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;
    cpe_timer_mgr_t m_timer_mgr;
};

#endif

