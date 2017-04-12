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
#include "ui_sprite_chipmunk_with_attractor_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

cpVect ui_sprite_chipmunk_with_attractor_gravitation_work(
    ui_sprite_chipmunk_with_attractor_t with_attractor, ui_sprite_chipmunk_obj_body_t body)
{
    ui_vector_2 target_pos;
    float target_mass;
    float distance;
    float radians;
    float apply_force_value;

    target_pos = ui_sprite_chipmunk_obj_body_world_pos(body);
    target_mass = cpBodyGetMass(&body->m_body);
    
    /*计算目标速度 */
    radians = cpe_math_radians(target_pos.x, target_pos.y, with_attractor->m_self_pos.x, with_attractor->m_self_pos.y);
    distance = cpe_math_distance(target_pos.x, target_pos.y, with_attractor->m_self_pos.x, with_attractor->m_self_pos.y);
    distance = cpe_limit_in_range(distance, with_attractor->m_gravitation.m_min_distance, with_attractor->m_gravitation.m_max_distance);

    apply_force_value = with_attractor->m_gravitation.m_G * target_mass * with_attractor->m_self_mass / (distance * distance);

    return cpv(
        cpe_cos_radians(radians) * apply_force_value,
        cpe_sin_radians(radians) * apply_force_value);
}
    
int ui_sprite_chipmunk_with_attractor_gravitation_enter(
    ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_with_attractor_t with_attractor,
    ui_sprite_entity_t entity, ui_sprite_fsm_action_t fsm_action, float ptm)
{
    if (with_attractor->m_cfg_gravitation.m_cfg_min_distance == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with attractor: min-distance not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    else if (ui_sprite_fsm_action_check_calc_float(
                 &with_attractor->m_gravitation.m_min_distance, with_attractor->m_cfg_gravitation.m_cfg_min_distance, fsm_action, NULL, module->m_em)
             != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with attractor: calc min-distance from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_attractor->m_cfg_gravitation.m_cfg_min_distance);
        return -1;
    }
    with_attractor->m_gravitation.m_min_distance *= ptm;

    if (with_attractor->m_cfg_gravitation.m_cfg_max_distance == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with attractor: max-distance not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    else if (ui_sprite_fsm_action_check_calc_float(
                 &with_attractor->m_gravitation.m_max_distance, with_attractor->m_cfg_gravitation.m_cfg_max_distance, fsm_action, NULL, module->m_em)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with attractor: calc max-distance from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_attractor->m_cfg_gravitation.m_cfg_max_distance);
        return -1;
    }
    with_attractor->m_gravitation.m_max_distance *= ptm;

    if (with_attractor->m_cfg_gravitation.m_cfg_G == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with attractor: G not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    else if (ui_sprite_fsm_action_check_calc_float(
                 &with_attractor->m_gravitation.m_G,
                 with_attractor->m_cfg_gravitation.m_cfg_G, fsm_action, NULL, module->m_em)
             != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with attractor: calc G from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_attractor->m_cfg_gravitation.m_cfg_G);
        return -1;
    }
    with_attractor->m_gravitation.m_G *= ptm;
    
    return 0;
}

int ui_sprite_chipmunk_with_attractor_gravitation_copy(
    ui_sprite_chipmunk_with_attractor_t to_with_attractor, ui_sprite_chipmunk_with_attractor_t from_with_attractor)
{
    if (from_with_attractor->m_cfg_gravitation.m_cfg_min_distance) {
        to_with_attractor->m_cfg_gravitation.m_cfg_min_distance
            = cpe_str_mem_dup(to_with_attractor->m_module->m_alloc, from_with_attractor->m_cfg_gravitation.m_cfg_min_distance);
    }

    if (from_with_attractor->m_cfg_gravitation.m_cfg_max_distance) {
        to_with_attractor->m_cfg_gravitation.m_cfg_max_distance
            = cpe_str_mem_dup(to_with_attractor->m_module->m_alloc, from_with_attractor->m_cfg_gravitation.m_cfg_max_distance);
    }

    if (from_with_attractor->m_cfg_gravitation.m_cfg_G) {
        to_with_attractor->m_cfg_gravitation.m_cfg_G
            = cpe_str_mem_dup(to_with_attractor->m_module->m_alloc, from_with_attractor->m_cfg_gravitation.m_cfg_G);
    }

    return 0;
}

void ui_sprite_chipmunk_with_attractor_gravitation_free(ui_sprite_chipmunk_with_attractor_t with_attractor) {
    if (with_attractor->m_cfg_gravitation.m_cfg_min_distance) {
        mem_free(with_attractor->m_module->m_alloc, with_attractor->m_cfg_gravitation.m_cfg_min_distance);
        with_attractor->m_cfg_gravitation.m_cfg_min_distance = NULL;
    }

    if (with_attractor->m_cfg_gravitation.m_cfg_max_distance) {
        mem_free(with_attractor->m_module->m_alloc, with_attractor->m_cfg_gravitation.m_cfg_max_distance);
        with_attractor->m_cfg_gravitation.m_cfg_max_distance = NULL;
    }

    if (with_attractor->m_cfg_gravitation.m_cfg_G) {
        cpe_str_mem_dup(with_attractor->m_module->m_alloc, with_attractor->m_cfg_gravitation.m_cfg_G);
        with_attractor->m_cfg_gravitation.m_cfg_G = NULL;
    }
}
    
int ui_sprite_chipmunk_with_attractor_gravitation_load(ui_sprite_chipmunk_with_attractor_t with_attractor, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = with_attractor->m_module;
    const char * str_value;

    with_attractor->m_type = ui_sprite_chipmunk_with_attractor_gravitation;
    
    if ((str_value = cfg_get_string(cfg, "min-distance", NULL))) {
        with_attractor->m_cfg_gravitation.m_cfg_min_distance = cpe_str_mem_dup(with_attractor->m_module->m_alloc, str_value);
        if (with_attractor->m_cfg_gravitation.m_cfg_min_distance == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create with_attractor action: min-distance %s load fail!",
                ui_sprite_chipmunk_module_name(module), str_value);
            return -1;
        }
    }

    if ((str_value = cfg_get_string(cfg, "max-distance", NULL))) {
        with_attractor->m_cfg_gravitation.m_cfg_max_distance = cpe_str_mem_dup(with_attractor->m_module->m_alloc, str_value);
        if (with_attractor->m_cfg_gravitation.m_cfg_max_distance == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create with_attractor action: max-distance %s load fail!",
                ui_sprite_chipmunk_module_name(module), str_value);
            return -1;
        }
    }

    if ((str_value = cfg_get_string(cfg, "G", NULL))) {
        with_attractor->m_cfg_gravitation.m_cfg_G = cpe_str_mem_dup(with_attractor->m_module->m_alloc, str_value);
        if (with_attractor->m_cfg_gravitation.m_cfg_G == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create with_attractor action: G %s load fail!",
                ui_sprite_chipmunk_module_name(module), str_value);
            return -1;
        }
    }

    return 0;
}

#ifdef __cplusplus
}
#endif
    
