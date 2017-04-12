#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "plugin_moving_env_i.h"
#include "plugin_moving_node_i.h"
#include "plugin_moving_control_i.h"

plugin_moving_env_t plugin_moving_env_create(plugin_moving_module_t module) {
    plugin_moving_env_t env;

    env = mem_alloc(module->m_alloc, sizeof(struct plugin_moving_env));
    if (env == NULL) {
        CPE_ERROR(module->m_em, "plugin_moving_env_create: create fail!");
        return NULL;
    }

    env->m_module = module;

    TAILQ_INIT(&env->m_controls);
    TAILQ_INIT(&env->m_free_controls);
    TAILQ_INIT(&env->m_free_nodes);

    return env;
}

void plugin_moving_env_free(plugin_moving_env_t env) {
    plugin_moving_module_t module = env->m_module;
    
    while(!TAILQ_EMPTY(&env->m_controls)) {
        plugin_moving_control_free(TAILQ_FIRST(&env->m_controls));
    }

    while(!TAILQ_EMPTY(&env->m_free_controls)) {
        plugin_moving_control_real_free(TAILQ_FIRST(&env->m_free_controls));
    }
    
    while(!TAILQ_EMPTY(&env->m_free_nodes)) {
        plugin_moving_node_real_free(TAILQ_FIRST(&env->m_free_nodes));
    }

    mem_free(module->m_alloc, env);
}

ui_data_mgr_t plugin_moving_env_data_mgr(plugin_moving_env_t env) {
    return env->m_module->m_data_mgr;
}
