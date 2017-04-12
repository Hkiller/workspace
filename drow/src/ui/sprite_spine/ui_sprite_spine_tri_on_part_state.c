#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "plugin/spine/plugin_spine_obj.h"
#include "plugin/spine/plugin_spine_obj_part.h"
#include "plugin/spine/plugin_spine_obj_part_state.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite_tri/ui_sprite_tri_condition.h"
#include "ui/sprite_tri/ui_sprite_tri_condition_meta.h"
#include "ui_sprite_spine_tri_on_part_state_i.h"
#include "ui_sprite_spine_utils_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_spine_tri_on_part_state_t
ui_sprite_spine_tri_on_part_state_create(ui_sprite_tri_rule_t rule) {
    ui_sprite_tri_condition_t condition;
    
    condition = ui_sprite_tri_condition_create(rule, UI_SPRITE_SPINE_TRI_ON_PART_STATE);
    if (condition == NULL) return NULL;

    return (ui_sprite_spine_tri_on_part_state_t)ui_sprite_tri_condition_data(condition);
}
    
void ui_sprite_spine_tri_on_part_state_free(ui_sprite_spine_tri_on_part_state_t on_part_state) {
    ui_sprite_tri_condition_t condition;

    condition = ui_sprite_tri_condition_from_data(on_part_state);

    ui_sprite_tri_condition_free(condition);
}

plugin_spine_obj_t ui_sprite_spine_tri_on_part_state_obj(ui_sprite_spine_tri_on_part_state_t on_part_state) {
    return on_part_state->m_obj;
}
    
void ui_sprite_spine_tri_on_part_state_set_obj(ui_sprite_spine_tri_on_part_state_t on_part_state, plugin_spine_obj_t obj) {
    on_part_state->m_obj = obj;
}

const char * ui_sprite_spine_tri_on_part_state_part_state(ui_sprite_spine_tri_on_part_state_t on_part_state) {
    return on_part_state->m_part_state;
}
    
int ui_sprite_spine_tri_on_part_state_set_part_state(ui_sprite_spine_tri_on_part_state_t on_part_state, const char * part_state) {
    ui_sprite_spine_module_t module = on_part_state->m_module;
    char * new_part_state = NULL;

    if (part_state) {
        new_part_state = cpe_str_mem_dup(module->m_alloc, part_state);
        if (new_part_state == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_spine_tri_on_part_state_init: dup require count %s fail", part_state);
            return -1;
        }
    }

    if (on_part_state->m_part_state) mem_free(module->m_alloc, on_part_state->m_part_state);

    on_part_state->m_part_state = new_part_state;

    return 0;
}

const char * ui_sprite_spine_tri_on_part_state_part_name(ui_sprite_spine_tri_on_part_state_t on_part_state) {
    return on_part_state->m_part_name;
}
    
int ui_sprite_spine_tri_on_part_state_set_part_name(ui_sprite_spine_tri_on_part_state_t on_part_state, const char * part_name) {
    ui_sprite_spine_module_t module = on_part_state->m_module;
    char * new_part_name = NULL;

    if (part_name) {
        new_part_name = cpe_str_mem_dup(module->m_alloc, part_name);
        if (new_part_name == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_spine_tri_on_part_state_init: dup require count %s fail", part_name);
            return -1;
        }
    }

    if (on_part_state->m_part_name) mem_free(module->m_alloc, on_part_state->m_part_name);

    on_part_state->m_part_name = new_part_name;

    return 0;
}

uint8_t ui_sprite_spine_tri_on_part_state_include_transition(ui_sprite_spine_tri_on_part_state_t on_part_state) {
    return on_part_state->m_include_transition;
}
    
void ui_sprite_spine_tri_on_part_state_set_include_transition(ui_sprite_spine_tri_on_part_state_t on_part_state, uint8_t i) {
    on_part_state->m_include_transition = i;
}

static int ui_sprite_spine_tri_on_part_state_init(void * ctx, ui_sprite_tri_condition_t condition) {
    ui_sprite_spine_tri_on_part_state_t data = (ui_sprite_spine_tri_on_part_state_t)ui_sprite_tri_condition_data(condition);
    ui_sprite_spine_module_t module = (ui_sprite_spine_module_t)ctx;
    
    data->m_module = module;
    data->m_obj = NULL;
    data->m_part_name = NULL;
    data->m_part_state = NULL;
    data->m_include_transition = 0;
    
    return 0;
}

static void ui_sprite_spine_tri_on_part_state_fini(void * ctx, ui_sprite_tri_condition_t condition) {
    ui_sprite_spine_module_t module = (ui_sprite_spine_module_t)ctx;
    ui_sprite_spine_tri_on_part_state_t on_part_state = (ui_sprite_spine_tri_on_part_state_t)ui_sprite_tri_condition_data(condition);
    
    if (on_part_state->m_part_state) {
        mem_free(module->m_alloc, on_part_state->m_part_state);
        on_part_state->m_part_state = NULL;
    }
}

static int ui_sprite_spine_tri_on_part_state_copy(void * ctx, ui_sprite_tri_condition_t condition, ui_sprite_tri_condition_t source) {
    ui_sprite_spine_tri_on_part_state_t to_data = (ui_sprite_spine_tri_on_part_state_t)ui_sprite_tri_condition_data(condition);
    ui_sprite_spine_tri_on_part_state_t from_data = (ui_sprite_spine_tri_on_part_state_t)ui_sprite_tri_condition_data(source);

    if (ui_sprite_spine_tri_on_part_state_init(ctx, condition) != 0) return -1;
    
    if (from_data->m_part_name) {
        if (ui_sprite_spine_tri_on_part_state_set_part_name(to_data, from_data->m_part_name) != 0) {
            ui_sprite_spine_tri_on_part_state_fini(ctx, condition);
            return -1;
        }
    }

    if (from_data->m_part_state) {
        if (ui_sprite_spine_tri_on_part_state_set_part_state(to_data, from_data->m_part_state) != 0) {
            ui_sprite_spine_tri_on_part_state_fini(ctx, condition);
            return -1;
        }
    }
    
    return 0;
}

static int ui_sprite_spine_tri_on_part_state_check(void * ctx, ui_sprite_tri_condition_t condition, uint8_t * r) {
    ui_sprite_spine_module_t module = (ui_sprite_spine_module_t)ctx;
    ui_sprite_spine_tri_on_part_state_t on_part_state = (ui_sprite_spine_tri_on_part_state_t)ui_sprite_tri_condition_data(condition);
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(on_part_state->m_obj));
    plugin_spine_obj_part_t part;
    plugin_spine_obj_part_state_t cur_state;
    
    if (on_part_state->m_part_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-on-part-state: part not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (on_part_state->m_part_state == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-on-part-state: part state not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    if (on_part_state->m_obj) {
        part = plugin_spine_obj_part_find(on_part_state->m_obj, on_part_state->m_part_name);
        if (part == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine-on-part-state: part %s not exist in obj!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), on_part_state->m_part_name);
            return -1;
        }
    }
    else {
        part = ui_sprite_spine_find_obj_part(module->m_sprite_render, entity, on_part_state->m_part_name, module->m_em);
        if (part == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine-on-part-state: part %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), on_part_state->m_part_name);
            return -1;
        }
    }

    cur_state = plugin_spine_obj_part_cur_state(part);

    if (cur_state == NULL || strcmp(plugin_spine_obj_part_state_name(cur_state), on_part_state->m_part_state) != 0) {
        *r = 0;
    }
    else {
        if (on_part_state->m_include_transition) {
            *r = 1;
        }
        else {
            if (plugin_spine_obj_part_is_in_enter(part)) {
                *r = 0;
            }
            else {
                *r = 1;
            }
        }
    }
    
    return 0;
}

int ui_sprite_spine_tri_on_part_state_regist(ui_sprite_spine_module_t module) {
    if (module->m_tri) {
        ui_sprite_tri_condition_meta_t meta;

        meta = ui_sprite_tri_condition_meta_create(
            module->m_tri, UI_SPRITE_SPINE_TRI_ON_PART_STATE, sizeof(struct ui_sprite_spine_tri_on_part_state),
            module,
            ui_sprite_spine_tri_on_part_state_init,
            ui_sprite_spine_tri_on_part_state_fini,
            ui_sprite_spine_tri_on_part_state_copy,
            ui_sprite_spine_tri_on_part_state_check);
        if (meta == NULL) {
            CPE_ERROR(
                module->m_em, "%s: spine-tri-on-part-state register: meta create fail",
                ui_sprite_spine_module_name(module));
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_spine_tri_on_part_state_unregist(ui_sprite_spine_module_t module) {
    if (module->m_tri) {
        ui_sprite_tri_condition_meta_t meta;

        meta = ui_sprite_tri_condition_meta_find(module->m_tri, UI_SPRITE_SPINE_TRI_ON_PART_STATE);
        if (meta == NULL) {
            CPE_ERROR(
                module->m_em, "%s: spine-tri-on-part-state unregister: meta not exist",
                ui_sprite_spine_module_name(module));
        }
        else {
            ui_sprite_tri_condition_meta_free(meta);
        }
    }
}

const char * UI_SPRITE_SPINE_TRI_ON_PART_STATE = "spine-on-part-state";

#ifdef __cplusplus
}
#endif
