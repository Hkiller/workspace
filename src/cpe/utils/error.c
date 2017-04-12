#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/error.h"

#ifdef _MSC_VER
#include <windows.h>
#endif

void cpe_error_do_notify(error_monitor_t monitor, const char * fmt, ...) {
    struct error_monitor_node * node = &monitor->m_node;
    va_list args;

    if (node) {
        va_start(args, fmt);

        while(node->m_next) {
            va_list tmp;
            va_copy(tmp, args);
            node->on_error(&monitor->m_curent_location, node->m_context, fmt, tmp);
            va_end(tmp);

            node = node->m_next;
        }

        node->on_error(&monitor->m_curent_location, node->m_context, fmt, args);

        va_end(args);
    }
}

void cpe_error_do_notify_var(error_monitor_t monitor, const char * fmt, va_list args) {
    struct error_monitor_node * node = &monitor->m_node;

    while(node) {
        if (node->m_next) {
            va_list tmp;
            va_copy(tmp, args);
            node->on_error(&monitor->m_curent_location, node->m_context, fmt, tmp);
            va_end(tmp);
        }
        else {
            node->on_error(&monitor->m_curent_location, node->m_context, fmt, args);
        }
        node = node->m_next;
    }
}

void cpe_error_monitor_add_node(error_monitor_t monitor, struct error_monitor_node * node) {
    struct error_monitor_node * lastNode;

    if (monitor == NULL) {
        return;
    }

    lastNode = &monitor->m_node;
    while(lastNode->m_next) {
        lastNode = lastNode->m_next;
    }

    lastNode->m_next = node;
    node->m_next = NULL;
}

void cpe_error_monitor_remove_node(error_monitor_t monitor, struct error_monitor_node * removeNode) {
    struct error_monitor_node * node;

    if (monitor == NULL) {
        return;
    }

    node = &monitor->m_node;
    while(node->m_next) {
        if (node->m_next == removeNode) {
            node->m_next = removeNode->m_next;
        }
        else {
            node = node->m_next;
        }
    }
}

void cpe_error_log_to_file(struct error_info * info, void * context, const char * fmt, va_list args) {
    if (context == NULL) return;

    if (info->m_file) fprintf((FILE *)context, "%s:%d: ", info->m_file, info->m_line > 0 ? info->m_line : 0);
    else if (info->m_line >= 0) fprintf((FILE *)context, "%d: ", info->m_line);

    vfprintf((FILE *)context, fmt, args);
}

void cpe_error_log_to_consol(struct error_info * info, void * context, const char * fmt, va_list args) {
#if defined(_MSC_VER)
	char buf[1024];
    size_t s;
    s = 0;

    if (info->m_file) s += snprintf(buf, sizeof(buf), "%s(%d): ", info->m_file, info->m_line > 0 ? info->m_line : 0);
    else if (info->m_line >= 0) s += snprintf(buf + s, sizeof(buf) - s, "%d: ", info->m_line);

    vsnprintf(buf + s, sizeof(buf) - s, fmt, args);
    OutputDebugStringA(buf);
    OutputDebugStringA("\n");
#endif
    if (info->m_file) printf("%s:%d: ", info->m_file, info->m_line > 0 ? info->m_line : 0);
    else if (info->m_line >= 0) printf("%d: ", info->m_line);

    vprintf(fmt, args);
    printf("\n");
}

void cpe_error_log_to_consol_and_flush(struct error_info * info, void * context, const char * fmt, va_list args) {
    cpe_error_log_to_consol(info, context, fmt, args);
    fflush(stdout);
}

void cpe_error_save_last_errno(struct error_info * info, void * context, const char * fmt, va_list args) {
    if (info->m_level == CPE_EL_ERROR) {
        *((int*)context) = info->m_errno;
    }
}

void cpe_error_monitor_node_init(
    struct error_monitor_node * node, 
    void (*on_error)(struct error_info * info, void * context, const char * fmt, va_list args),
    void * context)
{
    node->on_error = on_error;
    node->m_context = context;
    node->m_next = NULL;
}


void cpe_error_monitor_init(
    error_monitor_t monitor, 
    void (*on_error)(struct error_info * info, void * context, const char * fmt, va_list args),
    void * context)
{
    monitor->m_node.on_error = on_error;
    monitor->m_node.m_context = context;
    monitor->m_node.m_next = NULL;

    monitor->m_curent_location.m_file = NULL;
    monitor->m_curent_location.m_line = -1;
    monitor->m_curent_location.m_errno = 0;
    monitor->m_curent_location.m_level = CPE_EL_ERROR;
}
