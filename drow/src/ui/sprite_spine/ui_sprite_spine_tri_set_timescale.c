#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "plugin/spine/plugin_spine_obj.h"
#include "plugin/spine/plugin_spine_obj_part.h"
#include "plugin/spine/plugin_spine_obj_track.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_calc.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_tri/ui_sprite_tri_action.h"
#include "ui/sprite_tri/ui_sprite_tri_action_meta.h"
#include "ui_sprite_spine_tri_set_timescale_i.h"
#include "ui_sprite_spine_utils_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_spine_tri_set_timescale_t
ui_sprite_spine_tri_set_timescale_create(ui_sprite_tri_rule_t rule) {
    ui_sprite_tri_action_t action;
    
    action = ui_sprite_tri_action_create(rule, UI_SPRITE_SPINE_TRI_SET_TIMESCALE);
    if (action == NULL) return NULL;

    return (ui_sprite_spine_tri_set_timescale_t)ui_sprite_tri_action_data(action);
}
    
void ui_sprite_spine_tri_set_timescale_free(ui_sprite_spine_tri_set_timescale_t set_timescale) {
    ui_sprite_tri_action_t action;

    action = ui_sprite_tri_action_from_data(set_timescale);

    ui_sprite_tri_action_free(action);
}

plugin_spine_obj_t ui_sprite_spine_tri_set_timescale_obj(ui_sprite_spine_tri_set_timescale_t set_timescale) {
    return set_timescale->m_obj;
}

void ui_sprite_spine_tri_set_timescale_set_obj(ui_sprite_spine_tri_set_timescale_t set_timescale, plugin_spine_obj_t obj) {
    set_timescale->m_obj = obj;
}
    
const char * ui_sprite_spine_tri_set_timescale_part(ui_sprite_spine_tri_set_timescale_t set_timescale) {
    return set_timescale->m_part;
}
    
int ui_sprite_spine_tri_set_timescale_set_part(ui_sprite_spine_tri_set_timescale_t set_timescale, const char * part) {
    ui_sprite_spine_module_t module = set_timescale->m_module;
    char * new_part = NULL;

    if (part) {
        new_part = cpe_str_mem_dup(module->m_alloc, part);
        if (new_part == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_spine_tri_set_timescale_init: dup spine module %s fail", part);
            return -1;
        }
    }

    if (set_timescale->m_part) mem_free(module->m_alloc, set_timescale->m_part);

    set_timescale->m_part = new_part;

    return 0;
}

const char * ui_sprite_spine_tri_set_timescale_timescale(ui_sprite_spine_tri_set_timescale_t set_timescale) {
    return set_timescale->m_timescale;
}
    
int ui_sprite_spine_tri_set_timescale_set_timescale(ui_sprite_spine_tri_set_timescale_t set_timescale, const char * timescale) {
    ui_sprite_spine_module_t module = set_timescale->m_module;
    char * new_timescale = NULL;

    if (timescale) {
        new_timescale = cpe_str_mem_dup(module->m_alloc, timescale);
        if (new_timescale == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_spine_tri_set_timescale_init: dup require count %s fail", timescale);
            return -1;
        }
    }

    if (set_timescale->m_timescale) mem_free(module->m_alloc, set_timescale->m_timescale);

    set_timescale->m_timescale = new_timescale;

    return 0;
}

static int ui_sprite_spine_tri_set_timescale_init(void * ctx, ui_sprite_tri_action_t action) {
    ui_sprite_spine_tri_set_timescale_t data = (ui_sprite_spine_tri_set_timescale_t)ui_sprite_tri_action_data(action);
    ui_sprite_spine_module_t module = (ui_sprite_spine_module_t)ctx;
    
    data->m_module = module;
    data->m_obj = NULL;
    data->m_part = NULL;
    data->m_timescale = NULL;
    
    return 0;
}

static void ui_sprite_spine_tri_set_timescale_fini(void * ctx, ui_sprite_tri_action_t action) {
    ui_sprite_spine_module_t module = (ui_sprite_spine_module_t)ctx;
    ui_sprite_spine_tri_set_timescale_t set_timescale = (ui_sprite_spine_tri_set_timescale_t)ui_sprite_tri_action_data(action);
    
    if (set_timescale->m_part) {
        mem_free(module->m_alloc, set_timescale->m_part);
        set_timescale->m_part = NULL;
    }
    
    if (set_timescale->m_timescale == NULL) {
        mem_free(module->m_alloc, set_timescale->m_timescale);
        set_timescale->m_timescale = NULL;
    }
}

static int ui_sprite_spine_tri_set_timescale_copy(void * ctx, ui_sprite_tri_action_t action, ui_sprite_tri_action_t source) {
    ui_sprite_spine_module_t module = (ui_sprite_spine_module_t)ctx;
    ui_sprite_spine_tri_set_timescale_t to_data = (ui_sprite_spine_tri_set_timescale_t)ui_sprite_tri_action_data(action);
    ui_sprite_spine_tri_set_timescale_t from_data = (ui_sprite_spine_tri_set_timescale_t)ui_sprite_tri_action_data(source);

    if (ui_sprite_spine_tri_set_timescale_init(ctx, action) != 0) return -1;

    if (from_data->m_part) {
        to_data->m_part = cpe_str_mem_dup(module->m_alloc, from_data->m_part);
    }

    if (from_data->m_timescale) {
        to_data->m_timescale = cpe_str_mem_dup(module->m_alloc, from_data->m_timescale);
    }
    
    return 0;
}

static void ui_sprite_spine_tri_set_timescale_exec(void * ctx, ui_sprite_tri_action_t action) {
    ui_sprite_spine_module_t module = (ui_sprite_spine_module_t)ctx;
    ui_sprite_spine_tri_set_timescale_t set_timescale = (ui_sprite_spine_tri_set_timescale_t)ui_sprite_tri_action_data(action);
    ui_sprite_entity_t entity = ui_sprite_tri_action_entity(action);
    plugin_spine_obj_part_t part;
    float timescale;
    
    if (set_timescale->m_part == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-set-timescale: part not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (set_timescale->m_timescale == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-set-timescale: timescale not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (set_timescale->m_obj) {
        part = plugin_spine_obj_part_find(set_timescale->m_obj, set_timescale->m_part);
        if (part == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine-set-timescale: part %s not exist in obj!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), set_timescale->m_part);
            return;
        }
    }
    else {
        part = ui_sprite_spine_find_obj_part(module->m_sprite_render, entity, set_timescale->m_part, module->m_em);
        if (part == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine-set-timescale: part %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), set_timescale->m_part);
            return;
        }
    }

    if (ui_sprite_entity_check_calc_float(&timescale, set_timescale->m_timescale, entity, NULL, module->m_em) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-set-timescale: calc timescale from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), set_timescale->m_timescale);
        return;
    }

    plugin_spine_obj_track_set_time_scale(plugin_spine_obj_part_track(part), timescale);
}

int ui_sprite_spine_tri_set_timescale_regist(ui_sprite_spine_module_t module) {
    if (module->m_tri) {
        ui_sprite_tri_action_meta_t meta;

        meta = ui_sprite_tri_action_meta_create(
            module->m_tri, UI_SPRITE_SPINE_TRI_SET_TIMESCALE, sizeof(struct ui_sprite_spine_tri_set_timescale),
            module,
            ui_sprite_spine_tri_set_timescale_init,
            ui_sprite_spine_tri_set_timescale_fini,
            ui_sprite_spine_tri_set_timescale_copy,
            ui_sprite_spine_tri_set_timescale_exec);
        if (meta == NULL) {
            CPE_ERROR(
                module->m_em, "%s: spine-tri-have-entity register: meta create fail",
                ui_sprite_spine_module_name(module));
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_spine_tri_set_timescale_unregist(ui_sprite_spine_module_t module) {
    if (module->m_tri) {
        ui_sprite_tri_action_meta_t meta;

        meta = ui_sprite_tri_action_meta_find(module->m_tri, UI_SPRITE_SPINE_TRI_SET_TIMESCALE);
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

const char * UI_SPRITE_SPINE_TRI_SET_TIMESCALE = "spine-set-timescale";

#ifdef __cplusplus
}
#endif
