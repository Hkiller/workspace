#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "gd/app/app_context.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_state.h"
#include "plugin/ui/plugin_ui_phase.h"
#include "plugin/ui/plugin_ui_state_node.h"
#include "plugin/ui/plugin_ui_phase_node.h"
#include "appsvr_umeng_module_i.h"

static void appsvr_umeng_page_notify_state(appsvr_umeng_module_t module, plugin_ui_state_t state, uint8_t is_open) {
    char page_name[64];
    snprintf(
        page_name, sizeof(page_name), "%s.%s",
        plugin_ui_phase_name(plugin_ui_state_phase(state)),
        plugin_ui_state_name(state));
    
    if (is_open) {
        appsvr_umeng_on_page_begin(module, page_name);
    }
    else {
        appsvr_umeng_on_page_end(module, page_name);
    }
}
    
static ptr_int_t appsvr_umeng_page_monitor_tick(void * ctx, ptr_int_t arg, float delta_s) {
    appsvr_umeng_module_t module = ctx;
    plugin_ui_state_t cur_state = NULL;
    plugin_ui_env_t cur_env;

    assert(module->m_ui_module);

    cur_env = plugin_ui_env_first(module->m_ui_module);
    if (cur_env) {
        plugin_ui_phase_node_t cur_phase_node = plugin_ui_phase_node_current(cur_env);
        if (cur_phase_node) {
            plugin_ui_state_node_t cur_state_node = plugin_ui_state_node_current(cur_phase_node);
            if (cur_state_node) {
                cur_state = plugin_ui_state_node_process_state(cur_state_node);
            }
        }
    }

    if (cur_state == module->m_cur_state) return 0;

    if (module->m_cur_state) {
        appsvr_umeng_page_notify_state(module, module->m_cur_state, 0);
    }

    if (cur_state) {
        appsvr_umeng_page_notify_state(module, cur_state, 1);
    }

    module->m_cur_state = cur_state;

    return 0;
}

int appsvr_umeng_page_monitor_init(appsvr_umeng_module_t module) {
    if (module->m_ui_module) {
        if (gd_app_tick_add(module->m_app, appsvr_umeng_page_monitor_tick, module, 0) != 0) {
            CPE_ERROR(module->m_em, "appsvr_umeng_page_monitor_init: add tick fail!");
            return -1;
        }
    }
    
    return 0;
}

void appsvr_umeng_page_monitor_fini(appsvr_umeng_module_t module) {
    if (module->m_ui_module) {

        if (module->m_cur_state) {
            appsvr_umeng_page_notify_state(module, module->m_cur_state, 0);
            module->m_cur_state = NULL;
        }
        
        gd_app_tick_remove(module->m_app, appsvr_umeng_page_monitor_tick, module);
    }
}
