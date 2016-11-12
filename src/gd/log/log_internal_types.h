#ifndef GD_LOG_INTERNAL_TYPES_H
#define GD_LOG_INTERNAL_TYPES_H
#include "log4c.h"
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/error.h"
#include "gd/app/app_types.h"

struct log4c_em {
    struct error_monitor m_em;
    TAILQ_ENTRY(log4c_em) m_next;
    log4c_category_t * m_log4c_category;
    const char * m_name;
};

typedef TAILQ_HEAD(log4c_em_list, log4c_em) log4c_em_list_t;

struct log_context {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;

    log4c_em_list_t m_log4c_ems;
};

#endif
