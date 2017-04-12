#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin_ui_aspect_i.h"
#include "plugin_ui_env_action_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_control_frame_i.h"
#include "plugin_ui_control_action_i.h"
#include "plugin_ui_control_action_slots_i.h"
#include "plugin_ui_animation_i.h"
#include "plugin_ui_aspect_ref_i.h"

plugin_ui_aspect_t
plugin_ui_aspect_create(plugin_ui_env_t env, const char * name) {
    plugin_ui_aspect_t aspect;
    
    aspect = TAILQ_FIRST(&env->m_free_aspects);
    if (aspect) {
        TAILQ_REMOVE(&env->m_free_aspects, aspect, m_next);
    }
    else {
        aspect = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_aspect));
        if (aspect == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_aspect: create: alloc fail!");
            return NULL;
        }
    }

    aspect->m_env = env;
    cpe_str_dup(aspect->m_name, sizeof(aspect->m_name), name ? name : "");
    TAILQ_INIT(&aspect->m_env_actions);
    TAILQ_INIT(&aspect->m_controls);
    TAILQ_INIT(&aspect->m_animations);
    TAILQ_INIT(&aspect->m_control_actions);
    TAILQ_INIT(&aspect->m_control_frames);

    TAILQ_INSERT_TAIL(&env->m_aspects, aspect, m_next);

    return aspect;
}

void plugin_ui_aspect_free(plugin_ui_aspect_t aspect) {
    plugin_ui_env_t env = aspect->m_env;

    plugin_ui_aspect_clear(aspect);

    TAILQ_REMOVE(&env->m_aspects, aspect, m_next);
    TAILQ_INSERT_TAIL(&env->m_free_aspects, aspect, m_next);
}

void plugin_ui_aspect_real_free(plugin_ui_aspect_t aspect) {
    plugin_ui_env_t env = aspect->m_env;
    TAILQ_REMOVE(&env->m_free_aspects, aspect, m_next);
    mem_free(env->m_module->m_alloc, aspect);
}

void plugin_ui_aspect_clear(plugin_ui_aspect_t aspect) {
    plugin_ui_aspect_env_action_clear(aspect);
    plugin_ui_aspect_control_clear(aspect);
    plugin_ui_aspect_control_frame_clear(aspect);
    plugin_ui_aspect_control_action_clear(aspect);    
    plugin_ui_aspect_animation_clear(aspect);
}

int plugin_ui_aspect_control_add(plugin_ui_aspect_t aspect, plugin_ui_control_t control, uint8_t is_tie) {
    plugin_ui_env_t env = aspect->m_env;
    plugin_ui_aspect_ref_t ref;

    TAILQ_FOREACH(ref, &control->m_aspects, m_next_for_obj) {
        if (ref->m_aspect == aspect) {
            if (is_tie) ref->m_is_tie = is_tie;
            return 0;
        }
    }
    
    if (plugin_ui_aspect_ref_create(aspect, &aspect->m_controls, control, &control->m_aspects, is_tie) == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "asspect %s: add control %s: create ref fail",
            aspect->m_name, plugin_ui_control_path_dump(&env->m_module->m_dump_buffer, control));
        return -1;
    }

    return 0;
}

void plugin_ui_aspect_control_remove(plugin_ui_aspect_t aspect, plugin_ui_control_t control) {
    plugin_ui_aspect_ref_t ref;

    TAILQ_FOREACH(ref, &control->m_aspects, m_next_for_obj) {
        if (ref->m_aspect == aspect) {
            plugin_ui_aspect_ref_free(ref, &aspect->m_controls, &control->m_aspects);
            return;
        }
    }
}

void plugin_ui_aspect_control_clear(plugin_ui_aspect_t aspect) {
    while(!TAILQ_EMPTY(&aspect->m_controls)) {
        plugin_ui_aspect_ref_t ref = TAILQ_FIRST(&aspect->m_controls);
        plugin_ui_control_t control = ref->m_obj;
        if (ref->m_is_tie) {
            plugin_ui_control_free(control);
        }
        else {
            plugin_ui_aspect_ref_free(ref, &aspect->m_controls, &control->m_aspects);
        }
    }
}

uint8_t plugin_ui_aspect_control_is_in(plugin_ui_aspect_t aspect, plugin_ui_control_t control) {
    plugin_ui_aspect_ref_t ref;

    TAILQ_FOREACH(ref, &control->m_aspects, m_next_for_obj) {
        if (ref->m_aspect == aspect) return 1;
    }

    return 0;
}

uint8_t plugin_ui_aspect_control_has_frame_in(plugin_ui_aspect_t aspect, plugin_ui_control_t control) {
    plugin_ui_control_frame_t frame;

    TAILQ_FOREACH(frame, &control->m_frames, m_next) {
        if (plugin_ui_aspect_control_frame_is_in(aspect, frame)) return 1;
    }

    return 0;
}

uint8_t plugin_ui_aspect_control_has_action_in(plugin_ui_aspect_t aspect, plugin_ui_control_t control) {
    if (control->m_action_slots) {
        uint8_t i;

        for(i = 0; i < CPE_ARRAY_SIZE(control->m_action_slots->m_actions); ++i) {
            plugin_ui_control_action_list_t * al = &control->m_action_slots->m_actions[i];
            plugin_ui_control_action_t action;

            TAILQ_FOREACH(action, al, m_next) {
                if (plugin_ui_aspect_control_action_is_in(aspect, action)) return 1;
            }
        }
    }

    return 0;
}

static plugin_ui_control_t plugin_ui_aspect_control_next(struct plugin_ui_control_it * it) {
    plugin_ui_aspect_ref_t * data = (plugin_ui_aspect_ref_t *)(it->m_data);
    plugin_ui_aspect_ref_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_aspect);

    return r->m_obj;
}

void plugin_ui_aspect_controls(plugin_ui_control_it_t control_it, plugin_ui_aspect_t aspect) {
    *(plugin_ui_aspect_ref_t *)(control_it->m_data) = TAILQ_FIRST(&aspect->m_controls);
    control_it->next = plugin_ui_aspect_control_next;
}

int plugin_ui_aspect_control_frame_add(plugin_ui_aspect_t aspect, plugin_ui_control_frame_t frame, uint8_t is_tie) {
    plugin_ui_env_t env = aspect->m_env;
    plugin_ui_aspect_ref_t ref;

    TAILQ_FOREACH(ref, &frame->m_aspects, m_next_for_obj) {
        if (ref->m_aspect == aspect) {
            if (is_tie) ref->m_is_tie = is_tie;
            return 0;
        }
    }
    
    if (plugin_ui_aspect_ref_create(aspect, &aspect->m_control_frames, frame, &frame->m_aspects, is_tie) == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "asspect %s: add frame %s: create ref fail",
            aspect->m_name, plugin_ui_control_frame_dump(&env->m_module->m_dump_buffer, frame));
        return -1;
    }

    return 0;
}

void plugin_ui_aspect_control_frame_remove(plugin_ui_aspect_t aspect, plugin_ui_control_frame_t frame) {
    plugin_ui_aspect_ref_t ref;

    TAILQ_FOREACH(ref, &frame->m_aspects, m_next_for_obj) {
        if (ref->m_aspect == aspect) {
            plugin_ui_aspect_ref_free(ref, &aspect->m_control_frames, &frame->m_aspects);
            return;
        }
    }
}

uint8_t plugin_ui_aspect_control_frame_is_in(plugin_ui_aspect_t aspect, plugin_ui_control_frame_t frame) {
    plugin_ui_aspect_ref_t ref;

    TAILQ_FOREACH(ref, &frame->m_aspects, m_next_for_obj) {
        if (ref->m_aspect == aspect) return 1;
    }

    return 0;
}

void plugin_ui_aspect_control_frame_clear(plugin_ui_aspect_t aspect) {
    while(!TAILQ_EMPTY(&aspect->m_control_frames)) {
        plugin_ui_aspect_ref_t ref = TAILQ_FIRST(&aspect->m_control_frames);
        plugin_ui_control_frame_t frame = ref->m_obj;
        if (ref->m_is_tie) {
            plugin_ui_control_frame_free(frame);
        }
        else {
            plugin_ui_aspect_ref_free(ref, &aspect->m_control_frames, &frame->m_aspects);
        }
    }
}

static plugin_ui_control_frame_t plugin_ui_aspect_control_frame_next(struct plugin_ui_control_frame_it * it) {
    plugin_ui_aspect_ref_t * data = (plugin_ui_aspect_ref_t *)(it->m_data);
    plugin_ui_aspect_ref_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_aspect);

    return r->m_obj;
}

void plugin_ui_aspect_control_frames(plugin_ui_control_frame_it_t control_frame_it, plugin_ui_aspect_t aspect) {
    *(plugin_ui_aspect_ref_t *)(control_frame_it->m_data) = aspect ? TAILQ_FIRST(&aspect->m_control_frames) : NULL;
    control_frame_it->next = plugin_ui_aspect_control_frame_next;
}

int plugin_ui_aspect_control_action_add(plugin_ui_aspect_t aspect, plugin_ui_control_action_t action, uint8_t is_tie) {
    plugin_ui_env_t env = aspect->m_env;
    plugin_ui_aspect_ref_t ref;

    TAILQ_FOREACH(ref, &action->m_aspects, m_next_for_obj) {
        if (ref->m_aspect == aspect) {
            if (is_tie) ref->m_is_tie = is_tie;
            return 0;
        }
    }
    
    if (plugin_ui_aspect_ref_create(aspect, &aspect->m_control_actions, action, &action->m_aspects, is_tie) == NULL) {
        CPE_ERROR(env->m_module->m_em, "asspect %s: add action: create ref fail", aspect->m_name);
        return -1;
    }

    return 0;
}

void plugin_ui_aspect_control_action_remove(plugin_ui_aspect_t aspect, plugin_ui_control_action_t action) {
    plugin_ui_aspect_ref_t ref;

    TAILQ_FOREACH(ref, &action->m_aspects, m_next_for_obj) {
        if (ref->m_aspect == aspect) {
            plugin_ui_aspect_ref_free(ref, &aspect->m_control_actions, &action->m_aspects);
            return;
        }
    }
}

uint8_t plugin_ui_aspect_control_action_is_in(plugin_ui_aspect_t aspect, plugin_ui_control_action_t action) {
    plugin_ui_aspect_ref_t ref;

    TAILQ_FOREACH(ref, &action->m_aspects, m_next_for_obj) {
        if (ref->m_aspect == aspect) return 1;
    }

    return 0;
}

void plugin_ui_aspect_control_action_clear(plugin_ui_aspect_t aspect) {
    while(!TAILQ_EMPTY(&aspect->m_control_actions)) {
        plugin_ui_aspect_ref_t ref = TAILQ_FIRST(&aspect->m_control_actions);
        plugin_ui_control_action_t action = ref->m_obj;
        if (ref->m_is_tie) {
            plugin_ui_control_action_free(action);
        }
        else {
            plugin_ui_aspect_ref_free(ref, &aspect->m_control_actions, &action->m_aspects);
        }
    }
}

static plugin_ui_control_action_t plugin_ui_aspect_control_action_next(struct plugin_ui_control_action_it * it) {
    plugin_ui_aspect_ref_t * data = (plugin_ui_aspect_ref_t *)(it->m_data);
    plugin_ui_aspect_ref_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_aspect);

    return r->m_obj;
}

void plugin_ui_aspect_control_actions(plugin_ui_control_action_it_t control_action_it, plugin_ui_aspect_t aspect) {
    *(plugin_ui_aspect_ref_t *)(control_action_it->m_data) = TAILQ_FIRST(&aspect->m_control_actions);
    control_action_it->next = plugin_ui_aspect_control_action_next;
}

/*env acctions*/
int plugin_ui_aspect_env_action_add(plugin_ui_aspect_t aspect, plugin_ui_env_action_t action, uint8_t is_tie) {
    plugin_ui_env_t env = aspect->m_env;
    plugin_ui_aspect_ref_t ref;

    TAILQ_FOREACH(ref, &action->m_aspects, m_next_for_obj) {
        if (ref->m_aspect == aspect) {
            if (is_tie) ref->m_is_tie = is_tie;
            return 0;
        }
    }
    
    if (plugin_ui_aspect_ref_create(aspect, &aspect->m_env_actions, action, &action->m_aspects, is_tie) == NULL) {
        CPE_ERROR(env->m_module->m_em, "asspect %s: add action: create ref fail", aspect->m_name);
        return -1;
    }

    return 0;
}

void plugin_ui_aspect_env_action_remove(plugin_ui_aspect_t aspect, plugin_ui_env_action_t action) {
    plugin_ui_aspect_ref_t ref;

    TAILQ_FOREACH(ref, &action->m_aspects, m_next_for_obj) {
        if (ref->m_aspect == aspect) {
            plugin_ui_aspect_ref_free(ref, &aspect->m_control_actions, &action->m_aspects);
            return;
        }
    }
}

uint8_t plugin_ui_aspect_env_action_is_in(plugin_ui_aspect_t aspect, plugin_ui_env_action_t action) {
    plugin_ui_aspect_ref_t ref;

    TAILQ_FOREACH(ref, &action->m_aspects, m_next_for_obj) {
        if (ref->m_aspect == aspect) return 1;
    }

    return 0;
}

void plugin_ui_aspect_env_action_clear(plugin_ui_aspect_t aspect) {
    while(!TAILQ_EMPTY(&aspect->m_env_actions)) {
        plugin_ui_aspect_ref_t ref = TAILQ_FIRST(&aspect->m_env_actions);
        plugin_ui_env_action_t action = ref->m_obj;
        if (ref->m_is_tie) {
            plugin_ui_env_action_free(action);
        }
        else {
            plugin_ui_aspect_ref_free(ref, &aspect->m_env_actions, &action->m_aspects);
        }
    }
}

static plugin_ui_env_action_t plugin_ui_aspect_env_action_next(struct plugin_ui_env_action_it * it) {
    plugin_ui_aspect_ref_t * data = (plugin_ui_aspect_ref_t *)(it->m_data);
    plugin_ui_aspect_ref_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_aspect);

    return r->m_obj;
}

void plugin_ui_aspect_env_actions(plugin_ui_env_action_it_t env_action_it, plugin_ui_aspect_t aspect) {
    *(plugin_ui_aspect_ref_t *)(env_action_it->m_data) = TAILQ_FIRST(&aspect->m_env_actions);
    env_action_it->next = plugin_ui_aspect_env_action_next;
}

int plugin_ui_aspect_animation_add(plugin_ui_aspect_t aspect, plugin_ui_animation_t animation, uint8_t is_tie) {
    plugin_ui_env_t env = aspect->m_env;
    plugin_ui_aspect_ref_t ref;

    TAILQ_FOREACH(ref, &animation->m_aspects, m_next_for_obj) {
        if (ref->m_aspect == aspect) {
            if (is_tie) ref->m_is_tie = is_tie;
            return 0;
        }
    }
    
    if (plugin_ui_aspect_ref_create(aspect, &aspect->m_animations, animation, &animation->m_aspects, is_tie) == NULL) {
        CPE_ERROR(env->m_module->m_em, "asspect %s: create ref fail", aspect->m_name);
        return -1;
    }

    return 0;
}

void plugin_ui_aspect_animation_remove(plugin_ui_aspect_t aspect, plugin_ui_animation_t animation) {
    plugin_ui_aspect_ref_t ref;

    TAILQ_FOREACH(ref, &animation->m_aspects, m_next_for_obj) {
        if (ref->m_aspect == aspect) {
            plugin_ui_aspect_ref_free(ref, &aspect->m_animations, &animation->m_aspects);
            return;
        }
    }
}

uint8_t plugin_ui_aspect_animation_is_in(plugin_ui_aspect_t aspect, plugin_ui_animation_t animation) {
    plugin_ui_aspect_ref_t ref;

    TAILQ_FOREACH(ref, &animation->m_aspects, m_next_for_obj) {
        if (ref->m_aspect == aspect) return 1;
    }

    return 0;
}

void plugin_ui_aspect_animation_clear(plugin_ui_aspect_t aspect) {
    while(!TAILQ_EMPTY(&aspect->m_animations)) {
        plugin_ui_aspect_ref_t ref = TAILQ_FIRST(&aspect->m_animations);
        plugin_ui_animation_t animation = ref->m_obj;
        if (ref->m_is_tie) {
            plugin_ui_animation_free(animation);
        }
        else {
            plugin_ui_aspect_ref_free(ref, &aspect->m_animations, &animation->m_aspects);
        }
    }
}

static plugin_ui_animation_t plugin_ui_aspect_animation_next(struct plugin_ui_animation_it * it) {
    plugin_ui_aspect_ref_t * data = (plugin_ui_aspect_ref_t *)(it->m_data);
    plugin_ui_aspect_ref_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_aspect);

    return r->m_obj;
}

void plugin_ui_aspect_animations(plugin_ui_animation_it_t animation_it, plugin_ui_aspect_t aspect) {
    *(plugin_ui_aspect_ref_t *)(animation_it->m_data) = TAILQ_FIRST(&aspect->m_animations);
    animation_it->next = plugin_ui_aspect_animation_next;
}
