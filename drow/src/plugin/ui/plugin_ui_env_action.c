#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin_ui_env_action_i.h"
#include "plugin_ui_env_i.h"
#include "plugin_ui_aspect_i.h"
#include "plugin_ui_aspect_ref_i.h"

plugin_ui_env_action_t
plugin_ui_env_action_create(
    plugin_ui_env_t env, 
    plugin_ui_event_t evt,
    plugin_ui_event_fun_t fun, void * ctx)
{
    plugin_ui_env_action_t action;

    assert(env);
    assert(env->m_module);

    action = TAILQ_FIRST(&env->m_free_env_actions);
    if (action) {
        TAILQ_REMOVE(&env->m_free_env_actions, action, m_next_for_env);
    }
    else {
        action = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_env_action));
        if (action == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_action_create: alloc action fail!");
            return NULL;
        }
    }

    assert(action);

    action->m_env = env;
    TAILQ_INIT(&action->m_aspects);
    action->m_name_prefix = NULL;
    action->m_event = evt;
    action->m_fun = fun;
    action->m_ctx = ctx;
    action->m_is_processing = 0;
    action->m_is_free = 0;
    
    TAILQ_INSERT_TAIL(&env->m_env_actions, action, m_next_for_env);    

    return action;
}

void plugin_ui_env_action_free(plugin_ui_env_action_t action) {
    plugin_ui_env_t env = action->m_env;

    if (action->m_name_prefix) {
        mem_free(env->m_module->m_alloc, action->m_name_prefix);
        action->m_name_prefix = NULL;
    }
    
    while(!TAILQ_EMPTY(&action->m_aspects)) {
        plugin_ui_aspect_ref_t ref = TAILQ_FIRST(&action->m_aspects);
        plugin_ui_aspect_ref_free(ref, &ref->m_aspect->m_env_actions, &action->m_aspects);
    }

    if (action->m_is_processing) {
        action->m_is_free = 1;
        return;
    }
    
    TAILQ_REMOVE(&env->m_env_actions, action, m_next_for_env);    
    TAILQ_INSERT_TAIL(&env->m_free_env_actions, action, m_next_for_env);
}
    
void plugin_ui_env_action_real_free(plugin_ui_env_action_t action) {
    plugin_ui_env_t env = action->m_env;

    TAILQ_REMOVE(&env->m_free_env_actions, action, m_next_for_env);

    mem_free(env->m_module->m_alloc,  action);
}

uint8_t plugin_ui_env_action_data_capacity(plugin_ui_env_action_t action) {
    return (uint8_t)CPE_ARRAY_SIZE(action->m_data);
}

void * plugin_ui_env_action_data(plugin_ui_env_action_t action) {
    return action->m_data;
}

const char * plugin_ui_env_action_name_prefix(plugin_ui_env_action_t action) {
    return action->m_name_prefix;
}

int plugin_ui_env_action_set_name_prefix(plugin_ui_env_action_t action, const char * name_prefix) {
    plugin_ui_env_t env = action->m_env;

    if (action->m_name_prefix) {
        mem_free(env->m_module->m_alloc, action->m_name_prefix);
    }

    if (name_prefix) {
        action->m_name_prefix = cpe_str_mem_dup(env->m_module->m_alloc, name_prefix);
        if (action->m_name_prefix == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_action_set_name_prefix: alloc name prefix fail!");
            return -1;
        }
    }
    else {
        name_prefix = NULL;
    }

    return 0;
}
