#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "log_internal_types.h"

static void log4c_on_error(struct error_info * info, void * context, const char * fmt, va_list args);

int log4c_em_create(struct log_context * context, const char * em_name, log4c_category_t * category) {
    struct log4c_em * log4c_em;
    size_t name_len;

    assert(em_name);

    name_len = strlen(em_name) + 1;

    log4c_em = mem_alloc(context->m_alloc, sizeof(struct log4c_em) + name_len);
    if (log4c_em == NULL) {
        CPE_ERROR(context->m_em, "log4c_em: %s: malloc fail!", em_name);
        return -1;
    }
    memcpy(log4c_em + 1, em_name, name_len);
    log4c_em->m_name = (const char *)(log4c_em + 1);
    log4c_em->m_log4c_category = category;

    cpe_error_monitor_init(&log4c_em->m_em, log4c_on_error, log4c_em);

    if (strcmp(em_name, "default") == 0) {
        gd_app_set_em(context->m_app, &log4c_em->m_em);
    }
    else {
        gd_app_set_named_em(context->m_app, em_name, &log4c_em->m_em);
    }

    return 0;
}

static void log4c_em_free(struct log_context * context, struct log4c_em * log4c_em) {
    gd_app_context_t app;

    app = context->m_app;

    TAILQ_REMOVE(&context->m_log4c_ems, log4c_em, m_next);

    if (log4c_em->m_name) {
        if (gd_app_named_em(app, log4c_em->m_name) == &log4c_em->m_em) {
            gd_app_remove_named_em(app, log4c_em->m_name);
        }
    }
    else {
        if (gd_app_em(app) == &log4c_em->m_em) {
            gd_app_set_em(app, gd_app_print_em(app));
        }
    }

    mem_free(context->m_alloc, log4c_em);
}

void log4c_em_free_all(struct log_context * context) {
    while(!TAILQ_EMPTY(&context->m_log4c_ems)) {
        log4c_em_free(context, TAILQ_FIRST(&context->m_log4c_ems));
    }
}

static void log4c_on_error(struct error_info * info, void * context, const char * fmt, va_list args) {
    struct log4c_em * log4c_em;
    const log4c_location_info_t locinfo = { info->m_file, info->m_line, "(nil)", context };
    int priority = LOG4C_PRIORITY_NOTSET;

    log4c_em = (struct log4c_em *)context;

    switch (info->m_level) {
    case CPE_EL_INFO:
        priority = LOG4C_PRIORITY_INFO;
        break;
    case CPE_EL_WARNING:
        priority = LOG4C_PRIORITY_WARN;
        break;
    case CPE_EL_ERROR:
    default:
        priority = LOG4C_PRIORITY_ERROR;
        break;
    }

    __log4c_category_vlog(log4c_em->m_log4c_category, &locinfo, priority, fmt, args);
}
