#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin_spine_obj_part_i.h"

static int plugin_spine_obj_scane_parts_part_find_cur_state(plugin_spine_module_t module, plugin_spine_obj_part_t part);
static int plugin_spine_obj_scane_parts_scane_transition(
    plugin_spine_module_t module, plugin_spine_obj_part_t part, spAnimation * anim, char * arg, const char * left_args);
static int plugin_spine_obj_scane_parts_scane_state(
    plugin_spine_module_t module, plugin_spine_obj_part_t part, spAnimation * anim, char * arg, const char * left_args);

int plugin_spine_obj_scane_parts(plugin_spine_obj_t obj) {
    plugin_spine_module_t module = obj->m_module;
    spSkeleton* skeleton = obj->m_skeleton;
    int animations_pos;
    int rv = 0;
    plugin_spine_obj_part_t part;
    
    if (!TAILQ_EMPTY(&obj->m_parts)) {
        CPE_ERROR(module->m_em, "plugin_spine_obj_scane_parts: already have parts!");
        return -1;
    }

    /*根据动画名自动生成部件和跳转关系 */
    for(animations_pos = 0; animations_pos < skeleton->data->animationsCount; ++animations_pos) {
        spAnimation * anim = skeleton->data->animations[animations_pos];
        const char * anim_name = anim->name;
        const char * end;
        char name_buf[128];
        
        if (!cpe_str_start_with(anim_name, "P[")) continue;

        anim_name += 2; /*P[*/
        
        end = strchr(anim_name, ']');
        if (end == NULL) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_scane_parts: anim name %s format error!", anim->name);
            rv = -1;
            continue;
        }

        if (cpe_str_dup_range(name_buf, sizeof(name_buf), anim_name, end) == NULL) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_scane_parts: anim name %s part name dup fail!", anim->name);
            rv = -1;
            continue;
        }
        
        part = plugin_spine_obj_part_find(obj, name_buf);
        if (part == NULL) {
            part = plugin_spine_obj_part_create(obj, name_buf);
            if (part == NULL) {
                CPE_ERROR(module->m_em, "plugin_spine_obj_scane_parts: anim name %s create part %s fail!", anim->name, name_buf);
                rv = -1;
                continue;
            }
        }
        anim_name = end + 1;

        if (anim_name[0] != '_') {
            CPE_ERROR(module->m_em, "plugin_spine_obj_scane_parts: anim name %s left %s format fail(expect '_')!", anim->name, anim_name);
            rv = -1;
            continue;
        }
        anim_name++;

        end = strchr(anim_name, ']');
        if (end == NULL) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_scane_parts: anim name %s left %s format error!", anim->name, anim_name);
            rv = -1;
            continue;
        }

        if (cpe_str_start_with(anim_name, "T[")) {
            anim_name += 2;
            if (cpe_str_dup_range(name_buf, sizeof(name_buf), anim_name, end) == NULL) {
                CPE_ERROR(module->m_em, "plugin_spine_obj_scane_parts: anim name %s transition from state %s dup fail!", anim->name, anim_name);
                rv = -1;
                continue;
            }

            if (plugin_spine_obj_scane_parts_scane_transition(module, part, anim, name_buf, end + 1) != 0) {
                rv = -1;
                continue;
            }
        }
        else if (cpe_str_start_with(anim_name, "S[")) {
            anim_name += 2;
            if (cpe_str_dup_range(name_buf, sizeof(name_buf), anim_name, end) == NULL) {
                CPE_ERROR(module->m_em, "plugin_spine_obj_scane_parts: anim name %s transition from state %s dup fail!", anim->name, anim_name);
                rv = -1;
                continue;
            }

            if (plugin_spine_obj_scane_parts_scane_state(module, part, anim, name_buf, end + 1) != 0) {
                rv = -1;
                continue;
            }
        }
        else {
            CPE_ERROR(module->m_em, "plugin_spine_obj_scane_parts: anim name %s format %s fail!", anim->name, anim_name);
            rv = -1;
            continue;
        }
    }

    /*构造当前状态 */
    TAILQ_FOREACH(part, &obj->m_parts, m_next) {
        if (part->m_cur_state != NULL) continue;

        if (plugin_spine_obj_scane_parts_part_find_cur_state(module, part) != 0) {
            rv = -1;
            break;;
        }
    }
    
    return rv;
}

static int plugin_spine_obj_scane_parts_part_find_cur_state(plugin_spine_module_t module, plugin_spine_obj_part_t part) {
    plugin_spine_obj_part_state_t state;

    TAILQ_FOREACH(state, &part->m_states, m_next) {
        if (TAILQ_EMPTY(&state->m_as_to_transitions)) {
            plugin_spine_obj_part_set_cur_state_by_name(part, state->m_name);
            return 0;
        }
    }

    return 0;
}

static int plugin_spine_obj_scane_parts_scane_transition(
    plugin_spine_module_t module, plugin_spine_obj_part_t part, spAnimation * anim, char * arg, const char * left_args)
{
    plugin_spine_obj_part_state_t from_state;
    plugin_spine_obj_part_state_t to_state;
    const char * transition_name = NULL;
    plugin_spine_obj_part_transition_t transition;
    char * sep;

    if ((sep = strchr(arg, ':'))) {
        *sep = 0;
        transition_name = sep + 1;
    }

    if ((sep = strchr(arg, '-'))) {
        *sep = 0;
        
        from_state = plugin_spine_obj_part_state_find(part, arg);
        if (from_state == NULL) {
            from_state = plugin_spine_obj_part_state_create(part, arg);
            if (from_state == NULL) {
                CPE_ERROR(
                    module->m_em, "plugin_spine_obj_scane_parts: anim name %s part %s create from state %s fail!",
                    anim->name, plugin_spine_obj_part_name(part), arg);
                return -1;
            }
        }

        /*加载to_state */
        to_state = plugin_spine_obj_part_state_find(part, sep + 1);
        if (to_state == NULL) {
            to_state = plugin_spine_obj_part_state_create(part, sep + 1);
            if (to_state == NULL) {
                CPE_ERROR(
                    module->m_em, "plugin_spine_obj_scane_parts: anim name %s part %s create to state %s fail!",
                    anim->name, plugin_spine_obj_part_name(part), sep + 1);
                return -1;
            }
        }
    }
    else {
        from_state = plugin_spine_obj_part_state_find(part, arg);
        if (from_state == NULL) {
            from_state = plugin_spine_obj_part_state_create(part, arg);
            if (from_state == NULL) {
                CPE_ERROR(
                    module->m_em, "plugin_spine_obj_scane_parts: anim name %s part %s create from state %s fail!",
                    anim->name, plugin_spine_obj_part_name(part), arg);
                return -1;
            }
        }
        
        to_state = from_state;
    }
            
    /*创建transition*/
    if (transition_name == NULL) {
        transition_name = to_state->m_name;
    }
    
    transition = plugin_spine_obj_part_transition_create(from_state, to_state, transition_name);
    if (transition == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_spine_obj_scane_parts: anim name %s part %s create transition fail!",
            anim->name, plugin_spine_obj_part_name(part));
        return -1;
    }

    transition->m_animations[transition->m_animation_count].m_animation = anim;
    transition->m_animations[transition->m_animation_count].m_loop_count = 1;
    transition->m_animation_count++;

    if (left_args[0] == '+') {
        if (plugin_spine_obj_part_set_cur_state_by_name(part, transition->m_from->m_name) != 0
            || plugin_spine_obj_part_apply_transition(part, transition) != 0)
        {
            CPE_ERROR(
                module->m_em, "plugin_spine_obj_scane_parts: anim name %s part %s apply transition %s[%s ==> %s] fail!",
                anim->name, plugin_spine_obj_part_name(part), transition_name, from_state->m_name, to_state->m_name);
            return -1;
        }
    }

    return 0;
}

static int plugin_spine_obj_scane_parts_scane_state(
    plugin_spine_module_t module, plugin_spine_obj_part_t part, spAnimation * anim, char * arg, const char * left_args)
{
    plugin_spine_obj_part_state_t state;

    state = plugin_spine_obj_part_state_find(part, arg);
    if (state == NULL) {
        state = plugin_spine_obj_part_state_create(part, arg);
        if (state == NULL) {
            CPE_ERROR(
                module->m_em, "plugin_spine_obj_scane_parts: anim name %s part %s create state %s fail!",
                anim->name, plugin_spine_obj_part_name(part), arg);
            return -1;
        }
    }

    if (state->m_animation != NULL) {
        CPE_ERROR(
            module->m_em, "plugin_spine_obj_scane_parts: anim name %s part %s state %s already have animation %s!",
            anim->name, plugin_spine_obj_part_name(part), arg,
            state->m_animation->name);
        return -1;
    }
            
    state->m_animation = anim;

    if (left_args[0] == '+') {
        if (plugin_spine_obj_part_set_cur_state_by_name(part, state->m_name) != 0) {
            CPE_ERROR(
                module->m_em, "plugin_spine_obj_scane_parts: anim name %s part %s set cur state %s fail!",
                anim->name, plugin_spine_obj_part_name(part), state->m_name);
            return -1;
        }
    }

    return 0;
}
