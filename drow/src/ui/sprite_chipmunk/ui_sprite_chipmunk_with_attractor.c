#include <assert.h>
#include <stdio.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_chipmunk_with_attractor_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "cpe/pal/pal_strings.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_with_attractor_t ui_sprite_chipmunk_with_attractor_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_WITH_ATTRACTOR_NAME);
    return fsm_action ? (ui_sprite_chipmunk_with_attractor_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_with_attractor_free(ui_sprite_chipmunk_with_attractor_t with_attractor) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(with_attractor);
    ui_sprite_fsm_action_free(fsm_action);
}

/*处理一个body */
static void ui_sprite_chipmunk_with_attractor_do_attrack_one(
    ui_sprite_chipmunk_with_attractor_t with_attractor, ui_sprite_chipmunk_obj_body_t body,
    ui_sprite_entity_t self_entity, ui_sprite_entity_t target_entity)
{
    cpVect f;
    cpVect add_f;

    if (self_entity == target_entity) return;

    if (cpBodyGetType(&body->m_body) != CP_BODY_TYPE_DYNAMIC) {
        if (ui_sprite_entity_debug(self_entity) >= 2 || ui_sprite_entity_debug(target_entity) >= 2) {
            CPE_INFO(
                with_attractor->m_module->m_em, "entity %d(%s): chipmunk with attractor: ignore entity %d(%s)",
                ui_sprite_entity_id(self_entity), ui_sprite_entity_name(self_entity),
                ui_sprite_entity_id(target_entity), ui_sprite_entity_name(target_entity));
        }
        return;
    }

    switch(with_attractor->m_type) {
    case ui_sprite_chipmunk_with_attractor_gravitation:
        add_f = ui_sprite_chipmunk_with_attractor_gravitation_work(with_attractor, body);
        break;
    case ui_sprite_chipmunk_with_attractor_spring:
        add_f = ui_sprite_chipmunk_with_attractor_spring_work(with_attractor, body);
        break;
    case ui_sprite_chipmunk_with_attractor_force:
        assert(0);
        return;
        break;
    default:
        assert(0);
        return;
    }

    if (with_attractor->m_damping != 1.0f) {
        float damping_radians = cpvtoangle(body->m_body.v);
        float damping_v = cpvlength(body->m_body.v) * (1.0f - with_attractor->m_damping);
        float damping_acc = damping_v / with_attractor->m_step_duration;
        float damping_f = damping_acc * cpBodyGetMass(&body->m_body);

        add_f.x -= damping_f * cpe_cos_radians(damping_radians);
        add_f.y -= damping_f * cpe_sin_radians(damping_radians);
    }
  
    f = body->m_body.f;
    f.x += add_f.x;
    f.y += add_f.y;
    cpBodySetForce(&body->m_body, f);

    if (ui_sprite_entity_debug(self_entity) >= 2 || ui_sprite_entity_debug(target_entity) >= 2) {
        ui_vector_2 target_pos = ui_sprite_chipmunk_obj_body_world_pos(body);

        CPE_INFO(
            with_attractor->m_module->m_em,
            "entity %d(%s): chipmunk with attractor: (%f,%f) <== (%f,%f): absorb entity %d(%s), add-force(%f,%f), final-force=(%f,%f)",
            ui_sprite_entity_id(self_entity), ui_sprite_entity_name(self_entity),
            with_attractor->m_self_pos.x, with_attractor->m_self_pos.y,
            target_pos.x, target_pos.y,
            ui_sprite_entity_id(target_entity), ui_sprite_entity_name(target_entity),
            add_f.x, add_f.y, f.x, f.y);
    }
}

static void ui_sprite_chipmunk_with_attractor_visit_body(ui_sprite_chipmunk_env_t env, ui_sprite_chipmunk_obj_body_t body, void * ctx) {
    ui_sprite_chipmunk_with_attractor_t with_attractor = (ui_sprite_chipmunk_with_attractor_t)ctx;

    if (with_attractor->m_obj_group) {
        ui_sprite_entity_t entity = (ui_sprite_entity_t)ui_sprite_component_from_data(body->m_obj);
        if (!ui_sprite_group_has_entity_r(with_attractor->m_obj_group, entity)) return;
    }
    
    if (with_attractor->m_cfg_cache) {
        /*需要捕捉，添加到捕捉的列表中 */
        if (!ui_sprite_chipmunk_obj_body_group_have_body(&with_attractor->m_body_group, body)) {
            ui_sprite_chipmunk_obj_body_group_binding_create(&with_attractor->m_body_group, body);
        }
    }
    else {
        /*不需要捕捉，直接处理所有body */
        ui_sprite_chipmunk_with_attractor_do_attrack_one(
            with_attractor, body,
            ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_from_data(with_attractor)),
            ui_sprite_component_entity(ui_sprite_component_from_data(body->m_obj)));
    }
}

static void ui_sprite_chipmunk_with_attractor_do_attact(plugin_chipmunk_env_t env, void * ctx, float delta) {
    ui_sprite_chipmunk_with_attractor_t with_attractor = (ui_sprite_chipmunk_with_attractor_t)ctx;
    ui_sprite_chipmunk_module_t module = with_attractor->m_module;
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(with_attractor);
    ui_sprite_entity_t self_entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_body_group_binding_t binding_body;

    if (ui_sprite_chipmunk_env_query_bodies_by_shape(
            (ui_sprite_chipmunk_env_t)with_attractor->m_env, self_entity,
            ui_sprite_chipmunk_with_attractor_visit_body, with_attractor,
            &with_attractor->m_query_shape, with_attractor->m_category, with_attractor->m_mask, with_attractor->m_group)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with attractor: query fail!",
            ui_sprite_entity_id(self_entity), ui_sprite_entity_name(self_entity));
        return;
    }

    if (with_attractor->m_cfg_cache) {
        /*需要捕捉，处理所有已经捕捉到的body */
        TAILQ_FOREACH(binding_body, &with_attractor->m_body_group.m_bodies, m_next_for_group) {
            ui_sprite_chipmunk_with_attractor_do_attrack_one(
                with_attractor, binding_body->m_body,
                self_entity,
                ui_sprite_component_entity(ui_sprite_component_from_data(binding_body->m_body->m_obj)));
        }
    }
}

static int ui_sprite_chipmunk_with_attractor_update_target_pos_and_mass(
    ui_sprite_chipmunk_with_attractor_t with_attractor, ui_sprite_entity_t self_entity)
{
    ui_sprite_2d_transform_t transform;
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    ui_sprite_chipmunk_obj_body_t main_body;
    
    transform = ui_sprite_2d_transform_find(self_entity);
    if (transform == NULL) {
        CPE_ERROR(
            with_attractor->m_module->m_em, "entity %d(%s): chipmunk with attractor: entity no transform!",
            ui_sprite_entity_id(self_entity), ui_sprite_entity_name(self_entity));
        return -1;
    }

    with_attractor->m_self_pos = ui_sprite_2d_transform_origin_pos(transform);

    chipmunk_obj = ui_sprite_chipmunk_obj_find(self_entity);
    main_body = chipmunk_obj ? chipmunk_obj->m_main_body : NULL;

    if (main_body && cpBodyGetType(&main_body->m_body) == CP_BODY_TYPE_DYNAMIC) {
        with_attractor->m_self_mass = cpBodyGetMass(&main_body->m_body);
    }
    else {
        with_attractor->m_self_mass = 1.0f;
    }
    
    return 0;
}
    
static int ui_sprite_chipmunk_with_attractor_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_attractor_t with_attractor = (ui_sprite_chipmunk_with_attractor_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_chipmunk_env_t env;
    const char * str_value;
    float ptm;

    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(module->m_em, "%s: chipmunk with attractor: env not exist!", ui_sprite_chipmunk_module_name(module));
        return -1;
    }
    ptm = plugin_chipmunk_env_ptm(env->m_env);

    with_attractor->m_step_duration = plugin_chipmunk_env_step_duration(env->m_env);

    if (with_attractor->m_cfg_obj_group) {
        with_attractor->m_obj_group = ui_sprite_group_find_by_name(world, with_attractor->m_cfg_obj_group);
        if (with_attractor->m_obj_group == NULL) {
            CPE_ERROR(
                module->m_em, "%s: chipmunk with attractor: obj group %s not exist!",
                ui_sprite_chipmunk_module_name(module), with_attractor->m_cfg_obj_group);
            return -1;
        }
    }
    else {
        with_attractor->m_obj_group = NULL;
    }

    switch(with_attractor->m_type) {
    case ui_sprite_chipmunk_with_attractor_gravitation:
        if (ui_sprite_chipmunk_with_attractor_gravitation_enter(module, with_attractor, entity, fsm_action, ptm) != 0) {
            return -1;
        }
        break;
    case ui_sprite_chipmunk_with_attractor_spring:
        if (ui_sprite_chipmunk_with_attractor_spring_enter(module, with_attractor, entity, fsm_action, ptm) != 0) {
            return -1;
        }
        break;
    case ui_sprite_chipmunk_with_attractor_force:
        break;
    default:
        CPE_ERROR(module->m_em, "%s: chipmunk with attractor: attractor type unknown!", ui_sprite_chipmunk_module_name(module));
        return -1;
    }

    if (with_attractor->m_cfg_mask) {
        str_value = ui_sprite_fsm_action_check_calc_str(&module->m_dump_buffer, with_attractor->m_cfg_mask, fsm_action, NULL, module->m_em);
        if (str_value == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with attractor: calc mask from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_attractor->m_cfg_mask);
            return -1;
        }
        if (plugin_chipmunk_env_masks(env->m_env, &with_attractor->m_mask, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with attractor: calc mask from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_value);
            return -1;
        }
    }
    else {
        with_attractor->m_mask = 0xFFFF;
    }

    if (with_attractor->m_cfg_category) {
        str_value = ui_sprite_fsm_action_check_calc_str(&module->m_dump_buffer, with_attractor->m_cfg_category, fsm_action, NULL, module->m_em);
        if (str_value == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with attractor: calc category from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_attractor->m_cfg_category);
            return -1;
        }
        if (plugin_chipmunk_env_masks(env->m_env, &with_attractor->m_category, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with attractor: calc category from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_value);
            return -1;
        }
    }
    else {
        with_attractor->m_category = 0xFFFF;
    }

    if (with_attractor->m_cfg_group) {
        if (ui_sprite_fsm_action_check_calc_uint32(&with_attractor->m_group, with_attractor->m_cfg_group, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with attractor: calc group from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_attractor->m_cfg_group);
            return -1;
        }
    }
    else {
        with_attractor->m_group = 0;
    }

    if (with_attractor->m_cfg_damping) {
        if (ui_sprite_fsm_action_check_calc_float(&with_attractor->m_damping, with_attractor->m_cfg_damping, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with attractor: calc damping from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_attractor->m_cfg_damping);
            return -1;
        }
    }
    else {
        with_attractor->m_damping = 0.0f;
    }

    str_value = ui_sprite_fsm_action_check_calc_str(&module->m_dump_buffer, with_attractor->m_cfg_shape, fsm_action, NULL, module->m_em);
    if (str_value == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with attractor: calc shape from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_attractor->m_cfg_shape);
        return -1;
    }
    if (ui_sprite_chipmunk_load_shape_from_str(module, &with_attractor->m_query_shape, str_value) != 0) return -1;

    if (ui_sprite_chipmunk_with_attractor_update_target_pos_and_mass(with_attractor, entity) != 0) {
        return -1;
    }
    
    assert(with_attractor->m_updator == NULL);
    with_attractor->m_updator =
        plugin_chipmunk_env_updator_create(env->m_env, ui_sprite_chipmunk_with_attractor_do_attact, with_attractor);
    if (with_attractor->m_updator == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with attractor: create updator fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    with_attractor->m_env = (plugin_chipmunk_env_t)env;

    ui_sprite_fsm_action_start_update(fsm_action);
    
    return 0;
}

static void ui_sprite_chipmunk_with_attractor_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_chipmunk_with_attractor_t with_attractor = (ui_sprite_chipmunk_with_attractor_t)ui_sprite_fsm_action_data(fsm_action);

    if (ui_sprite_chipmunk_with_attractor_update_target_pos_and_mass(with_attractor, ui_sprite_fsm_action_to_entity(fsm_action)) != 0) {
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }
}
    
static void ui_sprite_chipmunk_with_attractor_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_attractor_t with_attractor = (ui_sprite_chipmunk_with_attractor_t)ui_sprite_fsm_action_data(fsm_action);

    ui_sprite_chipmunk_obj_body_group_clear(&with_attractor->m_body_group);
    
    assert(with_attractor->m_updator);
    plugin_chipmunk_env_updator_free(with_attractor->m_updator);
    with_attractor->m_updator = NULL;

    with_attractor->m_env = NULL;
}

static int ui_sprite_chipmunk_with_attractor_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_attractor_t with_attractor = (ui_sprite_chipmunk_with_attractor_t)ui_sprite_fsm_action_data(fsm_action);
    bzero(with_attractor, sizeof(*with_attractor));
    with_attractor->m_module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_obj_body_group_init(&with_attractor->m_body_group);
    return 0;
}

static void ui_sprite_chipmunk_with_attractor_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_attractor_t with_attractor = (ui_sprite_chipmunk_with_attractor_t)ui_sprite_fsm_action_data(fsm_action);
    
    assert(with_attractor->m_updator == NULL);

    ui_sprite_chipmunk_obj_body_group_fini(&with_attractor->m_body_group);

    switch(with_attractor->m_type) {
    case ui_sprite_chipmunk_with_attractor_gravitation:
        ui_sprite_chipmunk_with_attractor_gravitation_free(with_attractor);
        break;
    case ui_sprite_chipmunk_with_attractor_spring:
        ui_sprite_chipmunk_with_attractor_spring_free(with_attractor);
        break;
    case ui_sprite_chipmunk_with_attractor_force:
        break;
    default:
        break;
    }

    if (with_attractor->m_cfg_shape) {
        mem_free(module->m_alloc, with_attractor->m_cfg_shape);
        with_attractor->m_cfg_shape = NULL;
    }

    if (with_attractor->m_cfg_mask) {
        mem_free(module->m_alloc, with_attractor->m_cfg_mask);
        with_attractor->m_cfg_mask = NULL;
    }

    if (with_attractor->m_cfg_category) {
        mem_free(module->m_alloc, with_attractor->m_cfg_category);
        with_attractor->m_cfg_category = NULL;
    }
    
    if (with_attractor->m_cfg_group) {
        mem_free(module->m_alloc, with_attractor->m_cfg_group);
        with_attractor->m_cfg_group = NULL;
    }

    if (with_attractor->m_cfg_damping) {
        mem_free(module->m_alloc, with_attractor->m_cfg_damping);
        with_attractor->m_cfg_damping = NULL;
    }

    if (with_attractor->m_cfg_obj_group) {
        mem_free(module->m_alloc, with_attractor->m_cfg_obj_group);
        with_attractor->m_cfg_obj_group = NULL;
    }
}

static int ui_sprite_chipmunk_with_attractor_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_chipmunk_with_attractor_t to_with_attractor = (ui_sprite_chipmunk_with_attractor_t)ui_sprite_fsm_action_data(to);
    ui_sprite_chipmunk_with_attractor_t from_with_attractor = (ui_sprite_chipmunk_with_attractor_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_chipmunk_with_attractor_init(to, ctx)) return -1;

    to_with_attractor->m_cfg_cache = from_with_attractor->m_cfg_cache;

    if (from_with_attractor->m_cfg_shape) {
        to_with_attractor->m_cfg_shape = cpe_str_mem_dup(to_with_attractor->m_module->m_alloc, from_with_attractor->m_cfg_shape);
    }

    if (from_with_attractor->m_cfg_mask) {
        to_with_attractor->m_cfg_mask = cpe_str_mem_dup(to_with_attractor->m_module->m_alloc, from_with_attractor->m_cfg_mask);
    }

    if (from_with_attractor->m_cfg_category) {
        to_with_attractor->m_cfg_category = cpe_str_mem_dup(to_with_attractor->m_module->m_alloc, from_with_attractor->m_cfg_category);
    }
    
    if (from_with_attractor->m_cfg_group) {
        to_with_attractor->m_cfg_group = cpe_str_mem_dup(to_with_attractor->m_module->m_alloc, from_with_attractor->m_cfg_group);
    }

    if (from_with_attractor->m_cfg_damping) {
        to_with_attractor->m_cfg_damping = cpe_str_mem_dup(to_with_attractor->m_module->m_alloc, from_with_attractor->m_cfg_damping);
    }

    if (from_with_attractor->m_cfg_obj_group) {
        to_with_attractor->m_cfg_obj_group = cpe_str_mem_dup(to_with_attractor->m_module->m_alloc, from_with_attractor->m_cfg_obj_group);
    }
    
    switch(from_with_attractor->m_type) {
    case ui_sprite_chipmunk_with_attractor_gravitation:
        return ui_sprite_chipmunk_with_attractor_gravitation_copy(to_with_attractor, from_with_attractor);
    case ui_sprite_chipmunk_with_attractor_spring:
        return ui_sprite_chipmunk_with_attractor_spring_copy(to_with_attractor, from_with_attractor);
    case ui_sprite_chipmunk_with_attractor_force:
        break;
    default:
        break;
    }
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_chipmunk_with_attractor_load(
    void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg)
{
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_attractor_t with_attractor = (ui_sprite_chipmunk_with_attractor_t)ui_sprite_chipmunk_with_attractor_create(fsm_state, name);
    cfg_t cfg_value;
    const char * str_value;
    
    if (with_attractor == NULL) {
        CPE_ERROR(module->m_em, "%s: create with_attractor action: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    if (cfg_try_get_uint8(cfg, "cache", &with_attractor->m_cfg_cache) != 0) {
        CPE_ERROR(module->m_em, "%s: create with_attractor action: cache config load fail!", ui_sprite_chipmunk_module_name(module));
        ui_sprite_chipmunk_with_attractor_free(with_attractor);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "shape", NULL))) {
        if (with_attractor->m_cfg_shape) {
            mem_free(with_attractor->m_module->m_alloc, (void*)with_attractor->m_cfg_shape);
        }

        with_attractor->m_cfg_shape = cpe_str_mem_dup(with_attractor->m_module->m_alloc, str_value);
        if (with_attractor->m_cfg_shape == NULL) {
            CPE_ERROR(module->m_em, "%s: create with_attractor action: shape %s load fail!", ui_sprite_chipmunk_module_name(module), str_value);
            ui_sprite_chipmunk_with_attractor_free(with_attractor);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "mask", NULL))) {
        if (with_attractor->m_cfg_mask) {
            mem_free(with_attractor->m_module->m_alloc, (void*)with_attractor->m_cfg_mask);
        }

        with_attractor->m_cfg_mask = cpe_str_mem_dup(with_attractor->m_module->m_alloc, str_value);
        if (with_attractor->m_cfg_mask == NULL) {
            CPE_ERROR(module->m_em, "%s: create with_attractor action: mask %s load fail!", ui_sprite_chipmunk_module_name(module), str_value);
            ui_sprite_chipmunk_with_attractor_free(with_attractor);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "category", NULL))) {
        if (with_attractor->m_cfg_category) {
            mem_free(with_attractor->m_module->m_alloc, (void*)with_attractor->m_cfg_category);
        }

        with_attractor->m_cfg_category = cpe_str_mem_dup(with_attractor->m_module->m_alloc, str_value);
        if (with_attractor->m_cfg_category == NULL) {
            CPE_ERROR(module->m_em, "%s: create with_attractor action: category %s load fail!", ui_sprite_chipmunk_module_name(module), str_value);
            ui_sprite_chipmunk_with_attractor_free(with_attractor);
            return NULL;
        }
    }
    
    if ((str_value = cfg_get_string(cfg, "group", NULL))) {
        if (with_attractor->m_cfg_group) {
            mem_free(with_attractor->m_module->m_alloc, (void*)with_attractor->m_cfg_group);
        }

        with_attractor->m_cfg_group = cpe_str_mem_dup(with_attractor->m_module->m_alloc, str_value);
        if (with_attractor->m_cfg_group == NULL) {
            CPE_ERROR(module->m_em, "%s: create with_attractor action: group %s load fail!", ui_sprite_chipmunk_module_name(module), str_value);
            ui_sprite_chipmunk_with_attractor_free(with_attractor);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "damping", NULL))) {
        if (with_attractor->m_cfg_damping) {
            mem_free(with_attractor->m_module->m_alloc, (void*)with_attractor->m_cfg_damping);
        }

        with_attractor->m_cfg_damping = cpe_str_mem_dup(with_attractor->m_module->m_alloc, str_value);
        if (with_attractor->m_cfg_damping == NULL) {
            CPE_ERROR(module->m_em, "%s: create with_attractor action: damping %s load fail!", ui_sprite_chipmunk_module_name(module), str_value);
            ui_sprite_chipmunk_with_attractor_free(with_attractor);
            return NULL;
        }
    }
    
    if ((str_value = cfg_get_string(cfg, "obj-group", NULL))) {
        if (with_attractor->m_cfg_obj_group) {
            mem_free(with_attractor->m_module->m_alloc, (void*)with_attractor->m_cfg_obj_group);
        }

        with_attractor->m_cfg_obj_group = cpe_str_mem_dup(with_attractor->m_module->m_alloc, str_value);
        if (with_attractor->m_cfg_obj_group == NULL) {
            CPE_ERROR(module->m_em, "%s: create with_attractor action: obj-group %s load fail!", ui_sprite_chipmunk_module_name(module), str_value);
            ui_sprite_chipmunk_with_attractor_free(with_attractor);
            return NULL;
        }
    }
    
    if ((cfg_value = cfg_find_cfg(cfg, "gravitation"))) {
        if (ui_sprite_chipmunk_with_attractor_gravitation_load(with_attractor, cfg_value) != 0) {
            ui_sprite_chipmunk_with_attractor_free(with_attractor);
            return NULL;
        }
    }
    else if ((cfg_value = cfg_find_cfg(cfg, "spring"))) {
        if (ui_sprite_chipmunk_with_attractor_spring_load(with_attractor, cfg_value) != 0) {
            ui_sprite_chipmunk_with_attractor_free(with_attractor);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create with_attractor action: unknown calc type!", ui_sprite_chipmunk_module_name(module));
        ui_sprite_chipmunk_with_attractor_free(with_attractor);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(with_attractor);
}

    
int ui_sprite_chipmunk_with_attractor_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_ATTRACTOR_NAME, sizeof(struct ui_sprite_chipmunk_with_attractor));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk with attractor register: meta create fail",
            ui_sprite_chipmunk_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_with_attractor_enter, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_chipmunk_with_attractor_update, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_with_attractor_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_with_attractor_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_with_attractor_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_with_attractor_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_WITH_ATTRACTOR_NAME, ui_sprite_chipmunk_with_attractor_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_with_attractor_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_ATTRACTOR_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_WITH_ATTRACTOR_NAME);
}

const char * UI_SPRITE_CHIPMUNK_WITH_ATTRACTOR_NAME = "chipmunk-with-attractor";


#ifdef __cplusplus
}
#endif
    
