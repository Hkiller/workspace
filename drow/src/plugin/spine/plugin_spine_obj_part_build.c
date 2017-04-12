#include "plugin_spine_obj_part_i.h"
#include "plugin_spine_data_state_def_i.h"

int plugin_spine_obj_build_parts(plugin_spine_obj_t obj, plugin_spine_data_state_def_t spine_state_def) {
    plugin_spine_module_t module = obj->m_module;
    plugin_spine_data_part_t part_def;
    plugin_spine_data_part_state_t state_def;
    plugin_spine_data_part_transition_t transition_def;
    int rv = 0;
    
    TAILQ_FOREACH(part_def, &spine_state_def->m_parts, m_next) {
        const char * part_name = plugin_spine_data_part_name(part_def);
        const char * part_init_state = plugin_spine_data_part_init_state(part_def);
        const char * part_init_anim = plugin_spine_data_part_init_anim(part_def);
        plugin_spine_obj_part_t part;

        part = plugin_spine_obj_part_create(obj, part_name);
        if (part == NULL) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_build_parts: create part %s fail!", part_name);
            rv = -1;
            continue;
        }

        TAILQ_FOREACH(state_def, &part_def->m_states, m_next) {
            const char * name = plugin_spine_data_part_state_name(state_def);
            const char * anim = plugin_spine_data_part_state_anim(state_def);
            plugin_spine_obj_part_state_t state;
            
            state = plugin_spine_obj_part_state_create(part, name);
            if (state == NULL) {
                CPE_ERROR(module->m_em, "plugin_spine_obj_build_parts: create part state  %s fail!", name);
                rv = -1;
                continue;
            }

            if (anim[0]) {
                if (plugin_spine_obj_part_state_set_anim(state, anim) != 0) {
                    CPE_ERROR(module->m_em, "plugin_spine_obj_build_parts: create part state  %s anim %s fail!", name, anim);
                    rv = -1;
                    continue;
                }
            }
        }

        TAILQ_FOREACH(transition_def, &part_def->m_transitions, m_next) {
            const char * str_from = plugin_spine_data_part_transition_from(transition_def);
            const char * str_to = plugin_spine_data_part_transition_to(transition_def);
            const char * str_name = plugin_spine_data_part_transition_name(transition_def);
            const char * str_anim = plugin_spine_data_part_transition_anim(transition_def);
            plugin_spine_obj_part_state_t from;
            plugin_spine_obj_part_state_t to;
            plugin_spine_obj_part_transition_t transition;

            from = plugin_spine_obj_part_state_find(part, str_from);
            if (from == NULL) {
                CPE_ERROR(
                    module->m_em, "plugin_spine_obj_build_parts: create part transition %s, from state %s not exist!",
                    str_name, str_from);
                rv = -1;
                continue;
            }

            to = plugin_spine_obj_part_state_find(part, str_to);
            if (to == NULL) {
                CPE_ERROR(
                    module->m_em, "plugin_spine_obj_build_parts: create part transition %s, to state %s not exist!",
                    str_name, str_to);
                rv = -1;
                continue;
            }
            
            transition = plugin_spine_obj_part_transition_create(from, to, str_name);
            if (transition == NULL) {
                CPE_ERROR(module->m_em, "plugin_spine_obj_build_parts: create part transition  %s fail!", str_name);
                rv = -1;
                continue;
            }

            if (str_anim[0]) {
                if (plugin_spine_obj_part_transition_set_anim(transition, str_anim) != 0) {
                    CPE_ERROR(module->m_em, "plugin_spine_obj_build_parts: create part transition  %s anim %s fail!", str_name, str_anim);
                    rv = -1;
                    continue;
                }
            }
        }

        if (part_init_state[0]) {
            if (plugin_spine_obj_part_set_cur_state_by_name(part, part_init_state) != 0) {
                CPE_ERROR(
                    module->m_em, "plugin_spine_obj_build_parts: set part  %s init state %s fail!",
                    part_name, part_init_state);
                rv = -1;
            }
        }

        if (part_init_anim[0]) {
            if (plugin_spine_obj_apply_anim(part->m_obj, part_init_anim) != 0) {
                CPE_ERROR(
                    module->m_em, "plugin_spine_obj_build_parts: apply part %s init anim %s fail!",
                    part_name, part_init_anim);
                rv = -1;
            }
        }
    }
    
    return rv;
}

