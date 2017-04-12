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

cpVect ui_sprite_chipmunk_move_to_entity_spring_work(
    ui_sprite_chipmunk_move_to_entity_t move_to_entity,
    ui_sprite_entity_t target_entity, ui_sprite_chipmunk_obj_body_t target_body, ui_vector_2_t target_pos,
    ui_sprite_entity_t self_entity, ui_sprite_chipmunk_obj_body_t self_body, ui_vector_2_t self_pos)
{
    float distance;
    float radians;
    float apply_force_value;

    /*计算目标速度 */
    radians = cpe_math_radians(target_pos->x, target_pos->y, self_pos->x, self_pos->y);
    distance = cpe_math_distance(target_pos->x, target_pos->y, self_pos->x, self_pos->y);

    if (move_to_entity->m_spring.m_max_distance > 0.0f) {
        distance = cpe_min(distance, move_to_entity->m_spring.m_max_distance);
    }
    distance -= move_to_entity->m_spring.m_base_distance;
     
    apply_force_value = move_to_entity->m_spring.m_K * distance;

    return  cpv(
        cpe_cos_radians(radians) * apply_force_value,
        cpe_sin_radians(radians) * apply_force_value);
}
    
int ui_sprite_chipmunk_move_to_entity_spring_enter(
    ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_move_to_entity_t move_to_entity,
    ui_sprite_entity_t entity, ui_sprite_fsm_action_t fsm_action, float ptm)
{
    if (move_to_entity->m_cfg_spring.m_cfg_base_distance == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk move to entity(spring): min-distance not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    else if (ui_sprite_fsm_action_check_calc_float(
                 &move_to_entity->m_spring.m_base_distance, move_to_entity->m_cfg_spring.m_cfg_base_distance, fsm_action, NULL, module->m_em)
             != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk move to entity(spring): calc min-distance from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_to_entity->m_cfg_spring.m_cfg_base_distance);
        return -1;
    }
    move_to_entity->m_spring.m_base_distance *= ptm;

    if (move_to_entity->m_cfg_spring.m_cfg_max_distance == NULL) {
        move_to_entity->m_spring.m_max_distance = 0.0f;
    }
    else {
        if (ui_sprite_fsm_action_check_calc_float(
                &move_to_entity->m_spring.m_max_distance, move_to_entity->m_cfg_spring.m_cfg_max_distance, fsm_action, NULL, module->m_em)
            != 0)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk move to entity(spring): calc min-distance from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_to_entity->m_cfg_spring.m_cfg_max_distance);
            return -1;
        }
    }
    move_to_entity->m_spring.m_max_distance *= ptm;

    if (move_to_entity->m_cfg_spring.m_cfg_K == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk move to entity(spring): K not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    else if (ui_sprite_fsm_action_check_calc_float(
                 &move_to_entity->m_spring.m_K,
                 move_to_entity->m_cfg_spring.m_cfg_K, fsm_action, NULL, module->m_em)
             != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk move to entity(spring): calc K from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_to_entity->m_cfg_spring.m_cfg_K);
        return -1;
    }
    move_to_entity->m_spring.m_K *= ptm;

    return 0;
}

int ui_sprite_chipmunk_move_to_entity_spring_copy(
    ui_sprite_chipmunk_move_to_entity_t to_move_to_entity, ui_sprite_chipmunk_move_to_entity_t from_move_to_entity)
{
    if (from_move_to_entity->m_cfg_spring.m_cfg_base_distance) {
        to_move_to_entity->m_cfg_spring.m_cfg_base_distance
            = cpe_str_mem_dup(to_move_to_entity->m_module->m_alloc, from_move_to_entity->m_cfg_spring.m_cfg_base_distance);
    }

    if (from_move_to_entity->m_cfg_spring.m_cfg_max_distance) {
        to_move_to_entity->m_cfg_spring.m_cfg_max_distance
            = cpe_str_mem_dup(to_move_to_entity->m_module->m_alloc, from_move_to_entity->m_cfg_spring.m_cfg_max_distance);
    }
    
    if (from_move_to_entity->m_cfg_spring.m_cfg_K) {
        to_move_to_entity->m_cfg_spring.m_cfg_K
            = cpe_str_mem_dup(to_move_to_entity->m_module->m_alloc, from_move_to_entity->m_cfg_spring.m_cfg_K);
    }

    return 0;
}

void ui_sprite_chipmunk_move_to_entity_spring_free(ui_sprite_chipmunk_move_to_entity_t move_to_entity) {
    if (move_to_entity->m_cfg_spring.m_cfg_base_distance) {
        mem_free(move_to_entity->m_module->m_alloc, move_to_entity->m_cfg_spring.m_cfg_base_distance);
        move_to_entity->m_cfg_spring.m_cfg_base_distance = NULL;
    }

    if (move_to_entity->m_cfg_spring.m_cfg_max_distance) {
        mem_free(move_to_entity->m_module->m_alloc, move_to_entity->m_cfg_spring.m_cfg_max_distance);
        move_to_entity->m_cfg_spring.m_cfg_max_distance = NULL;
    }
    
    if (move_to_entity->m_cfg_spring.m_cfg_K) {
        cpe_str_mem_dup(move_to_entity->m_module->m_alloc, move_to_entity->m_cfg_spring.m_cfg_K);
        move_to_entity->m_cfg_spring.m_cfg_K = NULL;
    }
}
    
int ui_sprite_chipmunk_move_to_entity_spring_load(ui_sprite_chipmunk_move_to_entity_t move_to_entity, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = move_to_entity->m_module;
    const char * str_value;

    move_to_entity->m_type = ui_sprite_chipmunk_move_to_entity_spring;
    
    if ((str_value = cfg_get_string(cfg, "base-distance", NULL))) {
        move_to_entity->m_cfg_spring.m_cfg_base_distance = cpe_str_mem_dup(move_to_entity->m_module->m_alloc, str_value);
        if (move_to_entity->m_cfg_spring.m_cfg_base_distance == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create move_to_entity action: base-distance %s load fail!",
                ui_sprite_chipmunk_module_name(module), str_value);
            return -1;
        }
    }

    if ((str_value = cfg_get_string(cfg, "max-distance", NULL))) {
        move_to_entity->m_cfg_spring.m_cfg_max_distance = cpe_str_mem_dup(move_to_entity->m_module->m_alloc, str_value);
        if (move_to_entity->m_cfg_spring.m_cfg_max_distance == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create move_to_entity action: max-distance %s load fail!",
                ui_sprite_chipmunk_module_name(module), str_value);
            return -1;
        }
    }

    if ((str_value = cfg_get_string(cfg, "K", NULL))) {
        move_to_entity->m_cfg_spring.m_cfg_K = cpe_str_mem_dup(move_to_entity->m_module->m_alloc, str_value);
        if (move_to_entity->m_cfg_spring.m_cfg_K == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create move_to_entity action: K %s load fail!",
                ui_sprite_chipmunk_module_name(module), str_value);
            return -1;
        }
    }

    return 0;
}

#ifdef __cplusplus
}
#endif
    
