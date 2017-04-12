#ifndef APPSVR_STATISTICS_UMENG_EXECUTOR_H
#define APPSVR_STATISTICS_UMENG_EXECUTOR_H
#include "appsvr_umeng_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_umeng_executor_arg {
    char m_type;
    char m_def[64];
};
    
struct appsvr_umeng_executor {
    appsvr_umeng_module_t m_module;
    TAILQ_ENTRY(appsvr_umeng_executor) m_next;
    
    const char * m_op_name;
    const char * m_condition;
    uint8_t m_arg_count;
    struct appsvr_umeng_executor_arg m_args[5];
    plugin_app_env_executor_t m_executor;
    char m_backend_data[64];
};
    
void appsvr_umeng_executor_free(appsvr_umeng_executor_t executor);
int appsvr_umeng_load_executors(appsvr_umeng_module_t module, cfg_t cfg);

int appsvr_umeng_executor_backend_init(appsvr_umeng_executor_t executor);
void appsvr_umeng_executor_backend_exec(appsvr_umeng_executor_t executor, dr_data_source_t data_source);    
void appsvr_umeng_executor_backend_fini(appsvr_umeng_executor_t executor);
    
#ifdef __cplusplus
}
#endif
    
#endif
