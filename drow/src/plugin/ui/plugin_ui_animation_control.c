#include <assert.h>
#include "plugin_ui_animation_control_i.h"
#include "plugin_ui_animation_i.h"
#include "plugin_ui_animation_meta_i.h"
#include "plugin_ui_control_i.h"

plugin_ui_animation_control_t
plugin_ui_animation_control_create(plugin_ui_animation_t animation, plugin_ui_control_t control, uint8_t is_tie) {
    plugin_ui_env_t env = animation->m_env;
    plugin_ui_module_t module = env->m_module;
    plugin_ui_animation_meta_t meta = animation->m_meta;
    plugin_ui_animation_control_t animation_control;

    TAILQ_FOREACH(animation_control, &control->m_animations, m_next_for_control) {
        if (animation_control->m_animation == animation) {
            if (is_tie) animation_control->m_is_tie = is_tie;
            return animation_control;
        }
    }
    
    animation_control = TAILQ_FIRST(&env->m_free_animation_controls);
    if (animation_control) {
        TAILQ_REMOVE(&env->m_free_animation_controls, animation_control, m_next_for_animation);
    }
    else {
        animation_control = mem_alloc(module->m_alloc, sizeof(struct plugin_ui_animation_control) + module->m_animation_control_max_capacity);
        if (animation_control == NULL) {
            CPE_ERROR(module->m_em, "plugin_ui_animation_control: create: alloc fail!");
            return NULL;
        }
    }

    animation_control->m_animation = animation;
    animation_control->m_control = control;
    animation_control->m_is_tie = is_tie;

    if (meta->m_control_attach && meta->m_control_attach(animation_control, meta->m_ctx) != 0) {
        CPE_ERROR(module->m_em, "plugin_ui_animation_control: type %s attach control fail!", meta->m_name);
        animation_control->m_animation = (void*)env;
        TAILQ_INSERT_TAIL(&env->m_free_animation_controls, animation_control, m_next_for_animation);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&animation_control->m_animation->m_controls, animation_control, m_next_for_animation);
    TAILQ_INSERT_TAIL(&animation_control->m_control->m_animations, animation_control, m_next_for_control);
    
    return animation_control;
}

void plugin_ui_animation_control_free(plugin_ui_animation_control_t animation_control) {
    plugin_ui_env_t env = animation_control->m_animation->m_env;
    plugin_ui_animation_meta_t meta = animation_control->m_animation->m_meta;
    
    if (meta->m_control_detach) {
        meta->m_control_detach(animation_control, meta->m_ctx);
    }

    TAILQ_REMOVE(&animation_control->m_animation->m_controls, animation_control, m_next_for_animation);
    TAILQ_REMOVE(&animation_control->m_control->m_animations, animation_control, m_next_for_control);

    animation_control->m_animation = (void*)env;
    TAILQ_INSERT_TAIL(&env->m_free_animation_controls, animation_control, m_next_for_animation);
}
    
void plugin_ui_animation_control_real_free(plugin_ui_animation_control_t animation_control) {
    plugin_ui_env_t env = (void*)animation_control->m_animation;

    TAILQ_REMOVE(&env->m_free_animation_controls, animation_control, m_next_for_animation);
    mem_free(env->m_module->m_alloc, animation_control);
}

void * plugin_ui_animation_control_data(plugin_ui_animation_control_t animation_control) {
    return animation_control + 1;
}

plugin_ui_animation_control_t plugin_ui_animation_control_from_data(void * data) {
    return ((plugin_ui_animation_control_t)data) - 1;
}

uint8_t plugin_ui_animation_control_is_tie(plugin_ui_animation_control_t animation_control) {
    return animation_control->m_is_tie;
}

plugin_ui_animation_control_t
plugin_ui_animation_control_find(plugin_ui_animation_t animation, plugin_ui_control_t control) {
    plugin_ui_animation_control_t anim_control;

    TAILQ_FOREACH(anim_control, &control->m_animations, m_next_for_control) {
        if (anim_control->m_animation == animation) return anim_control;
    }

    return NULL;
}
