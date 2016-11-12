#ifndef CPE_UTILS_ERRORMONITOR_H
#define CPE_UTILS_ERRORMONITOR_H
#include "cpe/pal/pal_stdarg.h"
#include "utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define __attribute__(a)
#endif

typedef enum {
    CPE_EL_INFO,
    CPE_EL_WARNING,
    CPE_EL_ERROR
} error_level_t;

struct error_info {
    const char * m_file;
    int m_line;
    int m_errno;
    error_level_t m_level;
};

struct error_monitor_node {
    void (*on_error)(struct error_info * info, void * context, const char * fmt, va_list args);
    void * m_context;
    struct error_monitor_node * m_next;
};

struct error_monitor {
    struct error_monitor_node m_node;
    struct error_info m_curent_location;
};

/*utils functions*/
void cpe_error_log_to_file(struct error_info * info, void * context, const char * fmt, va_list args);
void cpe_error_log_to_consol(struct error_info * info, void * context, const char * fmt, va_list args);
void cpe_error_log_to_consol_and_flush(struct error_info * info, void * context, const char * fmt, va_list args);
void cpe_error_save_last_errno(struct error_info * info, void * context, const char * fmt, va_list args);

/*operations*/
void cpe_error_do_notify(error_monitor_t monitor, const char * fmt, ...) __attribute__((format(printf,2,3)));
void cpe_error_do_notify_var(error_monitor_t monitor, const char * fmt, va_list args);

void cpe_error_monitor_node_init(
    struct error_monitor_node * node, 
    void (*on_error)(struct error_info * info, void * context, const char * fmt, va_list args),
    void * context);
void cpe_error_monitor_init(
    error_monitor_t monitor, 
    void (*on_error)(struct error_info * info, void * context, const char * fmt, va_list args),
    void * context);
void cpe_error_monitor_add_node(error_monitor_t monitor, struct error_monitor_node * node);
void cpe_error_monitor_remove_node(error_monitor_t monitor, struct error_monitor_node * node);

#ifdef _MSC_VER
#define _CPE_DO_ERROR_NOTIFY(monitor, level, en, format, ...)   \
    if (monitor) {                                                  \
    (monitor)->m_curent_location.m_errno = en;                  \
    (monitor)->m_curent_location.m_level = level;               \
    cpe_error_do_notify((monitor), format, __VA_ARGS__);             \
    }
#else
#define _CPE_DO_ERROR_NOTIFY(monitor, level, en, format, args...)   \
    if (monitor) {                                                  \
        (monitor)->m_curent_location.m_errno = en;                  \
        (monitor)->m_curent_location.m_level = level;               \
        cpe_error_do_notify((monitor), format, ##args);             \
    }
#endif
#define CPE_DEF_ERROR_MONITOR_NODE_INITIALIZER(fun, context) \
    { (fun), (context), NULL }

#define CPE_DEF_ERROR_MONITOR_INITIALIZER(fun, context)     \
    { CPE_DEF_ERROR_MONITOR_NODE_INITIALIZER(fun, context)  \
            , { NULL, -1, 0, CPE_EL_ERROR } }

#define CPE_DEF_ERROR_MONITOR(name, fun, context) \
    struct error_monitor name = CPE_DEF_ERROR_MONITOR_INITIALIZER(fun, context)

#define CPE_DEF_ERROR_MONITOR_ADD(name, monitor, fun, context)          \
    struct error_monitor_node name = { fun, context, NULL };            \
    cpe_error_monitor_add_node(monitor, &name);

#define CPE_DEF_ERROR_MONITOR_REMOVE(name, monitor)                     \
    cpe_error_monitor_remove_node(monitor, &name);

#define CPE_ERROR_SET_LINE(monitor, line) if (monitor) { monitor->m_curent_location.m_line = (int)line; }
#define CPE_ERROR_SET_FILE(monitor, file) if (monitor) { monitor->m_curent_location.m_file = file; }
#define CPE_ERROR_SET_ERRNO(monitor, e) if (monitor) { monitor->m_curent_location.m_errno = (int)e; }
#define CPE_ERROR_SET_LEVEL(monitor, l) if (monitor) { monitor->m_curent_location.m_level = l; }

#ifdef _MSC_VER //for msvc
#define CPE_INFO(monitor, format, ...)                                  \
    _CPE_DO_ERROR_NOTIFY(monitor, CPE_EL_INFO, 0, __FILE__ "(%d): " format, __LINE__, __VA_ARGS__)

#define CPE_WARNING(monitor, format, ...)                               \
    _CPE_DO_ERROR_NOTIFY(monitor, CPE_EL_WARNING, -1, __FILE__ "(%d): " format, __LINE__, __VA_ARGS__)

#define CPE_ERROR(monitor, format, ...)                                 \
    _CPE_DO_ERROR_NOTIFY(monitor, CPE_EL_ERROR, -1, __FILE__ "(%d): " format, __LINE__, __VA_ARGS__)

#define CPE_WARNING_EX(monitor, en, format, ...)                        \
    _CPE_DO_ERROR_NOTIFY(monitor, CPE_EL_WARNING, en, __FILE__ "(%d): " format, __LINE__, __VA_ARGS__)

#define CPE_ERROR_EX(monitor, en, format, ...)                          \
    _CPE_DO_ERROR_NOTIFY(monitor, CPE_EL_ERROR, en, __FILE__ "(%d): " format, __LINE__, __VA_ARGS__)
#else //for other(gcc)
#define CPE_INFO(monitor, format, args...)                          \
    _CPE_DO_ERROR_NOTIFY(monitor, CPE_EL_INFO, 0, format, ##args)

#define CPE_WARNING(monitor, format, args...)                           \
    _CPE_DO_ERROR_NOTIFY(monitor, CPE_EL_WARNING, -1, format, ##args)

#define CPE_ERROR(monitor, format, args...)                         \
    _CPE_DO_ERROR_NOTIFY(monitor, CPE_EL_ERROR, -1, format, ##args)

#define CPE_WARNING_EX(monitor, en, format, args...)                    \
    _CPE_DO_ERROR_NOTIFY(monitor, CPE_EL_WARNING, en, format, ##args)

#define CPE_ERROR_EX(monitor, en, format, args...)                  \
    _CPE_DO_ERROR_NOTIFY(monitor, CPE_EL_ERROR, en, format, ##args)
#endif

#ifdef __cplusplus
}
#endif

#endif
