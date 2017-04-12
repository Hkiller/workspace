#include <assert.h>
#include "spine/Skeleton.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin/spine/plugin_spine_obj.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_spine_move_entity_i.h"
#include "plugin/spine/plugin_spine_utils.h"

ui_sprite_spine_move_entity_t ui_sprite_spine_move_entity_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SPINE_MOVE_ENTITY_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_spine_move_entity_free(ui_sprite_spine_move_entity_t move_entity) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(move_entity);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_spine_move_entity_set_res(ui_sprite_spine_move_entity_t move_entity, const char * res) {
    if (move_entity->m_cfg_skeleton_res) {
        mem_free(move_entity->m_module->m_alloc, move_entity->m_cfg_skeleton_res);
    }

    if (res) {
        move_entity->m_cfg_skeleton_res = cpe_str_mem_dup_trim(move_entity->m_module->m_alloc, res);
        if (move_entity->m_cfg_skeleton_res == NULL) {
            CPE_ERROR(move_entity->m_module->m_em, "move_entity: dup res %s fail!", res);
            return -1;
        }
    }
    else {
        move_entity->m_cfg_skeleton_res = NULL;
    }

    return 0;
}

const char * ui_sprite_spine_move_entity_res(ui_sprite_spine_move_entity_t move_entity) {
    return move_entity->m_cfg_skeleton_res;
}

int ui_sprite_spine_move_entity_set_bone(ui_sprite_spine_move_entity_t move_entity, const char * name) {
    if (move_entity->m_cfg_bone) {
        mem_free(move_entity->m_module->m_alloc, move_entity->m_cfg_bone);
    }

    if (name) {
        move_entity->m_cfg_bone = cpe_str_mem_dup_trim(move_entity->m_module->m_alloc, name);
        if (move_entity->m_cfg_bone == NULL) {
            CPE_ERROR(
                move_entity->m_module->m_em, "move_entity: dup name %s fail!", name);
            return -1;
        }
    }
    else {
        move_entity->m_cfg_bone = NULL;
    }

    return 0;
}

const char * ui_sprite_spine_move_entity_bone(ui_sprite_spine_move_entity_t move_entity) {
    return move_entity->m_cfg_bone;
}

static void ui_sprite_spine_move_entity_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s);
static void ui_sprite_spine_move_entity_on_event(void * ctx, ui_runtime_render_obj_t obj, const char * evt);

static int ui_sprite_spine_move_entity_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_move_entity_t move_entity = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform;
    plugin_spine_obj_t spine_obj;
    char * res = NULL;

    res = ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, move_entity->m_cfg_skeleton_res, fsm_action, NULL, module->m_em);
    if (res == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-skeleton %s[%s]: res calc fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_entity->m_cfg_skeleton_res, move_entity->m_cfg_bone);
        return -1;
    }

    assert(move_entity->m_render_obj == NULL);
    move_entity->m_render_obj = ui_runtime_render_obj_create_by_res(module->m_runtime, res, NULL);
    if (move_entity->m_render_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine-move-entity: create render obj from res %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), res);
        goto ENTER_FAIL;
    }
    ui_runtime_render_obj_set_evt_processor(move_entity->m_render_obj, ui_sprite_spine_move_entity_on_event, move_entity);

    spine_obj = (plugin_spine_obj_t)ui_runtime_render_obj_data(move_entity->m_render_obj);
    
    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-skeleton]: entity no transform",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }
    move_entity->m_base_pos = ui_sprite_2d_transform_origin_pos(transform);

    move_entity->m_bone = spSkeleton_findBone(plugin_spine_obj_skeleton(spine_obj), move_entity->m_cfg_bone);
    if (move_entity->m_bone == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-skeleton: res %s no bone %s",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_entity->m_cfg_skeleton_res, move_entity->m_cfg_bone);
        goto ENTER_FAIL;
    }
    
    ui_sprite_fsm_action_start_update(fsm_action);

    assert(res);
    mem_free(module->m_alloc, res);

    ui_sprite_spine_move_entity_update(fsm_action, ctx, 0.0f);

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): move-by-skeleton: res=%s bone=%s, base=(%f,%f)",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            move_entity->m_cfg_skeleton_res, move_entity->m_cfg_bone,
            move_entity->m_base_pos.x, move_entity->m_base_pos.y);
    }
    
    return 0;

ENTER_FAIL:
    if (move_entity->m_render_obj) {
        ui_runtime_render_obj_free(move_entity->m_render_obj);
        move_entity->m_render_obj = NULL;
    }

    move_entity->m_bone = NULL;
    
    if (res) {
        mem_free(module->m_alloc, res);
        res = NULL;
    }
    
    return -1;
}

static void ui_sprite_spine_move_entity_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_move_entity_t move_entity = ui_sprite_fsm_action_data(fsm_action);

    assert(move_entity->m_render_obj);
    ui_runtime_render_obj_free(move_entity->m_render_obj);
    move_entity->m_render_obj = NULL;

    move_entity->m_bone = NULL;
}

static void ui_sprite_spine_move_entity_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_spine_module_t module = ctx;
	ui_sprite_spine_move_entity_t move_entity = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t transform;
    ui_transform trans;
    ui_vector_2 pos;
    
    assert(move_entity->m_bone);
    assert(move_entity->m_render_obj);

    ui_runtime_render_obj_update(move_entity->m_render_obj, delta_s);

    if (plugin_spine_bone_calc_transform(move_entity->m_bone, &trans) != 0) return;

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-skeleton: entity no transform",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    ui_transform_get_pos_2(&trans, &pos);

    pos.x += move_entity->m_base_pos.x;
    pos.y += move_entity->m_base_pos.y;

    ui_transform_set_pos_2(&trans, &pos);

    ui_sprite_2d_transform_set_trans(transform, &trans);

    if (!ui_runtime_render_obj_is_playing(move_entity->m_render_obj)) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static int ui_sprite_spine_move_entity_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_move_entity_t move_entity = ui_sprite_fsm_action_data(fsm_action);
    move_entity->m_module = ctx;
	move_entity->m_cfg_skeleton_res = NULL;
    move_entity->m_cfg_bone = NULL;
    move_entity->m_render_obj = NULL;
    move_entity->m_bone = NULL;
    return 0;
}

static void ui_sprite_spine_move_entity_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_move_entity_t move_entity = ui_sprite_fsm_action_data(fsm_action);
    
    assert(move_entity->m_render_obj == NULL);
    assert(move_entity->m_bone == NULL);
    
    if (move_entity->m_cfg_skeleton_res) {
        mem_free(module->m_alloc, move_entity->m_cfg_skeleton_res);
        move_entity->m_cfg_skeleton_res = NULL;
    }

    if (move_entity->m_cfg_bone) {
        mem_free(module->m_alloc, move_entity->m_cfg_bone);
        move_entity->m_cfg_bone = NULL;
    }
}

static int ui_sprite_spine_move_entity_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
	ui_sprite_spine_move_entity_t to_move_entity = ui_sprite_fsm_action_data(to);
	ui_sprite_spine_move_entity_t from_move_entity = ui_sprite_fsm_action_data(from);

	if (ui_sprite_spine_move_entity_init(to, ctx)) return -1;

    if (from_move_entity->m_cfg_skeleton_res) {
        to_move_entity->m_cfg_skeleton_res = cpe_str_mem_dup(module->m_alloc, from_move_entity->m_cfg_skeleton_res);
    }

    if (from_move_entity->m_cfg_bone) {
        to_move_entity->m_cfg_bone = cpe_str_mem_dup(module->m_alloc, from_move_entity->m_cfg_bone);
    }

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_spine_move_entity_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_move_entity_t move_entity = ui_sprite_spine_move_entity_create(fsm_state, name);
    const char * str_value;
    
    if (move_entity == NULL) {
        CPE_ERROR(module->m_em, "%s: create spine-move-entity action: create fail!", ui_sprite_spine_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "res", NULL)) == NULL) {
        CPE_ERROR(module->m_em, "%s: create spine-move-entity action: res not configured!", ui_sprite_spine_module_name(module));
        goto LOADER_FAIL;
    }
    else if (ui_sprite_spine_move_entity_set_res(move_entity, str_value) != 0) {
        CPE_ERROR(module->m_em, "%s: create spine-move-entity action: set res %s fail!", ui_sprite_spine_module_name(module), str_value);
        goto LOADER_FAIL;
    }

    if ((str_value = cfg_get_string(cfg, "bone", NULL)) == NULL) {
        CPE_ERROR(module->m_em, "%s: create spine-move-entity action: bone not configured!", ui_sprite_spine_module_name(module));
        goto LOADER_FAIL;
    }
    else if (ui_sprite_spine_move_entity_set_bone(move_entity, str_value) != 0) {
        CPE_ERROR(module->m_em, "%s: create spine-move-entity action: set skeleton name %s!", ui_sprite_spine_module_name(module), str_value);
        goto LOADER_FAIL;
    }

    return ui_sprite_fsm_action_from_data(move_entity);

LOADER_FAIL:
    ui_sprite_spine_move_entity_free(move_entity);
    return NULL;
}

static void ui_sprite_spine_move_entity_on_event(void * ctx, ui_runtime_render_obj_t obj, const char * evt) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(ctx);
    ui_sprite_fsm_action_build_and_send_event(fsm_action, evt, NULL);
}

int ui_sprite_spine_move_entity_regist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SPINE_MOVE_ENTITY_NAME, sizeof(struct ui_sprite_spine_move_entity));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: spine enable emitter register: meta create fail",
            ui_sprite_spine_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_spine_move_entity_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_spine_move_entity_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_spine_move_entity_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_spine_move_entity_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_spine_move_entity_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_spine_move_entity_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SPINE_MOVE_ENTITY_NAME, ui_sprite_spine_move_entity_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_spine_move_entity_unregist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SPINE_MOVE_ENTITY_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_spine_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_SPINE_MOVE_ENTITY_NAME);
}

const char * UI_SPRITE_SPINE_MOVE_ENTITY_NAME = "spine-move-entity";
