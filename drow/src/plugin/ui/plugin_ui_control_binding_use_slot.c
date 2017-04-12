#include <assert.h>
#include "plugin_ui_control_binding_use_slot_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_env_i.h"

plugin_ui_control_binding_use_slot_t
plugin_ui_control_binding_use_slot_create(plugin_ui_control_binding_t binding, plugin_ui_page_slot_t slot) {
    plugin_ui_env_t env = slot->m_page->m_env;
    plugin_ui_control_binding_use_slot_t binding_use_slot;
    
    binding_use_slot = TAILQ_FIRST(&env->m_free_binding_use_slots);
    if (binding_use_slot) {
        TAILQ_REMOVE(&env->m_free_binding_use_slots, binding_use_slot, m_next_for_slot);
    }
    else {
        binding_use_slot = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_control_binding_use_slot));
        if (binding_use_slot == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_control_binding_use_slot_create: alloc fail!");
            return NULL;
        }
    }

    binding_use_slot->m_binding = binding;
    binding_use_slot->m_slot = slot;

    TAILQ_INSERT_TAIL(&binding->m_slots, binding_use_slot, m_next_for_binding);
    TAILQ_INSERT_TAIL(&slot->m_bindings, binding_use_slot, m_next_for_slot);

    return binding_use_slot;
}

void plugin_ui_control_binding_use_slot_free(plugin_ui_control_binding_use_slot_t binding_use_slot) {
    plugin_ui_env_t env = binding_use_slot->m_slot->m_page->m_env;

    TAILQ_REMOVE(&binding_use_slot->m_binding->m_slots, binding_use_slot, m_next_for_binding);
    TAILQ_REMOVE(&binding_use_slot->m_slot->m_bindings, binding_use_slot, m_next_for_slot);

    binding_use_slot->m_slot = (plugin_ui_page_slot_t)env;
    TAILQ_INSERT_TAIL(&env->m_free_binding_use_slots, binding_use_slot, m_next_for_slot);
}

void plugin_ui_control_binding_use_slot_real_free(plugin_ui_control_binding_use_slot_t binding_use_slot) {
    plugin_ui_env_t env = (plugin_ui_env_t)binding_use_slot->m_slot;

    TAILQ_REMOVE(&env->m_free_binding_use_slots, binding_use_slot, m_next_for_slot);

    mem_free(env->m_module->m_alloc, binding_use_slot);
}
