#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/moving/plugin_moving_module.h"
#include "plugin/moving/plugin_moving_plan_node.h"
#include "plugin/moving/plugin_moving_plan_segment.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_moving_move_by_plan_i.h"
#include "ui_sprite_moving_env_i.h"
#include "ui_sprite_moving_obj_i.h"

ui_sprite_moving_move_by_plan_t ui_sprite_moving_move_by_plan_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_MOVING_MOVE_BY_PLAN_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_moving_move_by_plan_free(ui_sprite_moving_move_by_plan_t move_by_plan) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(move_by_plan);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_moving_move_by_plan_set_res(ui_sprite_moving_move_by_plan_t move_by_plan, const char * res) {
    ui_sprite_moving_module_t module = move_by_plan->m_module;
    
    if (move_by_plan->m_cfg_res) {
        mem_free(module->m_alloc, move_by_plan->m_cfg_res);
    }

    if (res) {
        move_by_plan->m_cfg_res = cpe_str_mem_dup_trim(module->m_alloc, res);
        if (move_by_plan->m_cfg_res == NULL) return -1;
    }
    else {
        move_by_plan->m_cfg_res = NULL;
    }
    
    return 0;
}

const char * ui_sprite_moving_move_by_plan_res(ui_sprite_moving_move_by_plan_t move_by_plan) {
    return move_by_plan->m_cfg_res;
}

int ui_sprite_moving_move_by_plan_set_node_name(ui_sprite_moving_move_by_plan_t move_by_plan, const char * node_name) {
    ui_sprite_moving_module_t module = move_by_plan->m_module;
    
    if (move_by_plan->m_cfg_node_name) {
        mem_free(module->m_alloc, move_by_plan->m_cfg_node_name);
    }

    if (node_name) {
        move_by_plan->m_cfg_node_name = cpe_str_mem_dup_trim(module->m_alloc, node_name);
        if (move_by_plan->m_cfg_node_name == NULL) return -1;
    }
    else {
        move_by_plan->m_cfg_node_name = NULL;
    }
    
    return 0;
}

const char * ui_sprite_moving_move_by_plan_node_name(ui_sprite_moving_move_by_plan_t move_by_plan) {
    return move_by_plan->m_cfg_node_name;
}

int ui_sprite_moving_move_by_plan_set_move_policy(ui_sprite_moving_move_by_plan_t move_by_plan, const char * move_policy) {
    ui_sprite_moving_module_t module = move_by_plan->m_module;
    
    if (move_by_plan->m_cfg_move_policy) {
        mem_free(module->m_alloc, move_by_plan->m_cfg_move_policy);
    }

    if (move_policy) {
        move_by_plan->m_cfg_move_policy = cpe_str_mem_dup_trim(module->m_alloc, move_policy);
        if (move_by_plan->m_cfg_move_policy == NULL) return -1;
    }
    else {
        move_by_plan->m_cfg_move_policy = NULL;
    }
    
    return 0;
}

const char * ui_sprite_moving_move_by_plan_move_policy(ui_sprite_moving_move_by_plan_t move_by_plan) {
    return move_by_plan->m_cfg_move_policy;
}

int ui_sprite_moving_move_by_plan_set_loop_count(ui_sprite_moving_move_by_plan_t move_by_plan, const char * loop_count) {
    ui_sprite_moving_module_t module = move_by_plan->m_module;
    
    if (move_by_plan->m_cfg_loop_count) {
        mem_free(module->m_alloc, move_by_plan->m_cfg_loop_count);
    }

    if (loop_count) {
        move_by_plan->m_cfg_loop_count = cpe_str_mem_dup_trim(module->m_alloc, loop_count);
        if (move_by_plan->m_cfg_loop_count == NULL) return -1;
    }
    else {
        move_by_plan->m_cfg_loop_count = NULL;
    }
    
    return 0;
}

const char * ui_sprite_moving_move_by_plan_loop_count(ui_sprite_moving_move_by_plan_t move_by_plan) {
    return move_by_plan->m_cfg_loop_count;
}

static void ui_sprite_moving_move_by_plan_on_destory(void * ctx, plugin_moving_node_t moving_node) {
    ui_sprite_fsm_action_stop_update(ui_sprite_fsm_action_from_data(ctx));
}

static int ui_sprite_moving_move_by_plan_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_moving_module_t module = ctx;
    ui_sprite_moving_move_by_plan_t move_by_plan = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_moving_env_t env;
    ui_sprite_moving_obj_t moving_obj = NULL;
    plugin_moving_plan_t plan;
    plugin_moving_plan_node_t plan_node;
    ui_data_src_t plan_src;
    plugin_moving_node_t moving_node;
    ui_vector_2 base_pos;
    ui_sprite_2d_transform_t transform;
    ui_vector_2 begin_pos;
    const char * res;
    const char * move_policy;
    uint32_t loop_count;

    moving_obj = ui_sprite_moving_obj_find(entity);
    if (moving_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-plan: no moving obj",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }
    
    res = ui_sprite_fsm_action_check_calc_str(&module->m_tmp_buffer, move_by_plan->m_cfg_res, fsm_action, NULL, module->m_em);
    if (res == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-plan: res calc from %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_by_plan->m_cfg_res);
        goto ENTER_FAIL;
    }

    plan_src =
        ui_data_src_child_find_by_path(
            ui_data_mgr_src_root(plugin_moving_module_data_mgr(module->m_moving_module)),
            res, ui_data_src_type_moving_plan);
    if (plan_src == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-plan: res %s not exist",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), res);
        goto ENTER_FAIL;
    }

    plan = ui_data_src_product(plan_src);
    if (plan == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-plan: res not %s loaded",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), res);
        goto ENTER_FAIL;
    }

    move_by_plan->m_node_name = ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, move_by_plan->m_cfg_node_name, fsm_action, NULL, module->m_em);
    if (move_by_plan->m_node_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-plan: calc node name from %s fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_by_plan->m_cfg_node_name);
        goto ENTER_FAIL;
    }
        
    plan_node = plugin_moving_plan_node_find_by_name(plan, move_by_plan->m_node_name);
    if (plan_node == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-plan: node not exist",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }
    
    env = ui_sprite_moving_env_find(world);
    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-plan: moving env not exist",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-plan: entity no transform",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }
    begin_pos = ui_sprite_2d_transform_origin_pos(transform);

    assert(move_by_plan->m_control == NULL);
    move_by_plan->m_control = plugin_moving_control_create(env->m_env, plan);
    if (move_by_plan->m_control == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-plan: create control fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    base_pos.x = begin_pos.x;
    base_pos.y = begin_pos.y;

    move_policy = NULL;
    if (move_by_plan->m_cfg_move_policy) {
        move_policy = ui_sprite_fsm_action_check_calc_str(
            &module->m_tmp_buffer, move_by_plan->m_cfg_move_policy, fsm_action, NULL, module->m_em);
        if (move_policy == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): move-by-plan: move policy calc from %s fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_by_plan->m_cfg_move_policy);
            goto ENTER_FAIL;
        }
    }

    if (move_policy == NULL || strcmp(move_policy, "begin-at-cur-pos") == 0) {
        if (plugin_moving_control_adj_origin_pos_for_node_begin_at(move_by_plan->m_control, plan_node, &base_pos) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): move-by-plan: set control base pos begin at fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            goto ENTER_FAIL;
        }
    }
    else if (strcmp(move_policy, "end-at-cur-pos") == 0) {
        if (plugin_moving_control_adj_origin_pos_for_node_end_at(move_by_plan->m_control, plan_node, &base_pos) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): move-by-plan: set control base pos end at fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            goto ENTER_FAIL;
        }
    }
    else if (strcmp(move_policy, "no-adj") == 0) {
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-plan: unknown moving policy %s!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_policy);
        goto ENTER_FAIL;
    }

    if (move_by_plan->m_cfg_loop_count) {
        if (ui_sprite_fsm_action_check_calc_uint32(&loop_count, move_by_plan->m_cfg_loop_count, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): move-by-plan: loop count calc from %s fail",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_by_plan->m_cfg_loop_count);
            goto ENTER_FAIL;
        }
    }
    else {
        loop_count = 1;
    }
    
    moving_node = plugin_moving_node_create(move_by_plan->m_control, plan_node, loop_count);
    if (moving_node == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-plan: create node fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    ui_sprite_moving_obj_push_node(moving_obj, moving_node, move_by_plan, ui_sprite_moving_move_by_plan_on_destory);

    return ui_sprite_fsm_action_start_update(fsm_action);

ENTER_FAIL:
    if (moving_obj) {
        ui_sprite_moving_obj_destory_node_by_ctx(moving_obj, move_by_plan);
        if (move_by_plan->m_control) {
            plugin_moving_control_free(move_by_plan->m_control);
            move_by_plan->m_control = NULL;
        }
    }

    if (move_by_plan->m_node_name) {
        mem_free(module->m_alloc, move_by_plan->m_node_name);
        move_by_plan->m_node_name = NULL;
    }

    return -1;
}

static void ui_sprite_moving_move_by_plan_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_moving_module_t module = ctx;
    ui_sprite_moving_move_by_plan_t move_by_plan = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_moving_obj_t moving_obj;

    moving_obj = ui_sprite_moving_obj_find(entity);
    if (moving_obj) {
        ui_sprite_moving_obj_destory_node_by_ctx(moving_obj, move_by_plan);
    }
    
    assert(move_by_plan->m_control);
    plugin_moving_control_free(move_by_plan->m_control);
    move_by_plan->m_control = NULL;
    
    assert(move_by_plan->m_node_name);
    mem_free(module->m_alloc, move_by_plan->m_node_name);
    move_by_plan->m_node_name = NULL;
}

static void ui_sprite_moving_move_by_plan_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_moving_module_t module = ctx;
    ui_sprite_moving_move_by_plan_t move_by_plan = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    assert(move_by_plan->m_control);
    if (plugin_moving_control_update(move_by_plan->m_control, delta_s) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-by-plan: update fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }
}

static int ui_sprite_moving_move_by_plan_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_moving_move_by_plan_t move_by_plan = ui_sprite_fsm_action_data(fsm_action);
    move_by_plan->m_module = ctx;
    move_by_plan->m_cfg_res = NULL;
	move_by_plan->m_cfg_node_name = NULL;
    move_by_plan->m_cfg_move_policy = NULL;
    move_by_plan->m_cfg_loop_count = NULL;

    move_by_plan->m_node_name = NULL;
    move_by_plan->m_move_policy = ui_sprite_moving_policy_begin_at_cur_pos;
    move_by_plan->m_loop_count = 0;
    move_by_plan->m_control = NULL;
    return 0;
}

static void ui_sprite_moving_move_by_plan_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_moving_module_t module = ctx;
    ui_sprite_moving_move_by_plan_t move_by_plan = ui_sprite_fsm_action_data(fsm_action);
    
    assert(move_by_plan->m_control == NULL);
    assert(move_by_plan->m_node_name == NULL);

    if (move_by_plan->m_cfg_res) {
        mem_free(module->m_alloc, move_by_plan->m_cfg_res);
        move_by_plan->m_cfg_res = NULL;
    }

    if (move_by_plan->m_cfg_node_name) {
        mem_free(module->m_alloc, move_by_plan->m_cfg_node_name);
        move_by_plan->m_cfg_node_name = NULL;
    }

    if (move_by_plan->m_cfg_move_policy) {
        mem_free(module->m_alloc, move_by_plan->m_cfg_move_policy);
        move_by_plan->m_cfg_move_policy = NULL;
    }

    if (move_by_plan->m_cfg_loop_count) {
        mem_free(module->m_alloc, move_by_plan->m_cfg_loop_count);
        move_by_plan->m_cfg_loop_count = NULL;
    }
}

static int ui_sprite_moving_move_by_plan_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_moving_module_t module = ctx;
	ui_sprite_moving_move_by_plan_t to_move_by_plan = ui_sprite_fsm_action_data(to);
	ui_sprite_moving_move_by_plan_t from_move_by_plan = ui_sprite_fsm_action_data(from);

	if (ui_sprite_moving_move_by_plan_init(to, ctx)) return -1;

    if (from_move_by_plan->m_cfg_res) to_move_by_plan->m_cfg_res = cpe_str_mem_dup(module->m_alloc, from_move_by_plan->m_cfg_res);
    if (from_move_by_plan->m_cfg_node_name) to_move_by_plan->m_cfg_node_name = cpe_str_mem_dup(module->m_alloc, from_move_by_plan->m_cfg_node_name);
    if (from_move_by_plan->m_cfg_move_policy) to_move_by_plan->m_cfg_move_policy = cpe_str_mem_dup(module->m_alloc, from_move_by_plan->m_cfg_move_policy);
    if (from_move_by_plan->m_cfg_loop_count) to_move_by_plan->m_cfg_loop_count = cpe_str_mem_dup(module->m_alloc, from_move_by_plan->m_cfg_loop_count);

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_moving_move_by_plan_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_moving_module_t module = ctx;
    ui_sprite_moving_move_by_plan_t move_by_plan = ui_sprite_moving_move_by_plan_create(fsm_state, name);
    const char * str_value;
    
    if (move_by_plan == NULL) {
        CPE_ERROR(module->m_em, "%s: create move_by_plan action: create fail!", ui_sprite_moving_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "res", NULL))) {
        if (ui_sprite_moving_move_by_plan_set_res(move_by_plan, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create move_by_plan action: set res fail!", ui_sprite_moving_module_name(module));
            ui_sprite_moving_move_by_plan_free(move_by_plan);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create move_by_plan action: res not configured!", ui_sprite_moving_module_name(module));
        ui_sprite_moving_move_by_plan_free(move_by_plan);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "node-name", NULL))) {
        if (ui_sprite_moving_move_by_plan_set_node_name(move_by_plan, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create move_by_plan action: set node name %s fail!", ui_sprite_moving_module_name(module), str_value);
            ui_sprite_moving_move_by_plan_free(move_by_plan);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create move_by_plan action: node-name not configured!", ui_sprite_moving_module_name(module));
        ui_sprite_moving_move_by_plan_free(move_by_plan);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "loop-count", NULL))) {
        if (ui_sprite_moving_move_by_plan_set_loop_count(move_by_plan, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create move_by_plan action: set loop count %s!", ui_sprite_moving_module_name(module), str_value);
            ui_sprite_moving_move_by_plan_free(move_by_plan);
            return NULL;
        }
    }
        
    if ((str_value = cfg_get_string(cfg, "move-policy", NULL))) {
        if (ui_sprite_moving_move_by_plan_set_move_policy(move_by_plan, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create move_by_plan action: set move policy %s!", ui_sprite_moving_module_name(module), str_value);
            ui_sprite_moving_move_by_plan_free(move_by_plan);
            return NULL;
        }
    }

    return ui_sprite_fsm_action_from_data(move_by_plan);
}

int ui_sprite_moving_move_by_plan_regist(ui_sprite_moving_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_MOVING_MOVE_BY_PLAN_NAME, sizeof(struct ui_sprite_moving_move_by_plan));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: moving enable emitter register: meta create fail",
            ui_sprite_moving_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_moving_move_by_plan_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_moving_move_by_plan_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_moving_move_by_plan_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_moving_move_by_plan_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_moving_move_by_plan_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_moving_move_by_plan_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_MOVING_MOVE_BY_PLAN_NAME, ui_sprite_moving_move_by_plan_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_moving_move_by_plan_unregist(ui_sprite_moving_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_MOVING_MOVE_BY_PLAN_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_moving_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_MOVING_MOVE_BY_PLAN_NAME);
}

const char * UI_SPRITE_MOVING_MOVE_BY_PLAN_NAME = "move-by-plan";
