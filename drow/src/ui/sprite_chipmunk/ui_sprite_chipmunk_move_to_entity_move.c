#include <assert.h>
#include <stdio.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_chipmunk_move_to_entity_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

cpVect ui_sprite_chipmunk_move_to_entity_move_work(
    ui_sprite_chipmunk_move_to_entity_t move_to_entity,
    ui_sprite_entity_t target_entity, ui_sprite_chipmunk_obj_body_t target_body, ui_vector_2_t target_pos,
    ui_sprite_entity_t self_entity, ui_sprite_chipmunk_obj_body_t self_body, ui_vector_2_t self_pos)
{
    cpVect cur_speed;
    float self_mass = cpBodyGetMass(&self_body->m_body);
    float target_radians;
    float target_speed_value;
    cpVect target_speed;
    cpVect apply_speed;
    float apply_accel_value;
    float apply_accel_radians;
    cpVect addition_accel = cpv(0.0f, 0.0f);
    
    /*计算目标速度 */
    target_radians = cpe_math_radians(self_pos->x, self_pos->y, target_pos->x, target_pos->y);
    if (move_to_entity->m_move.m_stop) {
        target_speed_value = cpe_math_distance(self_pos->x, self_pos->y, target_pos->x, target_pos->y);
        if (target_speed_value > move_to_entity->m_move.m_max_speed) {
            target_speed_value = move_to_entity->m_move.m_max_speed;
        }
    }
    else {
        target_speed_value = move_to_entity->m_move.m_max_speed;
    }
    
    target_speed.x = target_speed_value * cpe_cos_radians(target_radians);
    target_speed.y = target_speed_value * cpe_sin_radians(target_radians);

    cur_speed = cpBodyGetVelocity(&self_body->m_body);
    if (move_to_entity->m_move.m_force) {
        float cur_speed_value = cpe_math_distance(0.0f, 0.0f, cur_speed.x, cur_speed.y) * move_to_entity->m_damping;
        cpVect adj_target = cpv(cpe_cos_radians(target_radians) * cur_speed_value, cpe_sin_radians(target_radians) * cur_speed_value);

        addition_accel.x = (adj_target.x - cur_speed.x) / move_to_entity->m_step_duration;
        addition_accel.y = (adj_target.y - cur_speed.y) / move_to_entity->m_step_duration;

        cur_speed.x = adj_target.x;
        cur_speed.y = adj_target.y;
    }
    else if (move_to_entity->m_damping != 1.0f) {
        float cur_speed_value = cpe_math_distance(0.0f, 0.0f, cur_speed.x, cur_speed.y) * move_to_entity->m_damping;
        float cur_speed_radians = cpe_math_radians(0.0f, 0.0f, cur_speed.x, cur_speed.y);
        cur_speed.x = cpe_cos_radians(cur_speed_radians) * cur_speed_value;
        cur_speed.y = cpe_sin_radians(cur_speed_radians) * cur_speed_value;
    }

    apply_speed.x = target_speed.x - cur_speed.x;
    apply_speed.y = target_speed.y - cur_speed.y;

    apply_accel_radians = cpe_math_radians(0.0f, 0.0f, apply_speed.x, apply_speed.y);
    apply_accel_value = cpe_math_distance(0.0f, 0.0f, apply_speed.x, apply_speed.y);

    /* printf( */
    /*     "xxxxx: entity %d(%s): speed: (%f,%f: %f) ==> (%f,%f: %f) = (%f,%f: %f)\n", */
    /*     ui_sprite_entity_id(self_entity), ui_sprite_entity_name(self_entity),  */
    /*     cur_speed.x, cur_speed.y, cpe_math_distance(0.0f, 0.0f, cur_speed.x, cur_speed.y), */
    /*     target_speed.x, target_speed.y, cpe_math_distance(0.0f, 0.0f, target_speed.x, target_speed.y), */
    /*     apply_speed.x, apply_speed.y, apply_accel_value); */
    
    if (apply_accel_value > move_to_entity->m_move.m_max_accel) {
        apply_accel_value = move_to_entity->m_move.m_max_accel;
    }

    return cpv(
        (cpe_cos_radians(apply_accel_radians) * apply_accel_value + addition_accel.x) * self_mass,
        (cpe_sin_radians(apply_accel_radians) * apply_accel_value + addition_accel.y) * self_mass);
}
    
int ui_sprite_chipmunk_move_to_entity_move_enter(
    ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_move_to_entity_t move_to_entity,
    ui_sprite_entity_t entity, ui_sprite_fsm_action_t fsm_action, float ptm)
{
    if (move_to_entity->m_cfg_move.m_cfg_max_accel == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk move to target: no max accel configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (move_to_entity->m_cfg_move.m_cfg_max_speed == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk move to target: no max speed configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (ui_sprite_fsm_action_check_calc_float(
            &move_to_entity->m_move.m_max_accel, move_to_entity->m_cfg_move.m_cfg_max_accel, fsm_action, NULL, module->m_em)
        != 0
        || ui_sprite_fsm_action_check_calc_float(
            &move_to_entity->m_move.m_max_speed, move_to_entity->m_cfg_move.m_cfg_max_speed, fsm_action, NULL, module->m_em)
        != 0)
    {
        return -1;
    }

    move_to_entity->m_move.m_max_accel *= ptm;
    move_to_entity->m_move.m_max_speed *= ptm;
    move_to_entity->m_move.m_force = move_to_entity->m_cfg_move.m_cfg_force;
    move_to_entity->m_move.m_stop = move_to_entity->m_cfg_move.m_cfg_stop;
    
    return 0;
}

int ui_sprite_chipmunk_move_to_entity_move_copy(
    ui_sprite_chipmunk_move_to_entity_t to_move_to_entity, ui_sprite_chipmunk_move_to_entity_t from_move_to_entity)
{
    if (from_move_to_entity->m_cfg_move.m_cfg_max_accel) {
        to_move_to_entity->m_cfg_move.m_cfg_max_accel
            = cpe_str_mem_dup(to_move_to_entity->m_module->m_alloc, from_move_to_entity->m_cfg_move.m_cfg_max_accel);
    }

    if (from_move_to_entity->m_cfg_move.m_cfg_max_speed) {
        to_move_to_entity->m_cfg_move.m_cfg_max_speed
            = cpe_str_mem_dup(to_move_to_entity->m_module->m_alloc, from_move_to_entity->m_cfg_move.m_cfg_max_speed);
    }

    to_move_to_entity->m_cfg_move.m_cfg_force = from_move_to_entity->m_cfg_move.m_cfg_force;
    to_move_to_entity->m_cfg_move.m_cfg_stop = from_move_to_entity->m_cfg_move.m_cfg_stop;
    
    return 0;
}

void ui_sprite_chipmunk_move_to_entity_move_free(ui_sprite_chipmunk_move_to_entity_t move_to_entity) {
    if (move_to_entity->m_cfg_move.m_cfg_max_speed) {
        mem_free(move_to_entity->m_module->m_alloc, move_to_entity->m_cfg_move.m_cfg_max_speed);
        move_to_entity->m_cfg_move.m_cfg_max_speed = NULL;
    }

    if (move_to_entity->m_cfg_move.m_cfg_max_accel) {
        mem_free(move_to_entity->m_module->m_alloc, move_to_entity->m_cfg_move.m_cfg_max_accel);
        move_to_entity->m_cfg_move.m_cfg_max_accel = NULL;
    }
}
    
int ui_sprite_chipmunk_move_to_entity_move_load(ui_sprite_chipmunk_move_to_entity_t move_to_entity, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = move_to_entity->m_module;
    const char * str_value;

    move_to_entity->m_type = ui_sprite_chipmunk_move_to_entity_move;

    if ((str_value = cfg_get_string(cfg, "max-accel", NULL))) {
        move_to_entity->m_cfg_move.m_cfg_max_accel = cpe_str_mem_dup_trim(module->m_alloc, str_value);
        if (move_to_entity->m_cfg_move.m_cfg_max_accel == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create move_to_entity action: max-accel %s load fail!",
                ui_sprite_chipmunk_module_name(module), str_value);
            return -1;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create move_to_entity action: max-accel not configured!",
            ui_sprite_chipmunk_module_name(module));
        return -1;
    }

    if ((str_value = cfg_get_string(cfg, "max-speed", NULL))) {
        move_to_entity->m_cfg_move.m_cfg_max_speed = cpe_str_mem_dup_trim(module->m_alloc, str_value);
        if (move_to_entity->m_cfg_move.m_cfg_max_speed == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create move_to_entity action: max-speed %s load fail!",
                ui_sprite_chipmunk_module_name(module), str_value);
            return -1;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create move_to_entity action: max-speed not configured!",
            ui_sprite_chipmunk_module_name(module));
        return -1;
    }

    move_to_entity->m_cfg_move.m_cfg_force = cfg_get_uint8(cfg, "force", 1);
    move_to_entity->m_cfg_move.m_cfg_stop = cfg_get_uint8(cfg, "stop", 0);

    return 0;
}

#ifdef __cplusplus
}
#endif
    
