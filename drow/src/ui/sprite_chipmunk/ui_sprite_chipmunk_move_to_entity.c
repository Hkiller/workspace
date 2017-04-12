#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/chipmunk/plugin_chipmunk_module.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_chipmunk_move_to_entity_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_move_to_entity_t ui_sprite_chipmunk_move_to_entity_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_MOVE_TO_ENTITY_NAME);
    return fsm_action ? (ui_sprite_chipmunk_move_to_entity_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_move_to_entity_free(ui_sprite_chipmunk_move_to_entity_t move_to_entity) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(move_to_entity);
    ui_sprite_fsm_action_free(fsm_action);
}

static void ui_sprite_chipmunk_move_to_entity_do_attact(plugin_chipmunk_env_t env, void * ctx, float delta) {
    ui_sprite_chipmunk_move_to_entity_t move_to_entity = (ui_sprite_chipmunk_move_to_entity_t)ctx;
    ui_sprite_chipmunk_module_t module = move_to_entity->m_module;
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(move_to_entity);
    ui_sprite_entity_t self_entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t self_obj;
    ui_sprite_chipmunk_obj_body_t self_body;
    ui_vector_2 self_pos;
    ui_sprite_entity_t target_entity;
    ui_sprite_chipmunk_obj_t target_obj;
    ui_sprite_chipmunk_obj_body_t target_body;
    ui_vector_2 target_pos;
    cpVect f;
    cpVect add_f;

    target_entity = ui_sprite_entity_find_by_id(ui_sprite_entity_world(self_entity), move_to_entity->m_target_entity);
    if (target_entity == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk move to entity: target entity %d not exist",
            ui_sprite_entity_id(self_entity), ui_sprite_entity_name(self_entity),
            move_to_entity->m_target_entity);
        return;
    }
    
    if (self_entity == target_entity) return;

    self_obj = ui_sprite_chipmunk_obj_find(self_entity);
    if (self_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk move to entity: self entity is not chipmunk obj",
            ui_sprite_entity_id(self_entity), ui_sprite_entity_name(self_entity));
        return;
    }

    self_body = ui_sprite_chipmunk_obj_main_body(self_obj);
    if (self_body == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk move to entity: no main body",
            ui_sprite_entity_id(self_entity), ui_sprite_entity_name(self_entity));
        return;
    }
    self_pos = ui_sprite_chipmunk_obj_body_world_pos(self_body);

    if (cpBodyGetType(&self_body->m_body) != CP_BODY_TYPE_DYNAMIC) {
        CPE_ERROR(
            move_to_entity->m_module->m_em, "entity %d(%s): chipmunk move to entity: ignore entity %d(%s)",
            ui_sprite_entity_id(self_entity), ui_sprite_entity_name(self_entity),
            ui_sprite_entity_id(target_entity), ui_sprite_entity_name(target_entity));
        return;
    }

    if (self_entity == target_entity) return;

    target_obj = ui_sprite_chipmunk_obj_find(target_entity);
    target_body = target_obj ? ui_sprite_chipmunk_obj_main_body(target_obj) : NULL;
    if (target_body) {
        target_pos = ui_sprite_chipmunk_obj_body_world_pos(target_body);
    }
    else {
        ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(target_entity);
        if (transform) {
            target_pos = ui_sprite_2d_transform_origin_pos(transform);
        }
        else {
            CPE_ERROR(
                move_to_entity->m_module->m_em, "entity %d(%s): chipmunk move to entity: target entity %d(%s) no transform",
                ui_sprite_entity_id(self_entity), ui_sprite_entity_name(self_entity),
                ui_sprite_entity_id(target_entity), ui_sprite_entity_name(target_entity));
            return;
        }
    }

    switch(move_to_entity->m_type) {
    case ui_sprite_chipmunk_move_to_entity_gravitation:
        add_f = ui_sprite_chipmunk_move_to_entity_gravitation_work(
            move_to_entity, target_entity, target_body, &target_pos, self_entity, self_body, &self_pos);
        break;
    case ui_sprite_chipmunk_move_to_entity_spring:
        add_f = ui_sprite_chipmunk_move_to_entity_spring_work(
            move_to_entity, target_entity, target_body, &target_pos, self_entity, self_body, &self_pos);
        break;
    case ui_sprite_chipmunk_move_to_entity_move:
        add_f = ui_sprite_chipmunk_move_to_entity_move_work(
            move_to_entity, target_entity, target_body, &target_pos, self_entity, self_body, &self_pos);
        break;
    default:
        assert(0);
        return;
    }
    cpe_assert_float_sane(add_f.x);
    cpe_assert_float_sane(add_f.y);
    
    cpe_assert_float_sane(move_to_entity->m_damping);
    if (move_to_entity->m_damping != 1.0f) {
        float damping_radians = cpvtoangle(self_body->m_body.v);
        float damping_v = cpvlength(self_body->m_body.v) * (1.0f - move_to_entity->m_damping);
        float damping_acc = damping_v / move_to_entity->m_step_duration;
        float damping_f = damping_acc * cpBodyGetMass(&self_body->m_body);

        cpe_assert_float_sane(damping_radians);
        cpe_assert_float_sane(damping_v);
        cpe_assert_float_sane(damping_acc);
        cpe_assert_float_sane(damping_f);

        add_f.x -= damping_f * cpe_cos_radians(damping_radians);
        add_f.y -= damping_f * cpe_sin_radians(damping_radians);
        cpe_assert_float_sane(add_f.x);
        cpe_assert_float_sane(add_f.y);
    }
  
    f = self_body->m_body.f;
    cpe_assert_float_sane(f.x);
    cpe_assert_float_sane(f.y);
    f.x += add_f.x;
    f.y += add_f.y;
    cpBodySetForce(&self_body->m_body, f);

    if (ui_sprite_entity_debug(self_entity) >= 2 || ui_sprite_entity_debug(target_entity) >= 2) {
        CPE_INFO(
            move_to_entity->m_module->m_em,
            "entity %d(%s): chipmunk move to entity: (%f,%f) <== (%f,%f): move to entity %d(%s), add-force(%f,%f), final-force=(%f,%f)",
            ui_sprite_entity_id(self_entity), ui_sprite_entity_name(self_entity),
            self_pos.x, self_pos.y,
            target_pos.x, target_pos.y,
            ui_sprite_entity_id(target_entity), ui_sprite_entity_name(target_entity),
            add_f.x, add_f.y, f.x, f.y);
    }
}

static int ui_sprite_chipmunk_move_to_entity_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_move_to_entity_t move_to_entity = (ui_sprite_chipmunk_move_to_entity_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_chipmunk_env_t env;
    float ptm;

    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(module->m_em, "%s: chipmunk move to entity: env not exist!", ui_sprite_chipmunk_module_name(module));
        return -1;
    }
    ptm = plugin_chipmunk_env_ptm(env->m_env);

    move_to_entity->m_step_duration = plugin_chipmunk_env_step_duration(env->m_env);

    if (move_to_entity->m_cfg_target_entity == NULL) {
        CPE_ERROR(module->m_em, "%s: chipmunk move to entity: target entity not configured!", ui_sprite_chipmunk_module_name(module));
        return -1;
    }
    if (ui_sprite_fsm_action_check_calc_uint32(
            &move_to_entity->m_target_entity, move_to_entity->m_cfg_target_entity, fsm_action, NULL, module->m_em)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: chipmunk move to entity: calc target entity from %s fail!",
            ui_sprite_chipmunk_module_name(module), move_to_entity->m_cfg_target_entity);
        return -1;
    }

    switch(move_to_entity->m_type) {
    case ui_sprite_chipmunk_move_to_entity_gravitation:
        if (ui_sprite_chipmunk_move_to_entity_gravitation_enter(module, move_to_entity, entity, fsm_action, ptm) != 0) {
            return -1;
        }
        break;
    case ui_sprite_chipmunk_move_to_entity_spring:
        if (ui_sprite_chipmunk_move_to_entity_spring_enter(module, move_to_entity, entity, fsm_action, ptm) != 0) {
            return -1;
        }
        break;
    case ui_sprite_chipmunk_move_to_entity_move:
        if (ui_sprite_chipmunk_move_to_entity_move_enter(module, move_to_entity, entity, fsm_action, ptm) != 0) {
            return -1;
        }
        break;
    default:
        CPE_ERROR(module->m_em, "%s: chipmunk move to entity: move type unknown!", ui_sprite_chipmunk_module_name(module));
        return -1;
    }

    if (move_to_entity->m_cfg_damping) {
        if (ui_sprite_fsm_action_check_calc_float(&move_to_entity->m_damping, move_to_entity->m_cfg_damping, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk move to entity: calc damping from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_to_entity->m_cfg_damping);
            return -1;
        }

        if (move_to_entity->m_damping < 0.0f || move_to_entity->m_damping > 1.0f) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk move to entity: damping %f(%s) fail out of range!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), move_to_entity->m_damping, move_to_entity->m_cfg_damping);
            return -1;
        }

        move_to_entity->m_damping = (1.0 - (1.0 - move_to_entity->m_damping) * move_to_entity->m_step_duration); 
    }
    else {
        move_to_entity->m_damping = 1.0f;
    }

    assert(move_to_entity->m_updator == NULL);
    move_to_entity->m_updator =
        plugin_chipmunk_env_updator_create(env->m_env, ui_sprite_chipmunk_move_to_entity_do_attact, move_to_entity);
    if (move_to_entity->m_updator == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk move to entity: create updator fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    move_to_entity->m_env = (plugin_chipmunk_env_t)env;

    if (ui_sprite_fsm_action_life_circle(fsm_action) == ui_sprite_fsm_action_life_circle_working) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }
    
    return 0;
}

static void ui_sprite_chipmunk_move_to_entity_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_move_to_entity_t move_to_entity = (ui_sprite_chipmunk_move_to_entity_t)ui_sprite_fsm_action_data(fsm_action);
    assert(move_to_entity->m_updator);
    
    assert(move_to_entity->m_updator);
    plugin_chipmunk_env_updator_free(move_to_entity->m_updator);
    move_to_entity->m_updator = NULL;

    move_to_entity->m_env = NULL;
}

static void ui_sprite_chipmunk_move_to_entity_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
	ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_move_to_entity_t move_to_entity = (ui_sprite_chipmunk_move_to_entity_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t self_entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t self_obj;
    ui_sprite_chipmunk_obj_body_t self_body;
    ui_vector_2 self_pos;
    ui_sprite_entity_t target_entity;
    ui_vector_2 target_pos;

    target_entity = ui_sprite_entity_find_by_id(ui_sprite_entity_world(self_entity), move_to_entity->m_target_entity);
    if (target_entity == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk move to entity: target entity %d not exist",
            ui_sprite_entity_id(self_entity), ui_sprite_entity_name(self_entity),
            move_to_entity->m_target_entity);
        return;
    }
    
    if (self_entity == target_entity) {
		ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    self_obj = ui_sprite_chipmunk_obj_find(self_entity);
    if (self_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk move to entity: self entity is not chipmunk obj",
            ui_sprite_entity_id(self_entity), ui_sprite_entity_name(self_entity));
        return;
    }

    self_body = ui_sprite_chipmunk_obj_main_body(self_obj);
    if (self_body == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk move to entity: no main body",
            ui_sprite_entity_id(self_entity), ui_sprite_entity_name(self_entity));
        return;
    }
    self_pos = ui_sprite_chipmunk_obj_body_world_pos(self_body);

    if (ui_vector_2_cmp(&self_pos, &target_pos) == 0) {
		ui_sprite_fsm_action_stop_update(fsm_action);
    }
}
    
static int ui_sprite_chipmunk_move_to_entity_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_move_to_entity_t move_to_entity = (ui_sprite_chipmunk_move_to_entity_t)ui_sprite_fsm_action_data(fsm_action);
    bzero(move_to_entity, sizeof(*move_to_entity));
    move_to_entity->m_module = (ui_sprite_chipmunk_module_t)ctx;
    return 0;
}

static void ui_sprite_chipmunk_move_to_entity_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_move_to_entity_t move_to_entity = (ui_sprite_chipmunk_move_to_entity_t)ui_sprite_fsm_action_data(fsm_action);
    
    assert(move_to_entity->m_updator == NULL);

    switch(move_to_entity->m_type) {
    case ui_sprite_chipmunk_move_to_entity_gravitation:
        ui_sprite_chipmunk_move_to_entity_gravitation_free(move_to_entity);
        break;
    case ui_sprite_chipmunk_move_to_entity_spring:
        ui_sprite_chipmunk_move_to_entity_spring_free(move_to_entity);
        break;
    case ui_sprite_chipmunk_move_to_entity_move:
        ui_sprite_chipmunk_move_to_entity_move_free(move_to_entity);
        break;
    default:
        break;
    }

    if (move_to_entity->m_cfg_damping) {
        mem_free(module->m_alloc, move_to_entity->m_cfg_damping);
        move_to_entity->m_cfg_damping = NULL;
    }

    if (move_to_entity->m_cfg_target_entity) {
        mem_free(module->m_alloc, (void*)move_to_entity->m_cfg_target_entity);
        move_to_entity->m_cfg_target_entity = NULL;
    }
}

static int ui_sprite_chipmunk_move_to_entity_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;    
    ui_sprite_chipmunk_move_to_entity_t to_move_to_entity = (ui_sprite_chipmunk_move_to_entity_t)ui_sprite_fsm_action_data(to);
    ui_sprite_chipmunk_move_to_entity_t from_move_to_entity = (ui_sprite_chipmunk_move_to_entity_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_chipmunk_move_to_entity_init(to, ctx)) return -1;

    to_move_to_entity->m_type = from_move_to_entity->m_type;
    
    if (from_move_to_entity->m_cfg_damping) {
        to_move_to_entity->m_cfg_damping = cpe_str_mem_dup(to_move_to_entity->m_module->m_alloc, from_move_to_entity->m_cfg_damping);
    }

    if (from_move_to_entity->m_cfg_target_entity) {
        to_move_to_entity->m_cfg_target_entity = cpe_str_mem_dup(module->m_alloc, from_move_to_entity->m_cfg_target_entity);
    }

    switch(from_move_to_entity->m_type) {
    case ui_sprite_chipmunk_move_to_entity_gravitation:
        return ui_sprite_chipmunk_move_to_entity_gravitation_copy(to_move_to_entity, from_move_to_entity);
    case ui_sprite_chipmunk_move_to_entity_spring:
        return ui_sprite_chipmunk_move_to_entity_spring_copy(to_move_to_entity, from_move_to_entity);
    case ui_sprite_chipmunk_move_to_entity_move:
        return ui_sprite_chipmunk_move_to_entity_move_copy(to_move_to_entity, from_move_to_entity);
    default:
        break;
    }
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_chipmunk_move_to_entity_load(
    void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg)
{
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_move_to_entity_t move_to_entity = (ui_sprite_chipmunk_move_to_entity_t)ui_sprite_chipmunk_move_to_entity_create(fsm_state, name);
    cfg_t cfg_value;
    const char * str_value;
    
    if (move_to_entity == NULL) {
        CPE_ERROR(module->m_em, "%s: create move_to_entity action: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "damping", NULL))) {
        if (move_to_entity->m_cfg_damping) {
            mem_free(move_to_entity->m_module->m_alloc, (void*)move_to_entity->m_cfg_damping);
        }

        move_to_entity->m_cfg_damping = cpe_str_mem_dup(move_to_entity->m_module->m_alloc, str_value);
        if (move_to_entity->m_cfg_damping == NULL) {
            CPE_ERROR(module->m_em, "%s: create move_to_entity action: damping %s load fail!", ui_sprite_chipmunk_module_name(module), str_value);
            ui_sprite_chipmunk_move_to_entity_free(move_to_entity);
            return NULL;
        }
    }
    
    if ((cfg_value = cfg_find_cfg(cfg, "gravitation"))) {
        if (ui_sprite_chipmunk_move_to_entity_gravitation_load(move_to_entity, cfg_value) != 0) {
            ui_sprite_chipmunk_move_to_entity_free(move_to_entity);
            return NULL;
        }
    }
    else if ((cfg_value = cfg_find_cfg(cfg, "spring"))) {
        if (ui_sprite_chipmunk_move_to_entity_spring_load(move_to_entity, cfg_value) != 0) {
            ui_sprite_chipmunk_move_to_entity_free(move_to_entity);
            return NULL;
        }
    }
    else if ((cfg_value = cfg_find_cfg(cfg, "move"))) {
        if (ui_sprite_chipmunk_move_to_entity_move_load(move_to_entity, cfg_value) != 0) {
            ui_sprite_chipmunk_move_to_entity_free(move_to_entity);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create move_to_entity action: unknown calc type!", ui_sprite_chipmunk_module_name(module));
        ui_sprite_chipmunk_move_to_entity_free(move_to_entity);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "target-entity", NULL))) {
        if (move_to_entity->m_cfg_target_entity) {
            mem_free(module->m_alloc, (void*)move_to_entity->m_cfg_target_entity);
        }

        move_to_entity->m_cfg_target_entity = cpe_str_mem_dup(module->m_alloc, str_value);
    }
    else {
        CPE_ERROR(module->m_em, "%s: create move_to_entity action: target-entity not configured!", ui_sprite_chipmunk_module_name(module));
        ui_sprite_chipmunk_move_to_entity_free(move_to_entity);
        return NULL;
    }
    
    return ui_sprite_fsm_action_from_data(move_to_entity);
}

int ui_sprite_chipmunk_move_to_entity_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_MOVE_TO_ENTITY_NAME, sizeof(struct ui_sprite_chipmunk_move_to_entity));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk move to target register: meta create fail",
            ui_sprite_chipmunk_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_move_to_entity_enter, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_chipmunk_move_to_entity_update, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_move_to_entity_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_move_to_entity_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_move_to_entity_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_move_to_entity_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_MOVE_TO_ENTITY_NAME, ui_sprite_chipmunk_move_to_entity_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_move_to_entity_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_MOVE_TO_ENTITY_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_MOVE_TO_ENTITY_NAME);
}

const char * UI_SPRITE_CHIPMUNK_MOVE_TO_ENTITY_NAME = "chipmunk-move-to-entity";

#ifdef __cplusplus
}
#endif
    
