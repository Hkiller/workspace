#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_vector_2.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/moving/plugin_moving_module.h"
#include "plugin/moving/plugin_moving_plan_node.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_moving_move_set_to_begin_i.h"

ui_sprite_moving_move_set_to_begin_t ui_sprite_moving_move_set_to_begin_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_MOVING_MOVE_SET_TO_BEGIN_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_moving_move_set_to_begin_free(ui_sprite_moving_move_set_to_begin_t move_set_to_begin) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(move_set_to_begin);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_moving_move_set_to_begin_set_res(ui_sprite_moving_move_set_to_begin_t move_set_to_begin, const char * res) {
    cpe_str_dup(move_set_to_begin->m_plan_res, sizeof(move_set_to_begin->m_plan_res), res);
    * cpe_str_trim_tail(move_set_to_begin->m_plan_res + strlen(move_set_to_begin->m_plan_res), move_set_to_begin->m_plan_res) = 0;
    return 0;
}

const char * ui_sprite_moving_move_set_to_begin_res(ui_sprite_moving_move_set_to_begin_t move_set_to_begin) {
    return move_set_to_begin->m_plan_res;
}

int ui_sprite_moving_move_set_to_begin_set_node_name(ui_sprite_moving_move_set_to_begin_t move_set_to_begin, const char * name) {
    cpe_str_dup(move_set_to_begin->m_node_name, sizeof(move_set_to_begin->m_node_name), name);
    return 0;
}

const char * ui_sprite_moving_move_set_to_begin_node_name(ui_sprite_moving_move_set_to_begin_t move_set_to_begin) {
    return move_set_to_begin->m_node_name;
}

ui_sprite_moving_policy_t ui_sprite_moving_move_set_to_begin_move_policy(ui_sprite_moving_move_set_to_begin_t move_set_to_begin) {
    return move_set_to_begin->m_move_policy;
}

void ui_sprite_moving_move_set_to_begin_set_move_policy(ui_sprite_moving_move_set_to_begin_t move_set_to_begin, ui_sprite_moving_policy_t policy) {
    move_set_to_begin->m_move_policy = policy;
}

static int ui_sprite_moving_move_set_to_begin_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_moving_module_t module = ctx;
    ui_sprite_moving_move_set_to_begin_t move_set_to_begin = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    plugin_moving_plan_t plan;
    plugin_moving_plan_node_t plan_node;
    ui_data_src_t plan_src;
    ui_sprite_2d_transform_t transform;
    ui_vector_2 entity_pos;
    char * res;
    
    res = ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, move_set_to_begin->m_plan_res, fsm_action, NULL, module->m_em);
    if (res == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-set-to-begin %s[%s]: res calc fail",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_set_to_begin->m_plan_res, move_set_to_begin->m_node_name);
        return -1;
    }

    plan_src =
        ui_data_src_child_find_by_path(
            ui_data_mgr_src_root(plugin_moving_module_data_mgr(module->m_moving_module)),
            res, ui_data_src_type_moving_plan);
    if (plan_src == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-set-to-begin %s[%s]: res %s not exist",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_set_to_begin->m_plan_res, move_set_to_begin->m_node_name, res);
        mem_free(module->m_alloc, res);
        return -1;
    }
    mem_free(module->m_alloc, res);
    res = NULL;

    plan = ui_data_src_product(plan_src);
    if (plan == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-set-to-begin %s[%s]: res not loaded",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_set_to_begin->m_plan_res, move_set_to_begin->m_node_name);
        return -1;
    }

    plan_node = plugin_moving_plan_node_find_by_name(plan, move_set_to_begin->m_node_name);
    if (plan_node == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-set-to-begin %s[%s]: node not exist",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_set_to_begin->m_plan_res, move_set_to_begin->m_node_name);
        return -1;
    }
    
    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-set-to-begin %s[%s]: entity no transform",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_set_to_begin->m_plan_res, move_set_to_begin->m_node_name);
        return -1;
    }

    switch(move_set_to_begin->m_move_policy) {
    case ui_sprite_moving_policy_end_at_cur_pos: {
        ui_vector_2 begin_pos;
        ui_vector_2 end_pos;
        
        if (plugin_moving_plan_node_begin_pos(plan_node, &begin_pos) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): move-set-to-begin %s[%s]: get begin pos fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                move_set_to_begin->m_plan_res, move_set_to_begin->m_node_name);
            return -1;
        }

        if (plugin_moving_plan_node_end_pos(plan_node, &end_pos) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): move-set-to-begin %s[%s]: get end pos fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                move_set_to_begin->m_plan_res, move_set_to_begin->m_node_name);
            return -1;
        }

        entity_pos = ui_sprite_2d_transform_origin_pos(transform);
        entity_pos.x += (end_pos.x - begin_pos.x);
        entity_pos.y += (end_pos.y - begin_pos.y);
        break;
    }
    case ui_sprite_moving_policy_no_adj: {
        ui_vector_2 begin_pos;
        if (plugin_moving_plan_node_begin_pos(plan_node, &begin_pos) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): move-set-to-begin %s[%s]: get begin pos fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                move_set_to_begin->m_plan_res, move_set_to_begin->m_node_name);
            return -1;
        }

        entity_pos.x = begin_pos.x;
        entity_pos.y = begin_pos.y;
        break;
    }
    default:
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-set-to-begin %s[%s]: unknown moving policy %d!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            move_set_to_begin->m_plan_res, move_set_to_begin->m_node_name,
            move_set_to_begin->m_move_policy);
        return -1;
    }

    ui_sprite_2d_transform_set_origin_pos(transform, entity_pos);

    return 0;
}

static void ui_sprite_moving_move_set_to_begin_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_moving_move_set_to_begin_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_moving_move_set_to_begin_t move_set_to_begin = ui_sprite_fsm_action_data(fsm_action);
    move_set_to_begin->m_module = ctx;
	move_set_to_begin->m_plan_res[0] = 0;
    move_set_to_begin->m_node_name[0] = 0;
    move_set_to_begin->m_move_policy = ui_sprite_moving_policy_no_adj;
    return 0;
}

static void ui_sprite_moving_move_set_to_begin_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_moving_move_set_to_begin_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
	ui_sprite_moving_move_set_to_begin_t to_move_set_to_begin = ui_sprite_fsm_action_data(to);
	ui_sprite_moving_move_set_to_begin_t from_move_set_to_begin = ui_sprite_fsm_action_data(from);

	if (ui_sprite_moving_move_set_to_begin_init(to, ctx)) return -1;

	cpe_str_dup(to_move_set_to_begin->m_plan_res, sizeof(to_move_set_to_begin->m_plan_res), from_move_set_to_begin->m_plan_res);
	cpe_str_dup(to_move_set_to_begin->m_node_name, sizeof(to_move_set_to_begin->m_node_name), from_move_set_to_begin->m_node_name);
	to_move_set_to_begin->m_move_policy = from_move_set_to_begin->m_move_policy;

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_moving_move_set_to_begin_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_moving_module_t module = ctx;
    ui_sprite_moving_move_set_to_begin_t move_set_to_begin = ui_sprite_moving_move_set_to_begin_create(fsm_state, name);
    const char * plan_res;
    const char * node_name;
    const char * move_policy;
    
    if (move_set_to_begin == NULL) {
        CPE_ERROR(module->m_em, "%s: create move_set_to_begin action: create fail!", ui_sprite_moving_module_name(module));
        return NULL;
    }

    plan_res = cfg_get_string(cfg, "res", NULL);
    if (plan_res == NULL) {
        CPE_ERROR(module->m_em, "%s: create move_set_to_begin action: res not configured!", ui_sprite_moving_module_name(module));
        ui_sprite_moving_move_set_to_begin_free(move_set_to_begin);
        return NULL;
    }
    
    if (ui_sprite_moving_move_set_to_begin_set_res(move_set_to_begin, plan_res) != 0) {
        CPE_ERROR(module->m_em, "%s: create move_set_to_begin action: set res fail!", ui_sprite_moving_module_name(module));
        ui_sprite_moving_move_set_to_begin_free(move_set_to_begin);
        return NULL;
    }

    node_name = cfg_get_string(cfg, "node-name", NULL);
    if (node_name == NULL) {
        CPE_ERROR(module->m_em, "%s: create move_set_to_begin action: node-name not configured!", ui_sprite_moving_module_name(module));
        ui_sprite_moving_move_set_to_begin_free(move_set_to_begin);
        return NULL;
    }

    if (ui_sprite_moving_move_set_to_begin_set_node_name(move_set_to_begin, node_name) != 0) {
        CPE_ERROR(module->m_em, "%s: create move_set_to_begin action: set node name!", ui_sprite_moving_module_name(module));
        ui_sprite_moving_move_set_to_begin_free(move_set_to_begin);
        return NULL;
    }

    move_policy = cfg_get_string(cfg, "move-policy", NULL);
    if (move_policy == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create move_set_to_begin action: move policy not configured!",
            ui_sprite_moving_module_name(module));
        ui_sprite_moving_move_set_to_begin_free(move_set_to_begin);
        return NULL;
    }
    else if (strcmp(move_policy, "begin-at-cur-pos") == 0) {
        CPE_ERROR(
            module->m_em, "%s: create move_set_to_begin action: move policy %s no effect!",
            ui_sprite_moving_module_name(module), move_policy);
        ui_sprite_moving_move_set_to_begin_free(move_set_to_begin);
        return NULL;
    }
    else if (strcmp(move_policy, "end-at-cur-pos") == 0) {
        ui_sprite_moving_move_set_to_begin_set_move_policy(move_set_to_begin, ui_sprite_moving_policy_end_at_cur_pos);
    }
    else if (strcmp(move_policy, "no-adj") == 0) {
        ui_sprite_moving_move_set_to_begin_set_move_policy(move_set_to_begin, ui_sprite_moving_policy_no_adj);
    }
    else {
        CPE_ERROR(module->m_em, "%s: create move_set_to_begin action: unknown move policy %s!", ui_sprite_moving_module_name(module), move_policy);
        ui_sprite_moving_move_set_to_begin_free(move_set_to_begin);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(move_set_to_begin);
}

int ui_sprite_moving_move_set_to_begin_regist(ui_sprite_moving_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_MOVING_MOVE_SET_TO_BEGIN_NAME, sizeof(struct ui_sprite_moving_move_set_to_begin));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: moving enable emitter register: meta create fail",
            ui_sprite_moving_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_moving_move_set_to_begin_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_moving_move_set_to_begin_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_moving_move_set_to_begin_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_moving_move_set_to_begin_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_moving_move_set_to_begin_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_MOVING_MOVE_SET_TO_BEGIN_NAME, ui_sprite_moving_move_set_to_begin_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_moving_move_set_to_begin_unregist(ui_sprite_moving_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_MOVING_MOVE_SET_TO_BEGIN_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_moving_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_MOVING_MOVE_SET_TO_BEGIN_NAME);
}

const char * UI_SPRITE_MOVING_MOVE_SET_TO_BEGIN_NAME = "move-set-to-begin";
