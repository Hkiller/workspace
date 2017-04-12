#ifndef APPSVR_STATISTICS_UMENG_MODULE_H
#define APPSVR_STATISTICS_UMENG_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/hash_string.h"
#include "cpe/utils/buffer.h"
#include "cpe/xcalc/xcalc_computer.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_types.h"
#include "plugin/ui/plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_umeng_module * appsvr_umeng_module_t;
typedef struct appsvr_umeng_backend * appsvr_umeng_backend_t;
typedef struct appsvr_umeng_executor_arg * appsvr_umeng_executor_arg_t;
typedef struct appsvr_umeng_executor * appsvr_umeng_executor_t;
typedef struct appsvr_umeng_click_info * appsvr_umeng_click_info_t;
typedef struct appsvr_umeng_pay_chanel * appsvr_umeng_pay_chanel_t;    

typedef TAILQ_HEAD(appsvr_umeng_executor_list, appsvr_umeng_executor) appsvr_umeng_executor_list_t;
typedef TAILQ_HEAD(appsvr_umeng_pay_chanel_list, appsvr_umeng_pay_chanel) appsvr_umeng_pay_chanel_list_t;

struct appsvr_umeng_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    plugin_app_env_module_t m_app_env;
    uint8_t m_debug;
    char * m_app_key;
    char * m_chanel;
    
    plugin_ui_module_t m_ui_module;
    plugin_ui_state_t m_cur_state;
    plugin_ui_env_action_t m_click_monitor;

    plugin_app_env_monitor_t m_suspend_monitor;

    struct cpe_hash_table m_click_infos;
    
    appsvr_umeng_executor_list_t m_executors;

    appsvr_umeng_pay_chanel_list_t m_pay_chanels;
    
    appsvr_umeng_backend_t m_backend;
    xcomputer_t m_computer;
    struct mem_buffer m_dump_buffer;
};

int appsvr_umeng_backend_init(appsvr_umeng_module_t module);
void appsvr_umeng_backend_fini(appsvr_umeng_module_t module);

int appsvr_umeng_suspend_monitor_init(appsvr_umeng_module_t module);
void appsvr_umeng_suspend_monitor_fini(appsvr_umeng_module_t module);
    
int appsvr_umeng_page_monitor_init(appsvr_umeng_module_t module);
void appsvr_umeng_page_monitor_fini(appsvr_umeng_module_t module);

int appsvr_umeng_click_monitor_init(appsvr_umeng_module_t module);
void appsvr_umeng_click_monitor_fini(appsvr_umeng_module_t module);
    
/*umeng ops*/    
void appsvr_umeng_on_page_begin(appsvr_umeng_module_t, const char * page_name);
void appsvr_umeng_on_page_end(appsvr_umeng_module_t, const char * page_name);
void appsvr_umeng_on_pause(appsvr_umeng_module_t module);
void appsvr_umeng_on_resume(appsvr_umeng_module_t module);
void appsvr_umeng_on_event(appsvr_umeng_module_t module, const char * id, uint32_t count, const char * attrs);

#ifdef __cplusplus
}
#endif

#endif
