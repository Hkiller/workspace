#include <assert.h>
#include "gd/app/app_log.h"
#include "cpe/utils/math_ex.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_vector_2.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_2d_move_i.h"
#include "ui_sprite_2d_module_i.h"

ui_sprite_2d_move_t
ui_sprite_2d_move_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action;
    fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_2D_MOVE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_2d_move_free(ui_sprite_2d_move_t show_anim) {
    ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(show_anim));
}

int ui_sprite_2d_move_set_decorator(ui_sprite_2d_move_t move, const char * decorator) {
    return ui_percent_decorator_setup(&move->m_cfg_decorator, decorator, move->m_module->m_em);
}

int ui_sprite_2d_move_set_max_speed(ui_sprite_2d_move_t move, const char * max_speed) {
    ui_sprite_2d_module_t module = move->m_module;
    
    if (move->m_cfg_max_speed) {
        mem_free(module->m_alloc, move->m_cfg_max_speed);
    }

    if (max_speed) {
        move->m_cfg_max_speed = cpe_str_mem_dup(module->m_alloc, max_speed);
        return move->m_cfg_max_speed == NULL ? -1 : 0;
    }
    else {
        move->m_cfg_max_speed = NULL;
        return 0;
    }
}

const char * ui_sprite_2d_move_max_speed(ui_sprite_2d_move_t move) {
    return move->m_cfg_max_speed;
}

int ui_sprite_2d_move_set_take_time(ui_sprite_2d_move_t move, const char * take_time) {
    ui_sprite_2d_module_t module = move->m_module;
    
    if (move->m_cfg_take_time) {
        mem_free(module->m_alloc, move->m_cfg_take_time);
    }

    if (take_time) {
        move->m_cfg_take_time = cpe_str_mem_dup(module->m_alloc, take_time);
        return move->m_cfg_take_time == NULL ? -1 : 0;
    }
    else {
        move->m_cfg_take_time = NULL;
        return 0;
    }
}

const char * ui_sprite_2d_move_take_time(ui_sprite_2d_move_t move) {
    return move->m_cfg_take_time;
}

static int ui_sprite_2d_move_update_begin_pos(
    ui_sprite_2d_move_t move, ui_sprite_entity_t self_entity, ui_sprite_fsm_action_t fsm_action)
{
    ui_sprite_2d_transform_t transform;

    transform = ui_sprite_2d_transform_find(self_entity);
    if (transform == NULL) {
        CPE_ERROR(
            move->m_module->m_em, "entity %d(%s): %s: self no transform!",
            ui_sprite_entity_id(self_entity), ui_sprite_entity_name(self_entity), ui_sprite_fsm_action_name(fsm_action));
        return -1;
    }

    move->m_begin_pos = ui_sprite_2d_transform_origin_pos(transform);
    if (!move->m_process_x) move->m_begin_pos.x = 0.0f;
    if (!move->m_process_y) move->m_begin_pos.y = 0.0f;
    
    return 0;
}

static void ui_sprite_2d_move_update_take_time(ui_sprite_2d_move_t move) {
    if (move->m_cfg_take_time == NULL && move->m_max_speed > 0.0f) {
        float distance = cpe_math_distance(move->m_begin_pos.x, move->m_begin_pos.y, move->m_target_pos.x, move->m_target_pos.y);
        move->m_take_time = distance / move->m_max_speed;
        //printf("distance=%f, speed=%f, take-time=%f\n", distance, move->m_max_speed, move->m_take_time);
    }
}

static void ui_sprite_2d_move_update_pos(ui_sprite_2d_move_t move, float delta, ui_sprite_entity_t entity) {
    ui_sprite_2d_transform_t transform;
    ui_vector_2 pos;
    
    if (move->m_take_time == 0) {
        move->m_cur_pos = move->m_target_pos;
    }
    else {
        float percent;
        ui_vector_2 result_pos;
        
        if (move->m_runing_time > move->m_take_time) return;
        
        move->m_runing_time += delta;
        percent = move->m_runing_time > move->m_take_time ? 1.0f : move->m_runing_time / move->m_take_time;

        percent = ui_percent_decorator_decorate(&move->m_cfg_decorator, percent);

        result_pos.x = move->m_process_x ? (move->m_begin_pos.x + (move->m_target_pos.x - move->m_begin_pos.x) * percent) : 0.0f;
        result_pos.y = move->m_process_y ? (move->m_begin_pos.y + (move->m_target_pos.y - move->m_begin_pos.y) * percent) : 0.0f;

        move->m_cur_pos = result_pos;
    }

    transform = ui_sprite_2d_transform_find(entity);
    assert(entity);

    pos = ui_sprite_2d_transform_origin_pos(transform);
    if (move->m_process_x) pos.x = move->m_cur_pos.x;
    if (move->m_process_y) pos.y = move->m_cur_pos.y;
    ui_sprite_2d_transform_set_origin_pos(transform, pos);
}

static uint8_t ui_sprite_2d_move_need_work(ui_sprite_2d_move_t move, ui_sprite_fsm_action_t fsm_action) {
    if(ui_sprite_fsm_action_life_circle(fsm_action) == ui_sprite_fsm_action_life_circle_working) {
        /*workding策略用于卡点，所以到达目标以后就停止 */
        return move->m_take_time > 0.0f && move->m_runing_time < move->m_take_time ? 1 : 0;
    }
    else {
        /*其他策略用于跟踪对象，永远update */
        return 1;
    }
}

static int ui_sprite_2d_move_calc_target_pos(ui_vector_2_t target_pos, ui_sprite_2d_move_t move, ui_sprite_entity_t entity, ui_sprite_fsm_action_t fsm_action) {
    ui_sprite_2d_module_t module = move->m_module;
    
    if (move->m_cfg_is_to_entity) {
        ui_sprite_world_t world = ui_sprite_entity_world(entity);
        ui_sprite_entity_t target_entity;
        ui_sprite_2d_transform_t target_transform;
        uint32_t target_entity_id;

        if (move->m_to_entity.m_cfg_target_entity == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: target entity not configured!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_action_name(fsm_action));
            return -1;
        }
        else {
            if (ui_sprite_fsm_action_check_calc_entity_id(&target_entity_id, move->m_to_entity.m_cfg_target_entity, fsm_action, NULL, module->m_em)!= 0) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): %s: calc target entity from %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_action_name(fsm_action), move->m_to_entity.m_cfg_target_entity);
                return -1;
            }
        }

        target_entity = ui_sprite_entity_find_by_id(world, target_entity_id);
        if (target_entity == NULL) {
            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): %s: target entity %d not exist!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_action_name(fsm_action),
                    target_entity_id);
            }
            return 0;
        }

        target_transform = ui_sprite_2d_transform_find(target_entity);
        if (target_transform == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: target entity %d no transform!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_action_name(fsm_action),
                target_entity_id);
            return -1;
        }

        *target_pos = ui_sprite_2d_transform_world_pos(target_transform, move->m_to_entity.m_cfg_pos_policy, UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL);
    }
    else {
        if (move->m_to_pos.m_cfg_x) {
            if (ui_sprite_fsm_action_check_calc_float(&target_pos->x, move->m_to_pos.m_cfg_x, fsm_action, NULL, module->m_em) != 0) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): 2d-move: calc target-x from %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move->m_to_pos.m_cfg_x);
                return -1;
            }
        }
        else {
            target_pos->x = 0.0f;
        }

        if (move->m_to_pos.m_cfg_y) {
            if (ui_sprite_fsm_action_check_calc_float(&target_pos->y, move->m_to_pos.m_cfg_y, fsm_action, NULL, module->m_em) != 0) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): 2d-move: calc target-y from %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move->m_to_pos.m_cfg_y);
                return -1;
            }
        }
        else {
            target_pos->y = 0.0f;
        }
    }

    if (!move->m_process_x) target_pos->x = 0.0f;
    if (!move->m_process_y) target_pos->y = 0.0f;

    return 0;
}
    
int ui_sprite_2d_move_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_move_t move = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_2d_module_t module = move->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (move->m_cfg_take_time) {
        if (ui_sprite_fsm_action_check_calc_float(&move->m_take_time, move->m_cfg_take_time, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: take time calc from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_action_name(fsm_action), move->m_cfg_take_time);
            return -1;
        }
    }
    else {
        move->m_take_time = 0.0f;
    }

    if (move->m_cfg_max_speed) {
        if (ui_sprite_fsm_action_check_calc_float(&move->m_max_speed, move->m_cfg_max_speed, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): %s: max speed calc from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ui_sprite_fsm_action_name(fsm_action), move->m_cfg_max_speed);
            return -1;
        }
    }
    else {
        move->m_max_speed = 0.0f;
    }

    if (move->m_cfg_is_to_entity) {
        move->m_process_x = move->m_to_entity.m_cfg_process_x;
        move->m_process_y = move->m_to_entity.m_cfg_process_y;
    }
    else {
        move->m_process_x = move->m_to_pos.m_cfg_x ? 1 : 0;
        move->m_process_y = move->m_to_pos.m_cfg_y ? 1 : 0;
    }

    if (ui_sprite_2d_move_calc_target_pos(&move->m_target_pos, move, entity, fsm_action) != 0) return -1;
    if (ui_sprite_2d_move_update_begin_pos(move, entity, fsm_action) != 0) return -1;
    ui_sprite_2d_move_update_take_time(move);
    
    move->m_runing_time = 0.0f;
    ui_sprite_2d_move_update_pos(move, 0.0f, entity);

    if (ui_sprite_2d_move_need_work(move, fsm_action)) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }

    return 0;
}

void ui_sprite_2d_move_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

int ui_sprite_2d_move_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_2d_move_t move_to = ui_sprite_fsm_action_data(fsm_action);

    bzero(move_to, sizeof(*move_to));
    move_to->m_module = ctx;

    return 0;
}

int ui_sprite_2d_move_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_2d_module_t module = ctx;
    ui_sprite_2d_move_t to_move_to = ui_sprite_fsm_action_data(to);
    ui_sprite_2d_move_t from_move_to = ui_sprite_fsm_action_data(from);

    ui_sprite_2d_move_init(to, ctx);

    to_move_to->m_cfg_decorator = from_move_to->m_cfg_decorator;
    to_move_to->m_cfg_is_to_entity = from_move_to->m_cfg_is_to_entity;
    
    if (from_move_to->m_cfg_is_to_entity) {
        if (from_move_to->m_to_entity.m_cfg_target_entity) {
            to_move_to->m_to_entity.m_cfg_target_entity = cpe_str_mem_dup(module->m_alloc, from_move_to->m_to_entity.m_cfg_target_entity);
        }
        to_move_to->m_to_entity.m_cfg_pos_policy = from_move_to->m_to_entity.m_cfg_pos_policy;
        to_move_to->m_to_entity.m_cfg_process_x = from_move_to->m_to_entity.m_cfg_process_x;
        to_move_to->m_to_entity.m_cfg_process_y = from_move_to->m_to_entity.m_cfg_process_y;
    }
    else {
        if (from_move_to->m_to_pos.m_cfg_x) {
            to_move_to->m_to_pos.m_cfg_x = cpe_str_mem_dup(module->m_alloc, from_move_to->m_to_pos.m_cfg_x);
        }

        if (from_move_to->m_to_pos.m_cfg_y) {
            to_move_to->m_to_pos.m_cfg_y = cpe_str_mem_dup(module->m_alloc, from_move_to->m_to_pos.m_cfg_y);
        }
    }

    if (from_move_to->m_cfg_max_speed) {
        to_move_to->m_cfg_max_speed = cpe_str_mem_dup(module->m_alloc, from_move_to->m_cfg_max_speed);
    }

    if (from_move_to->m_cfg_take_time) {
        to_move_to->m_cfg_take_time = cpe_str_mem_dup(module->m_alloc, from_move_to->m_cfg_take_time);
    }
    
    return 0;
}

void ui_sprite_2d_move_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_2d_move_t move = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_2d_module_t module = move->m_module;

    if (move->m_cfg_is_to_entity) {
        if (move->m_to_entity.m_cfg_target_entity) {
            mem_free(module->m_alloc, move->m_to_entity.m_cfg_target_entity);
            move->m_to_entity.m_cfg_target_entity = NULL;
        }
    }
    else {
        if (move->m_to_pos.m_cfg_x) {
            mem_free(module->m_alloc, move->m_to_pos.m_cfg_x);
            move->m_to_pos.m_cfg_x = NULL;
        }

        if (move->m_to_pos.m_cfg_y) {
            mem_free(module->m_alloc, move->m_to_pos.m_cfg_y);
            move->m_to_pos.m_cfg_y = NULL;
        }
    }

    if (move->m_cfg_max_speed) {
        mem_free(module->m_alloc, move->m_cfg_max_speed);
        move->m_cfg_max_speed = NULL;
    }

    if (move->m_cfg_take_time) {
        mem_free(module->m_alloc, move->m_cfg_take_time);
        move->m_cfg_take_time = NULL;
    }
}

void ui_sprite_2d_move_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
	ui_sprite_2d_move_t move = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_vector_2 cur_target_pos;

    if (ui_sprite_2d_move_calc_target_pos(&cur_target_pos, move, entity, fsm_action) != 0) {
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (cur_target_pos.x != move->m_target_pos.x || cur_target_pos.y != move->m_target_pos.y) {
        /*目标位置改变以后需要重新计 */
        move->m_begin_pos = move->m_cur_pos;
        move->m_target_pos = cur_target_pos;
        move->m_runing_time = 0.0f;
        ui_sprite_2d_move_update_take_time(move);
    }

    ui_sprite_2d_move_update_pos(move, delta, entity);

    if (!ui_sprite_2d_move_need_work(move, fsm_action)) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

ui_sprite_fsm_action_t ui_sprite_2d_move_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
	ui_sprite_2d_module_t module = ctx;
	ui_sprite_2d_move_t p2d_move = ui_sprite_2d_move_create(fsm_state, name);
    const char * str_value;

	if (p2d_move == NULL) {
		CPE_ERROR(module->m_em, "%s: create anim_2d_move action: create fail!", ui_sprite_2d_module_name(module));
		return NULL;
	}

    if ((str_value = cfg_get_string(cfg, "target-entity", NULL))) {
        p2d_move->m_cfg_is_to_entity = 1;

        p2d_move->m_to_entity.m_cfg_target_entity = cpe_str_mem_dup_trim(module->m_alloc, str_value);
        p2d_move->m_to_entity.m_cfg_process_x = cfg_get_uint8(cfg, "process-x", 1);
        p2d_move->m_to_entity.m_cfg_process_y = cfg_get_uint8(cfg, "process-y", 1);

    	p2d_move->m_to_entity.m_cfg_pos_policy = UI_SPRITE_2D_TRANSFORM_POS_ORIGIN;
        if ((str_value = cfg_get_string(cfg, "pos-policy", NULL))) {
            p2d_move->m_to_entity.m_cfg_pos_policy = ui_sprite_2d_transform_pos_policy_from_str(str_value);
            if (p2d_move->m_to_entity.m_cfg_pos_policy == 0) {
                CPE_ERROR(
                    module->m_em, "%s: create anim_2d_move: pos-policy %s is unknown!",
                    ui_sprite_2d_module_name(module), str_value);
                ui_sprite_2d_move_free(p2d_move);
                return NULL;
            }
        }
    }
    else {
        p2d_move->m_cfg_is_to_entity = 0;

        if ((str_value = cfg_get_string(cfg, "pos.x", NULL))) {
            p2d_move->m_to_pos.m_cfg_x = cpe_str_mem_dup_trim(module->m_alloc, str_value);
        }

        if ((str_value = cfg_get_string(cfg, "pos.y", NULL))) {
            p2d_move->m_to_pos.m_cfg_y = cpe_str_mem_dup_trim(module->m_alloc, str_value);
        }
    }
    
    if ((str_value = cfg_get_string(cfg, "decorator", NULL))) {
        if (ui_sprite_2d_move_set_decorator(p2d_move, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create 2d move follow action: decorator %s set fail!", ui_sprite_2d_module_name(module), str_value);
            ui_sprite_2d_move_free(p2d_move);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "take-time", NULL))) {
        ui_sprite_2d_move_set_take_time(p2d_move, str_value);
    }

    if ((str_value = cfg_get_string(cfg, "max-speed", NULL))) {
        ui_sprite_2d_move_set_max_speed(p2d_move, str_value);
    }
    
	return ui_sprite_fsm_action_from_data(p2d_move);
}

int ui_sprite_2d_move_regist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_2D_MOVE_NAME, sizeof(struct ui_sprite_2d_move));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_MOVE_NAME);
        return -1;
    }

    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_2d_move_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_2d_move_copy, module);
    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_2d_move_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_2d_move_exit, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_2d_move_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_2d_move_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_2D_MOVE_NAME, ui_sprite_2d_move_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_2d_move_unregist(ui_sprite_2d_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_2D_MOVE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_2d_module_name(module), UI_SPRITE_2D_MOVE_NAME);
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_2D_MOVE_NAME);
    }
}

const char * UI_SPRITE_2D_MOVE_NAME = "2d-move";
