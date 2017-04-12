#include <assert.h>
#include "spine/Skeleton.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/spine/plugin_spine_obj_ik.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui/sprite_spine/ui_sprite_spine_utils.h"
#include "ui_sprite_spine_ik_restore_i.h"

ui_sprite_spine_ik_restore_t ui_sprite_spine_ik_restore_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SPINE_IK_RESTORE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_spine_ik_restore_free(ui_sprite_spine_ik_restore_t ik_restore) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(ik_restore);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_spine_ik_restore_set_obj_name(ui_sprite_spine_ik_restore_t ik_restore, const char * obj_name) {
    assert(obj_name);

    if (ik_restore->m_cfg_obj_name) {
        mem_free(ik_restore->m_module->m_alloc, ik_restore->m_cfg_obj_name);
        ik_restore->m_cfg_obj_name = NULL;
    }

    ik_restore->m_cfg_obj_name = cpe_str_mem_dup(ik_restore->m_module->m_alloc, obj_name);
    
    return 0;
}

int ui_sprite_spine_ik_restore_set_prefix(ui_sprite_spine_ik_restore_t ik_restore, const char * prefix) {
    assert(prefix);

    if (ik_restore->m_cfg_prefix) {
        mem_free(ik_restore->m_module->m_alloc, ik_restore->m_cfg_prefix);
        ik_restore->m_cfg_prefix = NULL;
    }

    ik_restore->m_cfg_prefix = cpe_str_mem_dup(ik_restore->m_module->m_alloc, prefix);
     
    return 0;
}

static int ui_sprite_spine_ik_restore_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_spine_ik_restore_t ik_restore = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_render_anim_t render_anim;
    ui_sprite_render_sch_t render_sch;
    ui_runtime_render_obj_ref_t render_obj_ref;
    ui_runtime_render_obj_t render_obj;
    plugin_spine_obj_t spine_obj;
    struct plugin_spine_obj_ik_it ik_it;
    plugin_spine_obj_ik_t ik;
        
    render_sch = ui_sprite_render_sch_find(entity);
    if (render_sch == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ik restore: entity no anim sch!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    render_anim = ui_sprite_render_anim_find_by_name(render_sch, ik_restore->m_cfg_obj_name);
    if (render_anim == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ik restore: runing anim %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ik_restore->m_cfg_obj_name);
        return -1;
    }

    render_obj_ref = ui_sprite_render_anim_obj(render_anim);
    assert(render_obj_ref);
    
    render_obj = ui_runtime_render_obj_ref_obj(render_obj_ref);
    assert(render_obj);
    
    if (ui_runtime_render_obj_type_id(render_obj) != UI_OBJECT_TYPE_SKELETON) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ui_sprite_spine_find_obj: render obj %s is not spine obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ik_restore->m_cfg_obj_name);
        return -1;
    }
    
    spine_obj = ui_runtime_render_obj_data(render_obj);
    if (spine_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ik restore: spine-obj %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ik_restore->m_cfg_obj_name);
        return -1;
    }

    if (ik_restore->m_cfg_prefix == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): ik restore: prefix not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    plugin_spine_obj_iks(spine_obj, &ik_it);
    while((ik = plugin_spine_obj_ik_it_next(&ik_it))) {
        if (!cpe_str_start_with(plugin_spine_obj_ik_name(ik), ik_restore->m_cfg_prefix)) continue;
        plugin_spine_obj_ik_restore(ik);
    }

    ik_restore->m_spine_obj = spine_obj;

    if (ui_sprite_fsm_action_life_circle(fsm_action) == ui_sprite_fsm_action_life_circle_working) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }
    
    return 0;
}

static void ui_sprite_spine_ik_restore_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_spine_ik_restore_t ik_restore = ui_sprite_fsm_action_data(fsm_action);
    struct plugin_spine_obj_ik_it ik_it;
    plugin_spine_obj_ik_t ik;
    uint16_t processing_count = 0;
    
    plugin_spine_obj_iks(ik_restore->m_spine_obj, &ik_it);
    while((ik = plugin_spine_obj_ik_it_next(&ik_it))) {
        if (!cpe_str_start_with(plugin_spine_obj_ik_name(ik), ik_restore->m_cfg_prefix)) continue;
        if (plugin_spine_obj_ik_is_runing(ik)) processing_count++;
    }

    if (processing_count == 0) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static void ui_sprite_spine_ik_restore_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_ik_restore_t ik_restore = ui_sprite_fsm_action_data(fsm_action);
    ik_restore->m_spine_obj = NULL;
}

static int ui_sprite_spine_ik_restore_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_ik_restore_t ik_restore = ui_sprite_fsm_action_data(fsm_action);
    ik_restore->m_module = ctx;
    ik_restore->m_cfg_obj_name = NULL;
    ik_restore->m_cfg_prefix = NULL;
    ik_restore->m_spine_obj = NULL;
    return 0;
}

static void ui_sprite_spine_ik_restore_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t modue = ctx;
    ui_sprite_spine_ik_restore_t ik_restore = ui_sprite_fsm_action_data(fsm_action);

    assert(ik_restore->m_spine_obj == NULL);

    if (ik_restore->m_cfg_obj_name) {
        mem_free(modue->m_alloc, ik_restore->m_cfg_obj_name);
        ik_restore->m_cfg_obj_name = NULL;
    }

    if (ik_restore->m_cfg_prefix) {
        mem_free(modue->m_alloc, ik_restore->m_cfg_prefix);
        ik_restore->m_cfg_prefix = NULL;
    }
}

static int ui_sprite_spine_ik_restore_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_spine_module_t modue = ctx;    
	ui_sprite_spine_ik_restore_t to_ik_restore = ui_sprite_fsm_action_data(to);
	ui_sprite_spine_ik_restore_t from_ik_restore = ui_sprite_fsm_action_data(from);

	if (ui_sprite_spine_ik_restore_init(to, ctx)) return -1;

    if (from_ik_restore->m_cfg_obj_name) {
        to_ik_restore->m_cfg_obj_name = cpe_str_mem_dup(modue->m_alloc, from_ik_restore->m_cfg_obj_name);
    }

    if (from_ik_restore->m_cfg_prefix) {
        to_ik_restore->m_cfg_prefix = cpe_str_mem_dup(modue->m_alloc, from_ik_restore->m_cfg_prefix);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_spine_ik_restore_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_ik_restore_t ik_restore = ui_sprite_spine_ik_restore_create(fsm_state, name);
    const char * str_value;
    
    if (ik_restore == NULL) {
        CPE_ERROR(module->m_em, "%s: create ik_restore action: create fail!", ui_sprite_spine_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_spine_ik_restore_set_obj_name(ik_restore, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create ik_restore action: set obj name %s fail!",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_ik_restore_free(ik_restore);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create ik_restore action: anim-name not configured!", ui_sprite_spine_module_name(module));
        ui_sprite_spine_ik_restore_free(ik_restore);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "prefix", NULL))) {
        if (ui_sprite_spine_ik_restore_set_prefix(ik_restore, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create ik_restore action: set obj name %s fail!",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_ik_restore_free(ik_restore);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create ik_restore action: prefix not configured!", ui_sprite_spine_module_name(module));
        ui_sprite_spine_ik_restore_free(ik_restore);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(ik_restore);
}

int ui_sprite_spine_ik_restore_regist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SPINE_IK_RESTORE_NAME, sizeof(struct ui_sprite_spine_ik_restore));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ui enable emitter register: meta create fail",
            ui_sprite_spine_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_spine_ik_restore_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_spine_ik_restore_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_spine_ik_restore_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_spine_ik_restore_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_spine_ik_restore_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_spine_ik_restore_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SPINE_IK_RESTORE_NAME, ui_sprite_spine_ik_restore_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_spine_ik_restore_unregist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SPINE_IK_RESTORE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_spine_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_SPINE_IK_RESTORE_NAME);
}

const char * UI_SPRITE_SPINE_IK_RESTORE_NAME = "spine-ik-restore";
