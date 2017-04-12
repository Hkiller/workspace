#include "plugin_spine_obj_anim_group_binding_i.h"

plugin_spine_obj_anim_group_binding_t
plugin_spine_obj_anim_group_binding_create(plugin_spine_obj_anim_group_t group, plugin_spine_obj_anim_t anim) {
    plugin_spine_obj_anim_group_binding_t binding;

    binding = TAILQ_FIRST(&group->m_module->m_free_anim_group_bindings);
    if (binding) {
        TAILQ_REMOVE(&group->m_module->m_free_anim_group_bindings, binding, m_next_for_group);
    }
    else {
        binding = mem_alloc(group->m_module->m_alloc, sizeof(struct plugin_spine_obj_anim_group_binding));
        if (binding == NULL) {
            CPE_ERROR(group->m_module->m_em, "plugin_spine_obj_anim_group_binding_create: alloc fail");
            return NULL;
        }
    }

    binding->m_group = group;
    binding->m_anim = anim;
    TAILQ_INSERT_TAIL(&group->m_anims, binding, m_next_for_group);
    TAILQ_INSERT_TAIL(&anim->m_groups, binding, m_next_for_anim);

    return binding;
}

void plugin_spine_obj_anim_group_binding_free(plugin_spine_obj_anim_group_binding_t binding) {
    plugin_spine_module_t module = binding->m_group->m_module;
    
    TAILQ_REMOVE(&binding->m_group->m_anims, binding, m_next_for_group);
    TAILQ_REMOVE(&binding->m_anim->m_groups, binding, m_next_for_anim);

    binding->m_group = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_anim_group_bindings, binding, m_next_for_group);
}

void plugin_spine_obj_anim_group_binding_real_free_all(plugin_spine_module_t module) {
    plugin_spine_obj_anim_group_binding_t binding;
    
    while(!TAILQ_EMPTY(&module->m_free_anim_group_bindings)) {
        binding = TAILQ_FIRST(&module->m_free_anim_group_bindings);

        TAILQ_REMOVE(&module->m_free_anim_group_bindings, binding, m_next_for_group);
        mem_free(module->m_alloc, binding);
    }
}
    


