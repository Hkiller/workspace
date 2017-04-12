#include "cpe/utils/string_utils.h"
#include "appsvr_ad_ui_state_monitor_i.h"

appsvr_ad_ui_state_monitor_t
appsvr_ad_ui_state_monitor_create(appsvr_ad_module_t module, const char * state_name, const char * action_name) {
    appsvr_ad_ui_state_monitor_t monitor;

    monitor = mem_alloc(module->m_alloc, sizeof(struct appsvr_ad_ui_state_monitor));
    if (monitor == NULL) {
        CPE_ERROR(module->m_em, "appsvr_ad_ui_state_monitor_create: alloc fail!");
        return NULL;
    }

    monitor->m_module = module;
    cpe_str_dup(monitor->m_state_name, sizeof(monitor->m_state_name), state_name);
    cpe_str_dup(monitor->m_action_name, sizeof(monitor->m_action_name), action_name);

    TAILQ_INSERT_TAIL(&module->m_ui_state_monitors, monitor, m_next);
    return monitor;
}

void appsvr_ad_ui_state_monitor_free(appsvr_ad_ui_state_monitor_t ui_state_monitor) {
    TAILQ_REMOVE(&ui_state_monitor->m_module->m_ui_state_monitors, ui_state_monitor, m_next);
    mem_free(ui_state_monitor->m_module->m_alloc, ui_state_monitor);
}

appsvr_ad_ui_state_monitor_t
appsvr_ad_ui_state_monitor_find_by_state_name(appsvr_ad_module_t module, const char * state_name) {
    appsvr_ad_ui_state_monitor_t monitor;

    TAILQ_FOREACH(monitor, &module->m_ui_state_monitors, m_next) {
        if (strcmp(monitor->m_state_name, state_name) == 0) return monitor;
    }

    return NULL;
}
