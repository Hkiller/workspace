#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin_spine_obj_part_i.h"

plugin_spine_obj_part_state_t
plugin_spine_obj_part_state_create(plugin_spine_obj_part_t part, const char * state_name) {
    plugin_spine_module_t module = part->m_obj->m_module;
    plugin_spine_obj_part_state_t state;

    if (plugin_spine_obj_part_state_find(part, state_name) != NULL) {
        CPE_ERROR(module->m_em, "plugin_spine_obj_part_state_create: part %s already exist!", state_name);
        return NULL;
    }

    state = TAILQ_FIRST(&module->m_free_part_states);
    if (state) {
        TAILQ_REMOVE(&module->m_free_part_states, state, m_next);
    }
    else {
        state = mem_alloc(module->m_alloc, sizeof(struct plugin_spine_obj_part_state));
        if (state == NULL) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_part_state_create: alloc fail!");
            return NULL;
        }
    }

    state->m_part = part;
    cpe_str_dup(state->m_name, sizeof(state->m_name), state_name);
    state->m_animation = NULL;

    TAILQ_INIT(&state->m_as_from_transitions);
    TAILQ_INIT(&state->m_as_to_transitions);

    part->m_state_count++;
    TAILQ_INSERT_TAIL(&part->m_states, state, m_next);

    return state;
}

void plugin_spine_obj_part_state_free(plugin_spine_obj_part_state_t state) {
    plugin_spine_obj_part_t part = state->m_part;
    plugin_spine_module_t module = part->m_obj->m_module;

    if (part->m_cur_state == state) {
        part->m_cur_state = NULL;
    }

    while(!TAILQ_EMPTY(&state->m_as_from_transitions)) {
        plugin_spine_obj_part_transition_free(TAILQ_FIRST(&state->m_as_from_transitions));
    }

    while(!TAILQ_EMPTY(&state->m_as_to_transitions)) {
        plugin_spine_obj_part_transition_free(TAILQ_FIRST(&state->m_as_to_transitions));
    }

    assert(part->m_state_count > 0);
    part->m_state_count--;
    TAILQ_REMOVE(&part->m_states, state, m_next);

    state->m_part = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_part_states, state, m_next);
}

plugin_spine_obj_part_state_t
plugin_spine_obj_part_state_find(plugin_spine_obj_part_t part, const char * name) {
    plugin_spine_obj_part_state_t state;

    TAILQ_FOREACH(state, &part->m_states, m_next) {
        if (strcmp(state->m_name, name) == 0) return state;
    }

    return NULL;
}


int plugin_spine_obj_part_state_set_anim(plugin_spine_obj_part_state_t state, const char * anim_name) {
    plugin_spine_obj_t obj = state->m_part->m_obj;

    if (anim_name) {
        spAnimation * animation;
        
        animation = spSkeletonData_findAnimation(obj->m_skeleton->data, anim_name);
        if (!animation) {
            CPE_ERROR(obj->m_module->m_em, "plugin_spine_obj_part_state_set_anim: anim %s not exist!", anim_name);
            return -1;
        }

        state->m_animation = animation;
    }
    else {
        state->m_animation = NULL;
    }

    return 0;
}

const char * plugin_spine_obj_part_state_name(plugin_spine_obj_part_state_t state) {
    return state->m_name;
}

void plugin_spine_obj_part_state_real_free_all(plugin_spine_module_t module) {
    while(!TAILQ_EMPTY(&module->m_free_part_states)) {
        plugin_spine_obj_part_state_t part_state = TAILQ_FIRST(&module->m_free_part_states);
        TAILQ_REMOVE(&module->m_free_part_states, part_state, m_next);
        mem_free(module->m_alloc, part_state);
    }
}

static plugin_spine_obj_part_state_t plugin_spine_obj_part_state_next(struct plugin_spine_obj_part_state_it * it) {
    plugin_spine_obj_part_state_t * data = (plugin_spine_obj_part_state_t *)(it->m_data);
    plugin_spine_obj_part_state_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void plugin_spine_obj_part_states(plugin_spine_obj_part_t part, plugin_spine_obj_part_state_it_t it) {
    *(plugin_spine_obj_part_state_t *)(it->m_data) = TAILQ_FIRST(&part->m_states);
    it->next = plugin_spine_obj_part_state_next;
}
