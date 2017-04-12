#include "appsvr_notify_activate_i.h"

appsvr_notify_activate_t
appsvr_notify_activate_create(appsvr_notify_schedule_t schedule, appsvr_notify_adapter_t adapter) {
    appsvr_notify_module_t module = schedule->m_module;
    appsvr_notify_activate_t activate;

    activate = TAILQ_FIRST(&module->m_free_activates);
    if (activate) {
        TAILQ_REMOVE(&module->m_free_activates, activate, m_next_for_schedule);
    }
    else {
        activate = mem_alloc(module->m_alloc, sizeof(struct appsvr_notify_activate));
        if (activate == NULL) {
            CPE_ERROR(module->m_em, "appsvr_notify_activate_create: alloc fail!");
            return NULL;
        }
    }

    if (adapter->m_install_fun(adapter->m_ctx, schedule) != 0) {
        CPE_ERROR(module->m_em, "appsvr_notify_activate_create: install to adapter %s fail!", adapter->m_name);
        activate->m_schedule = (void*)module;
        TAILQ_INSERT_TAIL(&module->m_free_activates, activate, m_next_for_schedule);
        return NULL;
    }

    activate->m_schedule = schedule;
    activate->m_adapter = adapter;

    TAILQ_INSERT_TAIL(&schedule->m_activates, activate, m_next_for_schedule);
    TAILQ_INSERT_TAIL(&adapter->m_activates, activate, m_next_for_adapter);

    return activate;
}

void appsvr_notify_activate_free(appsvr_notify_activate_t activate) {
    appsvr_notify_module_t module = activate->m_schedule->m_module;

    activate->m_adapter->m_uninstall_fun(activate->m_adapter->m_ctx, activate->m_schedule);
    
    TAILQ_REMOVE(&activate->m_schedule->m_activates, activate, m_next_for_schedule);
    TAILQ_REMOVE(&activate->m_adapter->m_activates, activate, m_next_for_adapter);

    activate->m_schedule = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_activates, activate, m_next_for_schedule);
}

void appsvr_notify_activate_real_free(appsvr_notify_activate_t activate) {
    appsvr_notify_module_t module = (appsvr_notify_module_t)activate->m_schedule;

    TAILQ_REMOVE(&module->m_free_activates, activate, m_next_for_schedule);
    mem_free(module->m_alloc, activate);
}

