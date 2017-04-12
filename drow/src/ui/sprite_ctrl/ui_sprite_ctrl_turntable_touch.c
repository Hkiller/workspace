#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_ctrl_turntable_touch_i.h"
#include "ui_sprite_ctrl_turntable_member_i.h"
#include "ui_sprite_ctrl_turntable_i.h"

ui_sprite_ctrl_turntable_touch_t ui_sprite_ctrl_turntable_touch_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CTRL_TURNTABLE_TOUCH_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ctrl_turntable_touch_free(ui_sprite_ctrl_turntable_touch_t ctrl) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(ctrl);
    ui_sprite_fsm_action_free(fsm_action);
}


int ui_sprite_ctrl_turntable_touch_set_decorator(ui_sprite_ctrl_turntable_touch_t touch, const char * decorator) {
    return ui_percent_decorator_setup(&touch->m_updator.m_decorator, decorator, touch->m_module->m_em);
}

static void ui_sprite_ctrl_turntable_touch_on_begin(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_ctrl_turntable_touch_t touch = ctx;
    ui_sprite_ctrl_module_t module = touch->m_module;
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
    UI_SPRITE_EVT_CTRL_TURNTABLE_MEMBER_TOUCH_BEGIN const * evt_data = evt->data;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    ui_sprite_ctrl_turntable_member_t member = ui_sprite_ctrl_turntable_member_find(entity);

    if (member == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable-touch: on-begin: entity not turntable member!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (member->m_turntable == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable-touch: on-begin: entity not join turntable!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    ui_sprite_ctrl_turntable_updator_stop(&touch->m_updator, member);
    touch->m_state = ui_sprite_ctrl_turntable_touch_state_idle;
    touch->m_begin_pos.x = evt_data->world_pos.x;
    touch->m_begin_pos.y = evt_data->world_pos.y;

    ui_sprite_fsm_action_sync_update(action, 1);
}

static void ui_sprite_ctrl_turntable_touch_on_move(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_ctrl_turntable_touch_t touch = ctx;
    ui_sprite_ctrl_module_t module = touch->m_module;
    UI_SPRITE_EVT_CTRL_TURNTABLE_MEMBER_TOUCH_MOVE const * evt_data = evt->data;
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    ui_sprite_ctrl_turntable_member_t member = ui_sprite_ctrl_turntable_member_find(entity);
    ui_sprite_entity_t turntable_entity;
    ui_sprite_2d_transform_t turntable_transform;
    ui_vector_2 turntable_pos;
    float distance_to_turntable;
    float target_angle;

    if (member == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable-touch: on-move: entity not turntable member!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (member->m_turntable == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable-touch: on-move: entity not join turntable!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    turntable_entity = ui_sprite_component_entity(ui_sprite_component_from_data(member->m_turntable));
    turntable_transform = ui_sprite_2d_transform_find(turntable_entity);
    if (turntable_transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable-touch: on-move: turntable %d(%s) no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_entity_id(turntable_entity), ui_sprite_entity_name(turntable_entity));
        return;
    }

    turntable_pos = ui_sprite_2d_transform_origin_pos(turntable_transform);

    distance_to_turntable = cpe_math_distance(turntable_pos.x, turntable_pos.y, evt_data->world_pos.x, evt_data->world_pos.y);
    if (distance_to_turntable <= 1.0f) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): turntable-touch: on-move: distance to turntable %f too small, skip!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), distance_to_turntable);
        }
        return;
    }

    target_angle = cpe_math_angle(turntable_pos.x, turntable_pos.y, evt_data->world_pos.x, evt_data->world_pos.y);

    touch->m_state = ui_sprite_ctrl_turntable_touch_state_move;
    ui_sprite_ctrl_turntable_updator_set_max_speed(&touch->m_updator, evt_data->max_speed);
    ui_sprite_ctrl_turntable_updator_set_angle(&touch->m_updator, member, target_angle);
}

static float ui_sprite_ctrl_turntable_touch_calc_speed_angle(
    ui_sprite_ctrl_module_t module, ui_sprite_entity_t entity, ui_sprite_ctrl_turntable_member_t member,
    UI_SPRITE_EVT_CTRL_TURNTABLE_MEMBER_TOUCH_END const * evt_data)
{
    ui_vector_2 turntable_pos;
    float speed_v;
    float radians_v;
    float radians_p;
    float radius;

    speed_v = cpe_math_distance(0.0f, 0.0f, evt_data->speed.x, evt_data->speed.y);
    if (speed_v <= 0.001f) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): turntable-touch: calc speed tangent: stop for no speed!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        }

        return 0.0f;
    }

    if (ui_sprite_ctrl_turntable_pos(member->m_turntable, &turntable_pos) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable-touch: calc speed tangent: turntable get pos fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return 0.0f;
    }

    radians_v = cpe_math_radians(0.0f, 0.0f, evt_data->speed.x, evt_data->speed.y);
    radians_p = cpe_math_radians(turntable_pos.x, turntable_pos.y, evt_data->world_pos.x, evt_data->world_pos.y);
    radius = cpe_math_distance(turntable_pos.x, turntable_pos.y, evt_data->world_pos.x, evt_data->world_pos.y);

    if (radius < 0.01) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): turntable-touch: calc speed tangent: pos radius %f too small!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), radius);
        }

        return 0.0f;
    }

    return cpe_math_radians_to_angle(speed_v * sin(radians_v - radians_p)  / radius);
}

static void ui_sprite_ctrl_turntable_touch_on_end(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_ctrl_turntable_touch_t touch = ctx;
    ui_sprite_ctrl_module_t module = touch->m_module;
    UI_SPRITE_EVT_CTRL_TURNTABLE_MEMBER_TOUCH_END const * evt_data = evt->data;
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(ctx);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    ui_sprite_ctrl_turntable_member_t member = ui_sprite_ctrl_turntable_member_find(entity);
    ui_sprite_ctrl_turntable_t turntable = member->m_turntable;
    float target_angle;
    float speed_angle;
    
    ui_sprite_ctrl_turntable_updator_stop(&touch->m_updator, member);

    if (evt_data->speed_reduce == 0) {
        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): turntable-touch: on-end: stop for no speed reduce!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        }
        return;
    }

    if (member == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable-touch: on-end: entity not turntable member!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (turntable == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable-touch: on-end: entity not join turntable!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (evt_data->distance_threshold > 0.0f) {
        float distance = cpe_math_distance(touch->m_begin_pos.x, touch->m_begin_pos.y, evt_data->world_pos.x, evt_data->world_pos.y);
        if (distance < evt_data->distance_threshold) {
            speed_angle = 0.0f;
        }
        else {
            speed_angle = ui_sprite_ctrl_turntable_touch_calc_speed_angle(module, entity, member, evt_data);
        }
    }
    else {
        speed_angle = ui_sprite_ctrl_turntable_touch_calc_speed_angle(module, entity, member, evt_data);
    }

    if (fabs(speed_angle) <= 0.01 || fabs(speed_angle) <= evt_data->speed_threshold) {
        ui_sprite_ctrl_turntable_member_t focuse_member = ui_sprite_ctrl_turntable_find_focuse_member(turntable, 360.0f);

        target_angle = ui_sprite_ctrl_turntable_calc_member_angle(turntable, member, focuse_member, turntable->m_def.focuse_angle);

        if (target_angle > member->m_angle) {
            if ((target_angle - member->m_angle) > 180) {
                target_angle -= 360;
            }
        }
        else {
            if ((member->m_angle - target_angle) > 180) {
                target_angle += 360;
            }
        }
    }
    else {
        float save_angle = member->m_angle;
        float lock_target_angle;
        ui_sprite_ctrl_turntable_member_t focuse_member;

        target_angle = member->m_angle + speed_angle * (fabs(speed_angle) / evt_data->speed_reduce);

        /*讲角度先设置到最终结果，计算锁定的member的角度 */
        ui_sprite_ctrl_turntable_update_members_angle(turntable, member, target_angle);
        focuse_member = ui_sprite_ctrl_turntable_find_focuse_member(turntable, 360.0f);

        lock_target_angle =
            ui_sprite_ctrl_turntable_calc_member_angle(
                turntable, member,
                focuse_member,
                turntable->m_def.focuse_angle);
        ui_sprite_ctrl_turntable_update_members_angle(turntable, member, save_angle);

        ui_sprite_ctrl_turntable_set_focuse_member(turntable, NULL);

        if (speed_angle > 0.0f) {
            while(lock_target_angle < target_angle) lock_target_angle += 360.0f;
        }
        else {
            while(lock_target_angle > target_angle) lock_target_angle -= 360.0f;
        }

        target_angle = lock_target_angle;
    }

    touch->m_state = ui_sprite_ctrl_turntable_touch_state_move_by_speed;
    ui_sprite_ctrl_turntable_updator_set_max_speed(&touch->m_updator, fabs(speed_angle) < 200 ? 200 : fabs(speed_angle));
    ui_sprite_ctrl_turntable_updator_set_angle(&touch->m_updator, member, target_angle);

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): turntable-touch: on-end: move by speed, speed=%f!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            speed_angle);
    }
}

static int ui_sprite_ctrl_turntable_touch_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_turntable_touch_t touch = ui_sprite_fsm_action_data(fsm_action);
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if ((ui_sprite_fsm_action_add_event_handler(
             fsm_action, ui_sprite_event_scope_self, 
             "ui_sprite_evt_ctrl_turntable_member_touch_begin",
             ui_sprite_ctrl_turntable_touch_on_begin, touch) != 0)
        ||
        (ui_sprite_fsm_action_add_event_handler(
             fsm_action, ui_sprite_event_scope_self, 
             "ui_sprite_evt_ctrl_turntable_member_touch_move",
             ui_sprite_ctrl_turntable_touch_on_move, touch) != 0)
        ||
        (ui_sprite_fsm_action_add_event_handler(
             fsm_action, ui_sprite_event_scope_self, 
             "ui_sprite_evt_ctrl_turntable_member_touch_end",
             ui_sprite_ctrl_turntable_touch_on_end, touch) != 0)
        )
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable: enter: add eventer handler fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    return 0;
}

static void ui_sprite_ctrl_turntable_touch_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_turntable_touch_t touch = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_ctrl_turntable_member_t member = ui_sprite_ctrl_turntable_member_find(entity);

    ui_sprite_ctrl_turntable_updator_stop(&touch->m_updator, member);
}

static void ui_sprite_ctrl_turntable_touch_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_ctrl_turntable_touch_t touch = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_ctrl_turntable_member_t member = ui_sprite_ctrl_turntable_member_find(entity);
    ui_sprite_ctrl_turntable_t turntable;

    if (touch->m_state == ui_sprite_ctrl_turntable_touch_state_idle) return;

    if (member == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable-touch: on-udpate: entity not turntable member!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (member->m_turntable == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): turntable-touch: on-update: entity not join turntable!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    ui_sprite_ctrl_turntable_updator_update(&touch->m_updator, member, delta);

    turntable = member->m_turntable;

    if (touch->m_state == ui_sprite_ctrl_turntable_touch_state_move) {
        ui_sprite_ctrl_turntable_member_t focuse_member =
            ui_sprite_ctrl_turntable_find_focuse_member(turntable, touch->m_focuse_angle_range);
        if (focuse_member && focuse_member != turntable->m_focuse_member) {
            ui_sprite_ctrl_turntable_update_members_angle(turntable, focuse_member, turntable->m_def.focuse_angle);
            ui_sprite_ctrl_turntable_set_focuse_member(turntable, focuse_member);
        }
    }

    ui_sprite_ctrl_turntable_update_members_transform(turntable);

    if (!ui_sprite_ctrl_turntable_updator_is_runing(&touch->m_updator)) {
        if (touch->m_state == ui_sprite_ctrl_turntable_touch_state_move_by_speed) {
            ui_sprite_ctrl_turntable_member_t focuse_member;

            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): turntable-touch: update: move by speed stop for done!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            }

            focuse_member = ui_sprite_ctrl_turntable_find_focuse_member(turntable, touch->m_focuse_angle_range);
            if (focuse_member != turntable->m_focuse_member) {
                //ui_sprite_ctrl_turntable_update_members_angle(turntable, focuse_member, turntable->m_def.focuse_angle);
                ui_sprite_ctrl_turntable_set_focuse_member(turntable, focuse_member);
            }

            ui_sprite_fsm_action_stop_update(fsm_action);
        }
        else {
            touch->m_state = ui_sprite_ctrl_turntable_touch_state_idle;
        }
    }

}

static int ui_sprite_ctrl_turntable_touch_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_turntable_touch_t ctrl = ui_sprite_fsm_action_data(fsm_action);

    bzero(ctrl, sizeof(*ctrl));

    ctrl->m_module = ctx;
    ctrl->m_focuse_angle_range = 3.0f;

    return 0;
}

static void ui_sprite_ctrl_turntable_touch_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ctrl_turntable_touch_t touch = ui_sprite_fsm_action_data(fsm_action);

    assert(touch->m_updator.m_curent_op_id == 0);
}

static int ui_sprite_ctrl_turntable_touch_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_ctrl_turntable_touch_t to_touch = ui_sprite_fsm_action_data(to);
    ui_sprite_ctrl_turntable_touch_t from_touch = ui_sprite_fsm_action_data(from);

    if (ui_sprite_ctrl_turntable_touch_init(to, ctx)) return -1;

    to_touch->m_decorator = from_touch->m_decorator;
    to_touch->m_focuse_angle_range = from_touch->m_focuse_angle_range;

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_ctrl_turntable_touch_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_ctrl_module_t module = ctx;
    ui_sprite_ctrl_turntable_touch_t turntable_touch = ui_sprite_ctrl_turntable_touch_create(fsm_state, name);
    const char * decorator;

    if (turntable_touch == NULL) {
        CPE_ERROR(module->m_em, "%s: create ctrl_turntable_touch action: create fail!", ui_sprite_ctrl_module_name(module));
        return NULL;
    }

    if ((decorator = cfg_get_string(cfg, "decorator", NULL))) {
        if (ui_sprite_ctrl_turntable_touch_set_decorator(turntable_touch, decorator) != 0) {
            CPE_ERROR(module->m_em, "%s: create ctrl_turntable_touch action: create fail!", ui_sprite_ctrl_module_name(module));
            ui_sprite_ctrl_turntable_touch_free(turntable_touch);
            return NULL;
        }
    }

    return ui_sprite_fsm_action_from_data(turntable_touch);
}

int ui_sprite_ctrl_turntable_touch_regist(ui_sprite_ctrl_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_CTRL_TURNTABLE_TOUCH_NAME, sizeof(struct ui_sprite_ctrl_turntable_touch));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ctrl camera ctrl register: meta create fail",
            ui_sprite_ctrl_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ctrl_turntable_touch_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ctrl_turntable_touch_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ctrl_turntable_touch_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ctrl_turntable_touch_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ctrl_turntable_touch_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_ctrl_turntable_touch_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_CTRL_TURNTABLE_TOUCH_NAME, ui_sprite_ctrl_turntable_touch_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_ctrl_turntable_touch_unregist(ui_sprite_ctrl_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CTRL_TURNTABLE_TOUCH_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ctrl camera ctrl unregister: meta not exist",
            ui_sprite_ctrl_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CTRL_TURNTABLE_TOUCH_NAME);
    }
}

const char * UI_SPRITE_CTRL_TURNTABLE_TOUCH_NAME = "ctrl-turntable-touch";

