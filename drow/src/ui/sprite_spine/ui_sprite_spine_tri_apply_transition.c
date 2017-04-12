#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "plugin/spine/plugin_spine_obj.h"
#include "plugin/spine/plugin_spine_obj_part.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_tri/ui_sprite_tri_action.h"
#include "ui/sprite_tri/ui_sprite_tri_action_meta.h"
#include "ui_sprite_spine_tri_apply_transition_i.h"
#include "ui_sprite_spine_utils_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_spine_tri_apply_transition_t
ui_sprite_spine_tri_apply_transition_create(ui_sprite_tri_rule_t rule) {
    ui_sprite_tri_action_t action;
    
    action = ui_sprite_tri_action_create(rule, UI_SPRITE_SPINE_TRI_APPLY_TRANSITION);
    if (action == NULL) return NULL;

    return (ui_sprite_spine_tri_apply_transition_t)ui_sprite_tri_action_data(action);
}
    
void ui_sprite_spine_tri_apply_transition_free(ui_sprite_spine_tri_apply_transition_t apply_transition) {
    ui_sprite_tri_action_t action;

    action = ui_sprite_tri_action_from_data(apply_transition);

    ui_sprite_tri_action_free(action);
}

plugin_spine_obj_t ui_sprite_spine_tri_apply_transition_obj(ui_sprite_spine_tri_apply_transition_t apply_transition) {
    return apply_transition->m_obj;
}

void ui_sprite_spine_tri_apply_transition_set_obj(ui_sprite_spine_tri_apply_transition_t apply_transition, plugin_spine_obj_t obj) {
    apply_transition->m_obj = obj;
}
    
const char * ui_sprite_spine_tri_apply_transition_part(ui_sprite_spine_tri_apply_transition_t apply_transition) {
    return apply_transition->m_part;
}
    
int ui_sprite_spine_tri_apply_transition_set_part(ui_sprite_spine_tri_apply_transition_t apply_transition, const char * part) {
    ui_sprite_spine_module_t module = apply_transition->m_module;
    char * new_part = NULL;

    if (part) {
        new_part = cpe_str_mem_dup(module->m_alloc, part);
        if (new_part == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_spine_tri_apply_transition_init: dup spine module %s fail", part);
            return -1;
        }
    }

    if (apply_transition->m_part) mem_free(module->m_alloc, apply_transition->m_part);

    apply_transition->m_part = new_part;

    return 0;
}

const char * ui_sprite_spine_tri_apply_transition_transition(ui_sprite_spine_tri_apply_transition_t apply_transition) {
    return apply_transition->m_transition;
}
    
int ui_sprite_spine_tri_apply_transition_set_transition(ui_sprite_spine_tri_apply_transition_t apply_transition, const char * transition) {
    ui_sprite_spine_module_t module = apply_transition->m_module;
    char * new_transition = NULL;

    if (transition) {
        new_transition = cpe_str_mem_dup(module->m_alloc, transition);
        if (new_transition == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_spine_tri_apply_transition_init: dup require count %s fail", transition);
            return -1;
        }
    }

    if (apply_transition->m_transition) mem_free(module->m_alloc, apply_transition->m_transition);

    apply_transition->m_transition = new_transition;

    return 0;
}

static int ui_sprite_spine_tri_apply_transition_init(void * ctx, ui_sprite_tri_action_t action) {
    ui_sprite_spine_tri_apply_transition_t data = (ui_sprite_spine_tri_apply_transition_t)ui_sprite_tri_action_data(action);
    ui_sprite_spine_module_t module = (ui_sprite_spine_module_t)ctx;
    
    data->m_module = module;
    data->m_obj = NULL;
    data->m_part = NULL;
    data->m_transition = NULL;
    
    return 0;
}

static void ui_sprite_spine_tri_apply_transition_fini(void * ctx, ui_sprite_tri_action_t action) {
    ui_sprite_spine_module_t module = (ui_sprite_spine_module_t)ctx;
    ui_sprite_spine_tri_apply_transition_t apply_transition = (ui_sprite_spine_tri_apply_transition_t)ui_sprite_tri_action_data(action);
    
    if (apply_transition->m_part) {
        mem_free(module->m_alloc, apply_transition->m_part);
        apply_transition->m_part = NULL;
    }
    
    if (apply_transition->m_transition == NULL) {
        mem_free(module->m_alloc, apply_transition->m_transition);
        apply_transition->m_transition = NULL;
    }
}

static int ui_sprite_spine_tri_apply_transition_copy(void * ctx, ui_sprite_tri_action_t action, ui_sprite_tri_action_t source) {
    ui_sprite_spine_module_t module = (ui_sprite_spine_module_t)ctx;
    ui_sprite_spine_tri_apply_transition_t to_data = (ui_sprite_spine_tri_apply_transition_t)ui_sprite_tri_action_data(action);
    ui_sprite_spine_tri_apply_transition_t from_data = (ui_sprite_spine_tri_apply_transition_t)ui_sprite_tri_action_data(source);

    if (ui_sprite_spine_tri_apply_transition_init(ctx, action) != 0) return -1;

    if (from_data->m_part) {
        to_data->m_part = cpe_str_mem_dup(module->m_alloc, from_data->m_part);
    }

    if (from_data->m_transition) {
        to_data->m_transition = cpe_str_mem_dup(module->m_alloc, from_data->m_transition);
    }
    
    return 0;
}

static void ui_sprite_spine_tri_apply_transition_exec(void * ctx, ui_sprite_tri_action_t action) {
    ui_sprite_spine_module_t module = (ui_sprite_spine_module_t)ctx;
    ui_sprite_spine_tri_apply_transition_t apply_transition = (ui_sprite_spine_tri_apply_transition_t)ui_sprite_tri_action_data(action);
    ui_sprite_entity_t entity = ui_sprite_tri_action_entity(action);
    plugin_spine_obj_part_t part;
    plugin_spine_obj_part_state_t cur_state;
    plugin_spine_obj_part_transition_t transition = NULL;
    
    if (apply_transition->m_part == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-apply-transition: part not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (apply_transition->m_transition == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-apply-transition: transition not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (apply_transition->m_obj) {
        part = plugin_spine_obj_part_find(apply_transition->m_obj, apply_transition->m_part);
        if (part == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine-apply-transition: part %s not exist in obj!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), apply_transition->m_part);
            return;
        }
    }
    else {
        part = ui_sprite_spine_find_obj_part(module->m_sprite_render, entity, apply_transition->m_part, module->m_em);
        if (part == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine-apply-transition: part %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), apply_transition->m_part);
            return;
        }
    }

    cur_state = plugin_spine_obj_part_cur_state(part);

    transition = cur_state ? plugin_spine_obj_part_transition_find(cur_state, apply_transition->m_transition) : NULL;

    if (transition) {
        if (plugin_spine_obj_part_apply_transition(part, transition) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine-apply-transition: part %s apply transition %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), apply_transition->m_part, apply_transition->m_transition);
            return;
        }

        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): spine-apply-transition: part %s apply transition %s success!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), apply_transition->m_part, apply_transition->m_transition);
        }
    }
    else {
        if (plugin_spine_obj_part_set_cur_state_by_name(part, apply_transition->m_transition) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine-apply-transition: part %s set state %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), apply_transition->m_part, apply_transition->m_transition);
            return;
        }

        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): spine-apply-transition: part %s set state %s success!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), apply_transition->m_part, apply_transition->m_transition);
        }
    }
}

int ui_sprite_spine_tri_apply_transition_regist(ui_sprite_spine_module_t module) {
    if (module->m_tri) {
        ui_sprite_tri_action_meta_t meta;

        meta = ui_sprite_tri_action_meta_create(
            module->m_tri, UI_SPRITE_SPINE_TRI_APPLY_TRANSITION, sizeof(struct ui_sprite_spine_tri_apply_transition),
            module,
            ui_sprite_spine_tri_apply_transition_init,
            ui_sprite_spine_tri_apply_transition_fini,
            ui_sprite_spine_tri_apply_transition_copy,
            ui_sprite_spine_tri_apply_transition_exec);
        if (meta == NULL) {
            CPE_ERROR(
                module->m_em, "%s: spine-tri-have-entity register: meta create fail",
                ui_sprite_spine_module_name(module));
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_spine_tri_apply_transition_unregist(ui_sprite_spine_module_t module) {
    if (module->m_tri) {
        ui_sprite_tri_action_meta_t meta;

        meta = ui_sprite_tri_action_meta_find(module->m_tri, UI_SPRITE_SPINE_TRI_APPLY_TRANSITION);
        if (meta == NULL) {
            CPE_ERROR(
                module->m_em, "%s: spine-tri-have-entity unregister: meta not exist",
                ui_sprite_spine_module_name(module));
        }
        else {
            ui_sprite_tri_action_meta_free(meta);
        }
    }
}

const char * UI_SPRITE_SPINE_TRI_APPLY_TRANSITION = "spine-apply-transition";

#ifdef __cplusplus
}
#endif
