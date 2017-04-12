#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin_ui_popup_action_i.h"

plugin_ui_popup_action_t
plugin_ui_popup_action_create(plugin_ui_popup_t popup, const char * action_naem, plugin_ui_popup_action_fun_t fun, void * ctx) {
    plugin_ui_popup_action_t action;
    plugin_ui_env_t env = popup->m_env;
    
    action = TAILQ_FIRST(&env->m_free_popup_actions);
    if (action) {
        TAILQ_REMOVE(&env->m_free_popup_actions, action, m_next);
    }
    else {
        action = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_popup_action));
        if (action == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_popup_action_create: alloc action fail!");
            return NULL;
        }
    }

    assert(action);

    cpe_str_dup(action->m_name, sizeof(action->m_name), action_naem);
    action->m_popup = popup;
    action->m_fun = fun;
    action->m_ctx = ctx;

    TAILQ_INSERT_TAIL(&popup->m_actions, action, m_next);

    return action;
}

void plugin_ui_popup_action_free(plugin_ui_popup_action_t action) {
    plugin_ui_popup_t popup = action->m_popup;
    plugin_ui_env_t env = popup->m_env;

    TAILQ_REMOVE(&popup->m_actions, action, m_next);

    action->m_popup = (void*)env;
    TAILQ_INSERT_TAIL(&env->m_free_popup_actions, action, m_next);
}
    
void plugin_ui_popup_action_real_free(plugin_ui_popup_action_t action) {
    plugin_ui_env_t env = (void*)action->m_popup;

    TAILQ_REMOVE(&env->m_free_popup_actions, action, m_next);

    mem_free(env->m_module->m_alloc,  action);
}

uint8_t plugin_ui_popup_action_data_capacity(plugin_ui_popup_action_t action) {
    return (uint8_t)CPE_ARRAY_SIZE(action->m_data);
}

void * plugin_ui_popup_action_data(plugin_ui_popup_action_t action) {
    return action->m_data;
}
