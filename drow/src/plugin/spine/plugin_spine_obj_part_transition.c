#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "plugin_spine_obj_part_i.h"

plugin_spine_obj_part_transition_t
plugin_spine_obj_part_transition_create(plugin_spine_obj_part_state_t from, plugin_spine_obj_part_state_t to, const char * transition_name) {
    plugin_spine_module_t module = from->m_part->m_obj->m_module;
    plugin_spine_obj_part_transition_t transition;

    if (plugin_spine_obj_part_transition_find(from, transition_name) != NULL) {
        CPE_ERROR(module->m_em, "plugin_spine_obj_part_transition_create: part %s already exist!", transition_name);
        return NULL;
    }

    transition = TAILQ_FIRST(&module->m_free_part_transitions);
    if (transition) {
        TAILQ_REMOVE(&module->m_free_part_transitions, transition, m_next_for_from);
    }
    else {
        transition = mem_alloc(module->m_alloc, sizeof(struct plugin_spine_obj_part_transition));
        if (transition == NULL) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_part_transition_create: alloc fail!");
            return NULL;
        }
    }

    transition->m_from = from;
    transition->m_to = to;
    cpe_str_dup(transition->m_name, sizeof(transition->m_name), transition_name);
    transition->m_animation_count = 0;
    
    TAILQ_INSERT_TAIL(&from->m_as_from_transitions, transition, m_next_for_from);
    TAILQ_INSERT_TAIL(&to->m_as_to_transitions, transition, m_next_for_to);

    return transition;
}

void plugin_spine_obj_part_transition_free(plugin_spine_obj_part_transition_t transition) {
    plugin_spine_obj_part_t part = transition->m_from->m_part;
    plugin_spine_module_t module = part->m_obj->m_module;

    TAILQ_REMOVE(&transition->m_from->m_as_from_transitions, transition, m_next_for_from);
    TAILQ_REMOVE(&transition->m_to->m_as_to_transitions, transition, m_next_for_to);

    transition->m_from = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_part_transitions, transition, m_next_for_from);
}

plugin_spine_obj_part_transition_t
plugin_spine_obj_part_transition_find(plugin_spine_obj_part_state_t from, const char * name) {
    plugin_spine_obj_part_transition_t transition;

    TAILQ_FOREACH(transition, &from->m_as_from_transitions, m_next_for_from) {
        if (strcmp(transition->m_name, name) == 0) return transition;
    }

    return NULL;
}

plugin_spine_obj_part_transition_t
plugin_spine_obj_part_transition_find_by_target(plugin_spine_obj_part_state_t from, const char * target_state) {
    plugin_spine_obj_part_transition_t transition;

    TAILQ_FOREACH(transition, &from->m_as_from_transitions, m_next_for_from) {
        if (strcmp(transition->m_to->m_name, target_state) == 0) return transition;
    }

    return NULL;
}

const char * plugin_spine_obj_part_transition_name(plugin_spine_obj_part_transition_t transition) {
    return transition->m_name;
}

static int plugin_spine_obj_part_transition_add_anim_by_def_i(
    plugin_spine_obj_t obj, plugin_spine_obj_part_transition_t transition, const char * begin, size_t len)
{
    char anim_name[64];
    char * loop_count = NULL;
    struct plugin_spine_obj_part_transition_anim * anim;

    if (transition->m_animation_count + 1 > CPE_ARRAY_SIZE(transition->m_animations)) {
        CPE_ERROR(
            obj->m_module->m_em,
            "plugin_spine_obj_part_transition_set_anim: animation count overflow!!");
		return -1;
    }

    if (len + 1 > CPE_ARRAY_SIZE(anim_name)) {
        CPE_ERROR(
            obj->m_module->m_em,
            "plugin_spine_obj_part_transition_set_anim: animation name len overflow!!");
		return -1;
    }

    memcpy(anim_name, begin, len);
    anim_name[len] = 0;

    loop_count = strchr(anim_name, '*');
    if (loop_count) {
        *loop_count = 0;
        loop_count = loop_count + 1;
    }

    anim = transition->m_animations + transition->m_animation_count;
    
    anim->m_animation = spSkeletonData_findAnimation(obj->m_skeleton->data, anim_name);
	if (!anim->m_animation) {
        CPE_ERROR(
            obj->m_module->m_em,
            "plugin_spine_obj_part_transition_set_anim: anim %s not exist!", anim_name);
		return -1;
	}
    anim->m_loop_count = loop_count ? atoi(loop_count) : 1;

    if (anim->m_loop_count == 0) {
        CPE_ERROR(
            obj->m_module->m_em,
            "plugin_spine_obj_part_transition_set_anim: anim %s loop count %d error, cant use circle loop in transition!",
            anim_name, anim->m_loop_count);
		return -1;
    }

    transition->m_animation_count++;
    return 0;
}

int plugin_spine_obj_part_transition_add_anim_by_def(plugin_spine_obj_part_transition_t transition, const char * def) {
    plugin_spine_obj_t obj = transition->m_from->m_part->m_obj;

    return plugin_spine_obj_part_transition_add_anim_by_def_i(obj, transition, def, strlen(def));
}

int plugin_spine_obj_part_transition_set_anim(plugin_spine_obj_part_transition_t transition, const char * anim_def) {
    plugin_spine_obj_t obj = transition->m_from->m_part->m_obj;
    const char * anim_sep;

    transition->m_animation_count = 0;
    while((anim_sep = strchr(anim_def, '+'))) {
        if (plugin_spine_obj_part_transition_add_anim_by_def_i(obj, transition, anim_def, anim_sep - anim_def) != 0) return -1;
        anim_def = anim_sep + 1;
    }

    if (anim_def[0]) {
        if (plugin_spine_obj_part_transition_add_anim_by_def(transition, anim_def) != 0) return -1;
    }

    return 0;
}

void plugin_spine_obj_part_transition_real_free_all(plugin_spine_module_t module) {
    while(!TAILQ_EMPTY(&module->m_free_part_transitions)) {
        plugin_spine_obj_part_transition_t part_transition = TAILQ_FIRST(&module->m_free_part_transitions);
        TAILQ_REMOVE(&module->m_free_part_transitions, part_transition, m_next_for_from);
        mem_free(module->m_alloc, part_transition);
    }
}
