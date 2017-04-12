#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin/package/plugin_package_queue.h"
#include "plugin_ui_package_queue_managed_i.h"
#include "plugin_ui_package_queue_using_i.h"

plugin_ui_package_queue_managed_t
plugin_ui_package_queue_managed_create(
    plugin_ui_env_t env, const char * name, plugin_package_queue_policy_t policy)
{
    plugin_ui_package_queue_managed_t managed_queue;
    plugin_package_queue_t package_queue;
    
    assert(env);
    assert(env->m_module);

    package_queue = plugin_package_queue_create(env->m_module->m_package_module, name, policy);
    if (package_queue == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_package_queue_managed_create: create package queue %s fail!", name);
        return NULL;
    }
    plugin_package_queue_set_limit(package_queue, 0);
    
    managed_queue = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_package_queue_managed));
    if (managed_queue == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_package_queue_managed_create: alloc action fail!");
        plugin_package_queue_free(package_queue);
        return NULL;
    }

    managed_queue->m_env = env;
    managed_queue->m_package_queue = package_queue;
    TAILQ_INIT(&managed_queue->m_phases);
    
    TAILQ_INSERT_TAIL(&env->m_package_queue_manageds, managed_queue, m_next_for_env);    

    return managed_queue;
}

void plugin_ui_package_queue_managed_free(plugin_ui_package_queue_managed_t managed_queue) {
    plugin_ui_env_t env = managed_queue->m_env;

    while(!TAILQ_EMPTY(&managed_queue->m_phases)) {
        plugin_ui_package_queue_using_free(TAILQ_FIRST(&managed_queue->m_phases));
    }
    
    plugin_package_queue_free(managed_queue->m_package_queue);
    
    TAILQ_REMOVE(&env->m_package_queue_manageds, managed_queue, m_next_for_env);

    mem_free(env->m_module->m_alloc, managed_queue);
}

plugin_ui_package_queue_managed_t
plugin_ui_package_queue_managed_find(plugin_ui_env_t env, const char * name) {
    plugin_ui_package_queue_managed_t managed_queue;

    TAILQ_FOREACH(managed_queue, &env->m_package_queue_manageds, m_next_for_env) {
        if (strcmp(plugin_package_queue_name(managed_queue->m_package_queue), name) == 0) return managed_queue;
    }

    return NULL;
}
