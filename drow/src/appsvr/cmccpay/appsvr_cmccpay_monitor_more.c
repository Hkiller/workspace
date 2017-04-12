#include <assert.h>
#include "plugin/app_env/plugin_app_env_monitor.h"
#include "protocol/plugin/app_env/app_env_pro.h"
#include "appsvr_cmccpay_module_i.h"

static int appsvr_cmccpay_do_more_games(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size) {
    appsvr_cmccpay_module_t module = (appsvr_cmccpay_module_t)ctx;
    return appsvr_cmccpay_show_more_game_page(module);
}

int appsvr_cmccpay_monitor_more_init(appsvr_cmccpay_module_t module) {
    if (module->m_more_game_evt_name) {
        module->m_more_games_monitor = 
            plugin_app_env_monitor_create(
            module->m_app_env,
            module->m_more_game_evt_name,
            module, appsvr_cmccpay_do_more_games, NULL);
        if (module->m_more_games_monitor == NULL) {
            CPE_ERROR(module->m_em, "appsvr_cmccpay_plugin_init: create more_games fail!");
            return -1;
        }
    }

    return 0;
}

void appsvr_cmccpay_monitor_more_fini(appsvr_cmccpay_module_t module) {
    if (module->m_more_games_monitor) {
        plugin_app_env_monitor_free(module->m_more_games_monitor);
        module->m_more_games_monitor = NULL;
    }
}
