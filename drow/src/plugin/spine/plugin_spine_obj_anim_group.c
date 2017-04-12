#include "plugin_spine_obj_anim_group_i.h"
#include "plugin_spine_obj_anim_group_binding_i.h"

plugin_spine_obj_anim_group_t
plugin_spine_obj_anim_group_create(plugin_spine_module_t module) {
    plugin_spine_obj_anim_group_t gruop;

    gruop = mem_alloc(module->m_alloc, sizeof(struct plugin_spine_obj_anim_group));
    if (gruop == NULL) {
        CPE_ERROR(module->m_em, "plugin_spine_obj_anim_group_create: alloc fail!");
        return NULL;
    }

    gruop->m_module = module;
    TAILQ_INIT(&gruop->m_anims);

    return gruop;
}

void plugin_spine_obj_anim_group_free(plugin_spine_obj_anim_group_t anim_group) {
    plugin_spine_module_t module = anim_group->m_module;
    
    while(!TAILQ_EMPTY(&anim_group->m_anims)) {
        plugin_spine_obj_anim_group_binding_free(TAILQ_FIRST(&anim_group->m_anims));
    }

    mem_free(module->m_alloc, anim_group);
}

int plugin_spine_obj_anim_group_add_anim(plugin_spine_obj_anim_group_t anim_group, plugin_spine_obj_anim_t anim) {
    plugin_spine_module_t module = anim_group->m_module;
    plugin_spine_obj_anim_group_binding_t binding;
    
    TAILQ_FOREACH(binding, &anim->m_groups, m_next_for_anim) {
        if (binding->m_group == anim_group) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_anim_group_add_anim: anim is alreay in group!");
            return -1;
        }
    }

    binding = plugin_spine_obj_anim_group_binding_create(anim_group, anim);
    return binding ? 0 : -1;
}

int plugin_spine_obj_anim_group_remove_anim(plugin_spine_obj_anim_group_t anim_group, plugin_spine_obj_anim_t anim) {
    plugin_spine_module_t module = anim_group->m_module;
    plugin_spine_obj_anim_group_binding_t binding, next_binding;
    uint8_t removed_count = 0;
    
    for(binding = TAILQ_FIRST(&anim->m_groups); binding; binding = next_binding) {
        next_binding = TAILQ_NEXT(binding, m_next_for_anim);
        
        if (binding->m_group == anim_group) {
            removed_count++;
            plugin_spine_obj_anim_group_binding_free(binding);
        }
    }
    
    if (removed_count == 0) {
        CPE_ERROR(module->m_em, "plugin_spine_obj_anim_group_remove_anim: anim is not in group!");
        return -1;
    }

    return 0;
}

int plugin_spine_obj_anim_group_add_track_listener(plugin_spine_obj_anim_group_t anim_group, plugin_spine_anim_event_fun_t fun, void * ctx) {
    plugin_spine_module_t module = anim_group->m_module;
    plugin_spine_obj_anim_group_binding_t binding;

    TAILQ_FOREACH(binding, &anim_group->m_anims, m_next_for_group) {
        if (plugin_spine_obj_anim_add_track_listener(binding->m_anim, fun, ctx) != 0) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_anim_group_add_track_listener: add fail!");
            plugin_spine_obj_anim_group_remove_track_listener(anim_group, ctx);
            return -1;
        }
    }

    return 0;
}

void plugin_spine_obj_anim_group_remove_track_listener(plugin_spine_obj_anim_group_t anim_group, void * ctx) {
    plugin_spine_obj_anim_group_binding_t binding;

    TAILQ_FOREACH(binding, &anim_group->m_anims, m_next_for_group) {
        plugin_spine_obj_anim_remove_track_listener(binding->m_anim, ctx);
    }
}    

void plugin_spine_obj_anim_group_free_all_anims(plugin_spine_obj_anim_group_t anim_group) {
    while(!TAILQ_EMPTY(&anim_group->m_anims)) {
        plugin_spine_obj_anim_free(TAILQ_FIRST(&anim_group->m_anims)->m_anim);
    }
}

uint8_t plugin_spine_obj_anim_group_is_playing(plugin_spine_obj_anim_group_t anim_group) {
    plugin_spine_obj_anim_group_binding_t binding;

    TAILQ_FOREACH(binding, &anim_group->m_anims, m_next_for_group) {
        if (plugin_spine_obj_anim_is_playing(binding->m_anim)) return 1;
    }

    return 0;
}
