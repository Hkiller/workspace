#ifndef APPSVR_STATISTICS_TELECOMPAY_MODULE_H
#define APPSVR_STATISTICS_TELECOMPAY_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_types.h"
#include "plugin/app_env/plugin_app_env_types.h"
#include "appsvr/account/appsvr_account_types.h"
#include "appsvr/payment/appsvr_payment_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct appsvr_telecompay_module * appsvr_telecompay_module_t;
typedef struct appsvr_telecompay_backend * appsvr_telecompay_backend_t;

struct appsvr_telecompay_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    plugin_app_env_module_t m_app_env;
    uint8_t m_debug;
    plugin_app_env_monitor_t m_more_games_monitor;
    plugin_app_env_monitor_t m_stop_monitor;
    appsvr_payment_module_t m_payment_module;

    appsvr_telecompay_backend_t m_backend;
    
    char * m_more_game_evt_name;

    struct mem_buffer m_dump_buffer;
};

int appsvr_telecompay_backend_init(appsvr_telecompay_module_t module);
void appsvr_telecompay_backend_fini(appsvr_telecompay_module_t module);

int appsvr_telecompay_monitor_stop_init(appsvr_telecompay_module_t module);
void appsvr_telecompay_monitor_stop_fini(appsvr_telecompay_module_t module);    

int appsvr_telecompay_monitor_more_init(appsvr_telecompay_module_t module);
void appsvr_telecompay_monitor_more_fini(appsvr_telecompay_module_t module);  

int appsvr_telecompay_show_stop_page(appsvr_telecompay_module_t module);
int appsvr_telecompay_show_more_game_page(appsvr_telecompay_module_t module);

#ifdef __cplusplus
}
#endif

#endif
