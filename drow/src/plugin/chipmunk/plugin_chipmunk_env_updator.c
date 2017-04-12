#include "plugin_chipmunk_env_updator_i.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_chipmunk_env_updator_t plugin_chipmunk_env_updator_create(
    plugin_chipmunk_env_t env, plugin_chipmunk_env_update_fun_t fun, void * ctx)
{
    plugin_chipmunk_env_updator_t updator;

    updator = TAILQ_FIRST(&env->m_module->m_free_updators);
    if (updator) {
        TAILQ_REMOVE(&env->m_module->m_free_updators, updator, m_next);
    }
    else {
        updator = (plugin_chipmunk_env_updator_t)mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_chipmunk_env_updator));
    }

    updator->m_env = env;
    updator->m_fun = fun;
    updator->m_ctx = ctx;

    TAILQ_INSERT_TAIL(&env->m_updators, updator, m_next);

    return updator;
}

void plugin_chipmunk_env_updator_free(plugin_chipmunk_env_updator_t updator) {
    plugin_chipmunk_env_t env = updator->m_env;
    TAILQ_REMOVE(&env->m_updators, updator, m_next);

    updator->m_env = (plugin_chipmunk_env_t)(void*)env->m_module;
    TAILQ_INSERT_TAIL(&env->m_module->m_free_updators, updator, m_next);
}

void plugin_chipmunk_env_updator_real_free(plugin_chipmunk_env_updator_t updator) {
    plugin_chipmunk_module_t module = (plugin_chipmunk_module_t)updator->m_env;
    TAILQ_REMOVE(&module->m_free_updators, updator, m_next);
    mem_free(module->m_alloc, updator);
}

#ifdef __cplusplus
}
#endif
