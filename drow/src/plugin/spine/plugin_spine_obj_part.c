#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin_spine_obj_part_i.h"
#include "plugin_spine_obj_track_i.h"
#include "plugin_spine_obj_anim_i.h"
#include "plugin_spine_data_state_def_i.h"

static void plugin_spine_obj_part_apply_all_anims(plugin_spine_obj_part_t part);
static void plugin_spine_obj_part_on_enter_anim_done(
    void * ctx, plugin_spine_obj_anim_t anim, plugin_spine_anim_event_type_t type, struct spEvent* event);

plugin_spine_obj_part_t
plugin_spine_obj_part_create(plugin_spine_obj_t obj, const char * part_name) {
    plugin_spine_module_t module = obj->m_module;
    plugin_spine_obj_part_t part;
    plugin_spine_obj_track_t track;

    if (plugin_spine_obj_part_find(obj, part_name) != NULL) {
        CPE_ERROR(module->m_em, "plugin_spine_obj_part_create: part %s already exist!", part_name);
        return NULL;
    }

    track = plugin_spine_obj_track_create(obj, part_name);
    if (track == NULL) {
        CPE_ERROR(module->m_em, "plugin_spine_obj_part_create: part %s create track fail!", part_name);
        return NULL;
    }

    part = TAILQ_FIRST(&module->m_free_parts);
    if (part) {
        TAILQ_REMOVE(&module->m_free_parts, part, m_next);
    }
    else {
        part = mem_alloc(module->m_alloc, sizeof(struct plugin_spine_obj_part));
        if (part == NULL) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_part_create: alloc fail!");
            plugin_spine_obj_track_free(track);
            return NULL;
        }
    }

    part->m_obj = obj;
    part->m_track = track;
    cpe_str_dup(part->m_name, sizeof(part->m_name), part_name);
    part->m_cur_state = NULL;
    part->m_state_count = 0;
    TAILQ_INIT(&part->m_states);
    part->m_enter_transition = NULL;
    part->m_enter_anim_pos = 0;
    part->m_enter_anim = NULL;
    part->m_enter_anim_delay_destory = NULL;
    part->m_state_anim = NULL;
    
    TAILQ_INSERT_TAIL(&obj->m_parts, part, m_next);

    return part;
}

void plugin_spine_obj_part_free(plugin_spine_obj_part_t part) {
    plugin_spine_obj_t obj = part->m_obj;
    plugin_spine_module_t module = obj->m_module;

    if (part->m_enter_anim) {
        plugin_spine_obj_anim_remove_track_listener(part->m_enter_anim, part);        
        plugin_spine_obj_anim_free(part->m_enter_anim);
        part->m_enter_anim = NULL;
    }

    if (part->m_enter_anim_delay_destory) {
        plugin_spine_obj_anim_free(part->m_enter_anim_delay_destory);
        part->m_enter_anim_delay_destory = NULL;
    }
    
    if (part->m_state_anim) {
        plugin_spine_obj_anim_free(part->m_state_anim);
        part->m_state_anim = NULL;
    }

    plugin_spine_obj_track_free(part->m_track);
    part->m_track = NULL;

    while(!TAILQ_EMPTY(&part->m_states)) {
        plugin_spine_obj_part_state_free(TAILQ_FIRST(&part->m_states));
    }
    assert(part->m_cur_state == NULL);
    assert(part->m_state_count == 0);
    
    TAILQ_REMOVE(&obj->m_parts, part, m_next);

    part->m_obj = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_parts, part, m_next);
}

const char * plugin_spine_obj_part_name(plugin_spine_obj_part_t part) {
    return part->m_name;
}

plugin_spine_obj_t  plugin_spine_obj_part_obj(plugin_spine_obj_part_t part) {
    return part->m_obj;
}

plugin_spine_obj_part_t plugin_spine_obj_part_find(plugin_spine_obj_t obj, const char * part_name) {
    plugin_spine_obj_part_t part;

    TAILQ_FOREACH(part, &obj->m_parts, m_next) {
        if (strcmp(part->m_name, part_name) == 0) return part;
    }

    return NULL;
}

uint8_t plugin_spine_obj_part_state_count(plugin_spine_obj_part_t part) {
    return part->m_state_count;
}

plugin_spine_obj_track_t plugin_spine_obj_part_track(plugin_spine_obj_part_t part) {
    return part->m_track;
}

plugin_spine_obj_part_state_t plugin_spine_obj_part_cur_state(plugin_spine_obj_part_t part) {
    return part->m_cur_state;
}

plugin_spine_obj_anim_t plugin_spine_obj_part_state_anim(plugin_spine_obj_part_t part) {
    return part->m_state_anim;
}

plugin_spine_obj_anim_t plugin_spine_obj_part_enter_anim(plugin_spine_obj_part_t part) {
    return part->m_enter_anim;
}

int plugin_spine_obj_part_set_cur_state(plugin_spine_obj_part_t part, plugin_spine_obj_part_state_t state) {
    plugin_spine_obj_part_apply_all_anims(part);

    part->m_cur_state = state;
    part->m_enter_transition = NULL;
    part->m_enter_anim_pos = 0;
    
    if (state->m_animation) {
        part->m_state_anim = plugin_spine_obj_anim_create_i(part->m_track, state->m_animation, 0);
        if (part->m_state_anim == NULL) {
            CPE_ERROR(part->m_obj->m_module->m_em, "state %s create anim fail", plugin_spine_obj_part_state_name(state));
            return -1;
        }
    }

    return 0;
}

int plugin_spine_obj_part_set_cur_state_by_name(plugin_spine_obj_part_t part, const char * name) {
    plugin_spine_obj_part_state_t state;

    if (part->m_cur_state && strcmp(part->m_cur_state->m_name, name) == 0) return 0;
    
    state = plugin_spine_obj_part_state_find(part, name);
    if (state == NULL) {
        CPE_ERROR(part->m_obj->m_module->m_em, "state %s not exist", name);
        return -1; 
    }

    return plugin_spine_obj_part_set_cur_state(part, state);
}

int plugin_spine_obj_part_switch_or_set_to_state(plugin_spine_obj_part_t part, plugin_spine_obj_part_state_t to_state, uint8_t force_change) {
    plugin_spine_obj_part_transition_t transition;
    
    if (!force_change && part->m_cur_state == to_state) return 0;

    transition = plugin_spine_obj_part_transition_find_by_target(part->m_cur_state, plugin_spine_obj_part_state_name(to_state));
    if (transition) {
        return plugin_spine_obj_part_apply_transition(part, transition);
    }
    else {
        return plugin_spine_obj_part_set_cur_state(part, to_state);
    }
}

int plugin_spine_obj_part_switch_or_set_to_state_by_name(plugin_spine_obj_part_t part, const char * name, uint8_t force_change) {
    plugin_spine_obj_part_state_t to_state = plugin_spine_obj_part_state_find(part, name);
    if (to_state == NULL) {
        CPE_ERROR(part->m_obj->m_module->m_em, "state %s not exist", name);
        return -1; 
    }

    return plugin_spine_obj_part_switch_or_set_to_state(part, to_state, force_change);
}

int plugin_spine_obj_part_apply_transition(plugin_spine_obj_part_t part, plugin_spine_obj_part_transition_t transition) {
    if (part->m_enter_transition == transition && transition->m_animation_count == 0) return 0;

    plugin_spine_obj_part_apply_all_anims(part);

    assert(part->m_enter_transition == NULL);
    assert(part->m_enter_anim == NULL);
    assert(part->m_enter_anim_pos == 0);
    
    part->m_enter_transition = transition;
    part->m_cur_state = transition->m_to;

    if (transition->m_animation_count) {
        struct plugin_spine_obj_part_transition_anim * anim_def = transition->m_animations;
        part->m_enter_anim = plugin_spine_obj_anim_create_i(part->m_track, anim_def->m_animation, anim_def->m_loop_count);
        if (part->m_enter_anim == NULL) {
            CPE_ERROR(
                part->m_obj->m_module->m_em, "part %s transition %s.%s create anim fail",
                part->m_name, transition->m_from->m_name, transition->m_name);
            return -1; 
        }

        plugin_spine_obj_anim_add_track_listener(
            part->m_enter_anim, plugin_spine_obj_part_on_enter_anim_done, part);

        assert(part->m_obj->m_anim_state->tracks[part->m_track->m_track_index]);
    }
    else if (part->m_cur_state->m_animation) {
        part->m_state_anim = plugin_spine_obj_anim_create_i(part->m_track, part->m_cur_state->m_animation, 0);
        if (part->m_state_anim == NULL) {
            CPE_ERROR(
                part->m_obj->m_module->m_em, "part %s state %s create anim fail",
                part->m_name, transition->m_to->m_name);
            return -1; 
        }
    }

    return 0;
}

int plugin_spine_obj_part_apply_transition_by_name(plugin_spine_obj_part_t part, const char * transition_name) {
    plugin_spine_obj_part_transition_t transition;

    if (part->m_cur_state == NULL) {
        CPE_ERROR(part->m_obj->m_module->m_em, "part %s no cur state", part->m_name);
        return -1;
    }

    transition = plugin_spine_obj_part_transition_find(part->m_cur_state, transition_name);
    if (transition == NULL) {
        CPE_ERROR(
            part->m_obj->m_module->m_em, "part %s cur state %s: no transaction %s",
            part->m_name, part->m_cur_state->m_name, transition_name);
        return -1;
    }

    return plugin_spine_obj_part_apply_transition(part, transition);
}

int plugin_spine_obj_part_apply_transition_force(plugin_spine_obj_part_t part, const char * name) {
    plugin_spine_obj_part_transition_t transition = NULL;

    if (part->m_cur_state) {
        transition = plugin_spine_obj_part_transition_find(part->m_cur_state, name);
    }

    if (transition) {
        return plugin_spine_obj_part_apply_transition(part, transition);
    }
    else {
        plugin_spine_obj_part_state_t state = plugin_spine_obj_part_state_find(part, name);
        if (state == NULL) {
            if (part->m_cur_state) {
                CPE_ERROR(
                    part->m_obj->m_module->m_em, "part %s no transition %s.%s or state %s",
                    part->m_name, part->m_cur_state->m_name, name, name);
            }
            else {
                CPE_ERROR(
                    part->m_obj->m_module->m_em, "part %s no state %s",
                    part->m_name, name);
            }
            return -1;
        }
        else {
            return plugin_spine_obj_part_set_cur_state(part, state);
        }
    }        
}

uint8_t plugin_spine_obj_part_is_in_enter(plugin_spine_obj_part_t part) {
    if (part->m_enter_anim && plugin_spine_obj_anim_is_playing(part->m_enter_anim)) {
        /* printf("xxxxx: %p: %s.%s is in enter %s\n", */
        /*        part->m_obj, ui_runtime_render_obj_name(ui_runtime_render_obj_from_data(part->m_obj)), */
        /*        plugin_spine_obj_part_name(part), */
        /*        part->m_enter_anim->m_track_entry->animation->name); */
        return 1;
    }
    else {
        return 0;
    }
}

void plugin_spine_obj_part_real_free_all(plugin_spine_module_t module) {
    while(!TAILQ_EMPTY(&module->m_free_parts)) {
        plugin_spine_obj_part_t part = TAILQ_FIRST(&module->m_free_parts);
        TAILQ_REMOVE(&module->m_free_parts, part, m_next);
        mem_free(module->m_alloc, part);
    }
}

static void plugin_spine_obj_part_apply_all_anims(plugin_spine_obj_part_t part) {
    plugin_spine_module_t module = part->m_obj->m_module;
    int events_buf_count = 0;

    if (part->m_enter_anim_delay_destory) {
        plugin_spine_obj_anim_free(part->m_enter_anim_delay_destory);
        part->m_enter_anim_delay_destory = NULL;
    }
    
    if (part->m_enter_transition) {
        uint8_t i;
        
        if (part->m_enter_anim) {
            plugin_spine_obj_track_apply_all_animations(part->m_track);
            plugin_spine_obj_anim_remove_track_listener(part->m_enter_anim, part);        
            plugin_spine_obj_anim_free(part->m_enter_anim);
            part->m_enter_anim = NULL;
        }

        for(i = part->m_enter_anim_pos + 1; i < part->m_enter_transition->m_animation_count; ++i) {
            spAnimation* animation = part->m_enter_transition->m_animations[i].m_animation;
            spAnimation_apply(
                animation,
                part->m_obj->m_skeleton, -1.0f, animation->duration, 0,
                module->m_events_buf, &events_buf_count);
        }

        part->m_enter_transition = NULL;
        part->m_enter_anim_pos = 0;
    }
    else {
        assert(part->m_enter_anim == NULL);
        assert(part->m_enter_anim_pos == 0);
    }
            
    if(part->m_state_anim) {
        plugin_spine_obj_track_apply_all_animations(part->m_track);
        plugin_spine_obj_anim_free(part->m_state_anim);
        part->m_state_anim = NULL;
    }
    else if (part->m_cur_state && part->m_cur_state->m_animation) {
        spAnimation* animation = part->m_cur_state->m_animation;
        spAnimation_apply(
            animation,
            part->m_obj->m_skeleton, -1.0f, animation->duration, 0,
            module->m_events_buf, &events_buf_count);
    }
}

static void plugin_spine_obj_part_on_enter_anim_done(
    void * ctx, plugin_spine_obj_anim_t anim, plugin_spine_anim_event_type_t type, struct spEvent* event)
{
    plugin_spine_obj_part_t part = ctx;

    if (part->m_enter_anim_delay_destory) {
        plugin_spine_obj_anim_free(part->m_enter_anim_delay_destory);
        part->m_enter_anim_delay_destory = NULL;
    }
    
    if (type != plugin_spine_anim_event_complete) return;
    
    plugin_spine_obj_anim_remove_track_listener(part->m_enter_anim, part);        
    part->m_enter_anim_delay_destory = part->m_enter_anim;
    part->m_enter_anim = NULL;

    assert(part->m_enter_transition);
    part->m_enter_anim_pos++;
    if (part->m_enter_anim_pos < part->m_enter_transition->m_animation_count) {
        struct plugin_spine_obj_part_transition_anim * anim_def = &part->m_enter_transition->m_animations[part->m_enter_anim_pos];
        part->m_enter_anim = plugin_spine_obj_anim_create_i(part->m_track, anim_def->m_animation, anim_def->m_loop_count);
        if (part->m_enter_anim == NULL) {
            CPE_ERROR(
                part->m_obj->m_module->m_em, "part %s transition %s.%s create anim fail",
                part->m_name, part->m_enter_transition->m_from->m_name, part->m_enter_transition->m_name);
            return; 
        }

        plugin_spine_obj_anim_add_track_listener(
            part->m_enter_anim, plugin_spine_obj_part_on_enter_anim_done, part);
    }
    else if (part->m_cur_state->m_animation) {
        part->m_state_anim = plugin_spine_obj_anim_create_i(part->m_track, part->m_cur_state->m_animation, 0);
        if (part->m_state_anim == NULL) {
            CPE_ERROR(
                part->m_obj->m_module->m_em, "part %s state %s create anim fail",
                part->m_name, part->m_cur_state->m_name);
            return; 
        }
    }
}
