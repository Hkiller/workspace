#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin_ui_package_queue_using_i.h"

plugin_ui_package_queue_using_t
plugin_ui_package_queue_using_create(plugin_ui_package_queue_managed_t queue, plugin_ui_phase_t phase) {
    plugin_ui_env_t env = queue->m_env;
    plugin_ui_package_queue_using_t using;
    
    assert(env);
    assert(env->m_module);

    using = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_package_queue_using));
    if (using == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_package_queue_using_create: alloc action fail!");
        return NULL;
    }

    using->m_queue = queue;
    using->m_phase = phase;
    using->m_limit = 0;
    
    TAILQ_INSERT_TAIL(&queue->m_phases, using, m_next_for_queue);    
    TAILQ_INSERT_TAIL(&phase->m_using_package_queues, using, m_next_for_phase);    

    return using;
}

void plugin_ui_package_queue_using_free(plugin_ui_package_queue_using_t using) {
    plugin_ui_env_t env = using->m_queue->m_env;
    
    TAILQ_REMOVE(&using->m_queue->m_phases, using, m_next_for_queue);    
    TAILQ_REMOVE(&using->m_phase->m_using_package_queues, using, m_next_for_phase);    

    mem_free(env->m_module->m_alloc, using);
}

plugin_ui_package_queue_using_t
plugin_ui_package_queue_using_find(plugin_ui_package_queue_managed_t queue, plugin_ui_phase_t phase) {
    plugin_ui_package_queue_using_t using;

    TAILQ_FOREACH(using, &phase->m_using_package_queues, m_next_for_phase) {
        if (using->m_queue == queue) return using;
    }
    
    return NULL;
}

uint32_t plugin_ui_package_queue_using_limit(plugin_ui_package_queue_using_t using) {
    return using->m_limit;
}

void plugin_ui_package_queue_using_set_limit(plugin_ui_package_queue_using_t using, uint32_t limit) {
    using->m_limit = limit;
}
