#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "render/model/ui_data_src.h"
#include "render/cache/ui_cache_group.h"
#include "plugin_spine_data_state_def_i.h"
#include "plugin_spine_atlas_i.h"

plugin_spine_data_state_def_t plugin_spine_data_state_def_create(plugin_spine_module_t module, ui_data_src_t src) {
    plugin_spine_data_state_def_t spine;

    if (ui_data_src_type(src) != ui_data_src_type_spine_state_def) {
        CPE_ERROR(
            module->m_em, "create spine state def >at %s: src not spine!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    if (ui_data_src_product(src)) {
        CPE_ERROR(
            module->m_em, "create spine state def at %s: product already loaded!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    spine = mem_alloc(module->m_alloc, sizeof(struct plugin_spine_data_state_def));
    if (spine == NULL) {
        CPE_ERROR(
            module->m_em, "create spine state def at %s: alloc fail!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    spine->m_module = module;
    spine->m_src = src;
    spine->m_part_count = 0;
    TAILQ_INIT(&spine->m_parts);

    ui_data_src_set_product(src, spine);

    return spine;
}

void plugin_spine_data_state_def_free(plugin_spine_data_state_def_t def) {
    plugin_spine_module_t module = def->m_module;

    while(!TAILQ_EMPTY(&def->m_parts)) {
        plugin_spine_data_part_free(TAILQ_FIRST(&def->m_parts));
    }
    assert(def->m_part_count == 0);

    mem_free(module->m_alloc, def);
}


/*part*/
plugin_spine_data_part_t plugin_spine_data_part_create(plugin_spine_data_state_def_t state_def) {
    plugin_spine_module_t module = state_def->m_module;
    plugin_spine_data_part_t part;

    part = mem_alloc(module->m_alloc, sizeof(struct plugin_spine_data_part));
    if (part == NULL) {
        CPE_ERROR(module->m_em, "plugin_spine_data_part_create: alloc fail!");
        return NULL;
    }

    part->m_def = state_def;
    TAILQ_INIT(&part->m_states);
    part->m_state_count = 0;
    TAILQ_INIT(&part->m_transitions);
    part->m_transition_count = 0;

    bzero(&part->m_data, sizeof(part->m_data));
    
    state_def->m_part_count++;
    TAILQ_INSERT_TAIL(&state_def->m_parts, part, m_next);

    return part;
}

void plugin_spine_data_part_free(plugin_spine_data_part_t part) {
    plugin_spine_module_t module = part->m_def->m_module;

    while(!TAILQ_EMPTY(&part->m_states)) {
        plugin_spine_data_part_state_free(TAILQ_FIRST(&part->m_states));
    }
    assert(part->m_state_count == 0);

    while(!TAILQ_EMPTY(&part->m_transitions)) {
        plugin_spine_data_part_transition_free(TAILQ_FIRST(&part->m_transitions));
    }
    assert(part->m_transition_count == 0);
    
    part->m_def->m_part_count--;
    TAILQ_REMOVE(&part->m_def->m_parts, part, m_next);

    mem_free(module->m_alloc, part);
}

SPINE_PART * plugin_spine_data_part_data(plugin_spine_data_part_t part) {
    return &part->m_data;
}

const char * plugin_spine_data_part_name(plugin_spine_data_part_t part) {
    return ui_data_src_msg(part->m_def->m_src, part->m_data.name);
}

const char * plugin_spine_data_part_init_state(plugin_spine_data_part_t part) {
    return ui_data_src_msg(part->m_def->m_src, part->m_data.init_state);
}

const char * plugin_spine_data_part_init_anim(plugin_spine_data_part_t part) {
    return ui_data_src_msg(part->m_def->m_src, part->m_data.init_anim);
}

/*state*/
plugin_spine_data_part_state_t plugin_spine_data_part_state_create(plugin_spine_data_part_t part) {
    plugin_spine_module_t module = part->m_def->m_module;
    plugin_spine_data_part_state_t state;

    state = mem_alloc(module->m_alloc, sizeof(struct plugin_spine_data_part_state));
    if (state == NULL) {
        CPE_ERROR(module->m_em, "plugin_spine_data_part_state_create: alloc fail!");
        return NULL;
    }

    state->m_part = part;

    bzero(&state->m_data, sizeof(state->m_data));

    part->m_state_count++;
    TAILQ_INSERT_TAIL(&part->m_states, state, m_next);

    return state;
}

void plugin_spine_data_part_state_free(plugin_spine_data_part_state_t state) {
    plugin_spine_module_t module = state->m_part->m_def->m_module;

    state->m_part->m_state_count--;
    TAILQ_REMOVE(&state->m_part->m_states, state, m_next);

    mem_free(module->m_alloc, state);
}

SPINE_PART_STATE * plugin_spine_data_part_state_data(plugin_spine_data_part_state_t state) {
    return &state->m_data;
}

const char * plugin_spine_data_part_state_name(plugin_spine_data_part_state_t state) {
    return ui_data_src_msg(state->m_part->m_def->m_src, state->m_data.name);
}

const char * plugin_spine_data_part_state_anim(plugin_spine_data_part_state_t state) {
    return ui_data_src_msg(state->m_part->m_def->m_src, state->m_data.anim);
}

/*transition*/
plugin_spine_data_part_transition_t plugin_spine_data_part_transition_create(plugin_spine_data_part_t part) {
    plugin_spine_module_t module = part->m_def->m_module;
    plugin_spine_data_part_transition_t transition;

    transition = mem_alloc(module->m_alloc, sizeof(struct plugin_spine_data_part_transition));
    if (transition == NULL) {
        CPE_ERROR(module->m_em, "plugin_spine_data_part_transition_create: alloc fail!");
        return NULL;
    }

    transition->m_part = part;

    bzero(&transition->m_data, sizeof(transition->m_data));
    
    part->m_transition_count++;
    TAILQ_INSERT_TAIL(&part->m_transitions, transition, m_next);

    return transition;
}

void plugin_spine_data_part_transition_free(plugin_spine_data_part_transition_t transition) {
    plugin_spine_module_t module = transition->m_part->m_def->m_module;

    transition->m_part->m_transition_count--;
    TAILQ_REMOVE(&transition->m_part->m_transitions, transition, m_next);

    mem_free(module->m_alloc, transition);
}

SPINE_PART_TRANSITION * plugin_spine_data_part_transition_data(plugin_spine_data_part_transition_t transition) {
    return &transition->m_data;
}

const char * plugin_spine_data_part_transition_from(plugin_spine_data_part_transition_t transition) {
    return ui_data_src_msg(transition->m_part->m_def->m_src, transition->m_data.from);
}

const char * plugin_spine_data_part_transition_to(plugin_spine_data_part_transition_t transition) {
    return ui_data_src_msg(transition->m_part->m_def->m_src, transition->m_data.to);
}

const char * plugin_spine_data_part_transition_name(plugin_spine_data_part_transition_t transition) {
    return ui_data_src_msg(transition->m_part->m_def->m_src, transition->m_data.name);
}

const char * plugin_spine_data_part_transition_anim(plugin_spine_data_part_transition_t transition) {
    return ui_data_src_msg(transition->m_part->m_def->m_src, transition->m_data.anim);
}    
