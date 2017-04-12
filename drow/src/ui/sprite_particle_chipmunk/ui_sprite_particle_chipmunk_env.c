#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "plugin/particle/plugin_particle_obj_emitter.h"
#include "plugin/particle/plugin_particle_obj_particle.h"
#include "plugin/particle/plugin_particle_obj_plugin_data.h"
#include "plugin/particle/plugin_particle_data.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_module.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_env.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_obj.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_obj_body.h"
#include "ui_sprite_particle_chipmunk_env_i.h"
#include "ui_sprite_particle_chipmunk_body_i.h"
#include "ui_sprite_particle_chipmunk_with_collision_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static void ui_sprite_particle_chipmunk_env_update(ui_sprite_world_t world, void * ctx, float delta_s);
static cpBool ui_sprite_particle_chipmunk_env_begin_collision(cpArbiter *arb, cpSpace *space, cpDataPointer userData);

static void ui_sprite_particle_chipmunk_env_clear(ui_sprite_world_res_t world_res, void * ctx) {
    ui_sprite_particle_chipmunk_env_t env = (ui_sprite_particle_chipmunk_env_t)ui_sprite_world_res_data(world_res);
        
    while(!TAILQ_EMPTY(&env->m_collided_bodys)) {
        ui_sprite_particle_chipmunk_body_free(TAILQ_FIRST(&env->m_collided_bodys));
    }
}

ui_sprite_particle_chipmunk_env_t
ui_sprite_particle_chipmunk_env_create(ui_sprite_particle_chipmunk_module_t module, ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res;
    ui_sprite_particle_chipmunk_env_t env;
    ui_sprite_chipmunk_env_t chipmunk_env;
    cpCollisionHandler * collision_handelr;
    
    chipmunk_env = ui_sprite_chipmunk_env_find(world);
    if (chipmunk_env == NULL) {
        CPE_ERROR(module->m_em, "create particle_chipmunk: no chipmunk env!");
        return NULL;
    }

    world_res =
        ui_sprite_world_res_create(
            world, UI_SPRITE_PARTICLE_CHIPMUNK_ENV_NAME, sizeof(struct ui_sprite_particle_chipmunk_env));

    if (world_res == NULL) {
        CPE_ERROR(module->m_em, "create particle_chipmunk env: creat res fail!");
        return NULL;
    }

    env = (ui_sprite_particle_chipmunk_env_t)ui_sprite_world_res_data(world_res);

    bzero(env, sizeof(*env));

    env->m_module = module;
    env->m_chipmunk_env = chipmunk_env;
    env->m_env = ui_sprite_chipmunk_env_env(chipmunk_env);
    env->m_obj_collision_type = ui_sprite_chipmunk_env_collision_type(chipmunk_env);

    if (plugin_chipmunk_env_register_collision_type(&env->m_collision_type, env->m_env, "particle") != 0) {
        CPE_ERROR(module->m_em, "create particle_chipmunk env: register particle_chipmunk env fail!");
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    if (ui_sprite_world_add_updator(world, ui_sprite_particle_chipmunk_env_update, env) != 0) {
        CPE_ERROR(module->m_em, "create particle_chipmunk env: add updator fail!");
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    collision_handelr =
        cpSpaceAddWildcardHandler(
            (cpSpace*)plugin_chipmunk_env_space(env->m_env),
            env->m_collision_type);
    assert(collision_handelr);
    collision_handelr->beginFunc = ui_sprite_particle_chipmunk_env_begin_collision;
    collision_handelr->userData = env;

    TAILQ_INIT(&env->m_collided_bodys);

    ui_sprite_world_res_set_free_fun(world_res, ui_sprite_particle_chipmunk_env_clear, NULL);

    return env;
}

void ui_sprite_particle_chipmunk_env_free(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_PARTICLE_CHIPMUNK_ENV_NAME);
    if (world_res) {
        ui_sprite_world_res_free(world_res);
    }
}

ui_sprite_particle_chipmunk_env_t ui_sprite_particle_chipmunk_env_find(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_PARTICLE_CHIPMUNK_ENV_NAME);
    return world_res ? (ui_sprite_particle_chipmunk_env_t)ui_sprite_world_res_data(world_res) : NULL;
}

int ui_sprite_particle_chipmunk_env_set_update_priority(ui_sprite_particle_chipmunk_env_t env, int8_t priority) {
    ui_sprite_world_res_t res = ui_sprite_world_res_from_data(env);
    ui_sprite_world_t world = ui_sprite_world_res_world(res);
    return ui_sprite_world_set_updator_priority(world, env, priority);
}

static void ui_sprite_particle_chipmunk_env_update(ui_sprite_world_t world, void * ctx, float delta_s) {
    ui_sprite_particle_chipmunk_env_t env = (ui_sprite_particle_chipmunk_env_t)ctx;

    /*清除所有消弹完成的子弹 */
    while(!TAILQ_EMPTY(&env->m_collided_bodys)) {
        ui_sprite_particle_chipmunk_body_free(TAILQ_FIRST(&env->m_collided_bodys));
    }
}

static void ui_sprite_particle_chipmunk_env_post_step_remove_body(cpSpace *space, cpShape *shape, cpDataPointer userData) {
    ui_sprite_particle_chipmunk_body_t body = (ui_sprite_particle_chipmunk_body_t)userData;
    ui_sprite_particle_chipmunk_env_t env = body->m_with_collision->m_chipmunk_env;
    
    assert(body->m_state == ui_sprite_particle_chipmunk_body_state_active);

    /*进入消弹状态 */
    body->m_state = ui_sprite_particle_chipmunk_body_state_colliede;
    TAILQ_INSERT_TAIL(&env->m_collided_bodys, body, m_next_for_env);
    
    if (body->m_shape.shape.space) {
        cpSpaceRemoveShape(body->m_shape.shape.space, (cpShape*)&body->m_shape);
    }
    
    cpShapeDestroy(&body->m_shape.shape);
    cpBodyDestroy(&body->m_body);
}

static cpBool ui_sprite_particle_chipmunk_env_begin_collision(cpArbiter *arb, cpSpace *space, cpDataPointer userData) {
    ui_sprite_particle_chipmunk_env_t env = (ui_sprite_particle_chipmunk_env_t)userData;
    ui_sprite_particle_chipmunk_body_t body = NULL;
    ui_sprite_chipmunk_obj_body_t target = NULL;
    ui_vector_2_t pt;
    ui_vector_2 pt_buf;

    if (cpArbiterGetCount(arb) > 0) {
        cpVect t = cpArbiterGetPointA(arb, 0);
        pt_buf.x = t.x;
        pt_buf.y = t.y;
        pt = &pt_buf;
    }
    else {
        pt = NULL;
    }

    if (cpShapeGetCollisionType(arb->a) == env->m_collision_type) {
        body = (ui_sprite_particle_chipmunk_body_t)arb->a->userData;

        if (ui_sprite_particle_chipmunk_body_on_collided(body, pt)) {
            cpSpaceAddPostStepCallback(space, (cpPostStepFunc)ui_sprite_particle_chipmunk_env_post_step_remove_body, body, body);
        }

        if (cpShapeGetCollisionType(arb->b) == env->m_obj_collision_type) {
            target = (ui_sprite_chipmunk_obj_body_t)arb->b->userData;
        }
    }
    
    if (cpShapeGetCollisionType(arb->b) == env->m_collision_type) {
        body = (ui_sprite_particle_chipmunk_body_t)arb->b->userData;

        if (ui_sprite_particle_chipmunk_body_on_collided(body, pt)) {
            cpSpaceAddPostStepCallback(space, (cpPostStepFunc)ui_sprite_particle_chipmunk_env_post_step_remove_body, body, body);
        }

        if (cpShapeGetCollisionType(arb->a) == env->m_obj_collision_type) {
            target = (ui_sprite_chipmunk_obj_body_t)arb->a->userData;
        }
    }

    if (target) {
        UI_SPRITE_CHIPMUNK_COLLISION_DATA collisition_data;
        ui_sprite_fsm_action_t from_action = ui_sprite_fsm_action_from_data(body->m_with_collision);
        ui_sprite_entity_t from_entity = ui_sprite_fsm_action_to_entity(from_action);
        plugin_particle_obj_emitter_t emitter;
        struct dr_data_source emitter_data_buf;
        struct dr_data_source collision_data_buf;
        dr_data_source_t data_source = NULL;
        dr_data_t emitter_data;
        ui_sprite_event_t evt;

        /*填写发射器附带信息 */
        emitter = plugin_particle_obj_particle_emitter(
            plugin_particle_obj_plugin_data_particle(
                plugin_particle_obj_plugin_data_from_data(body)));
        emitter_data = plugin_particle_obj_emitter_addition_data(emitter);
        if (emitter_data) {
            emitter_data_buf.m_data = *emitter_data;
            emitter_data_buf.m_next = data_source;
            data_source = &emitter_data_buf;
        }

        /*填写碰撞信息 */
        collisition_data.collision_state = UI_SPRITE_CHIPMUNK_COLLISION_STATE_BEGIN;
        collisition_data.collision_entity_id = ui_sprite_entity_id(from_entity);
        
        cpe_str_dup(
            collisition_data.collision_part_name,
            sizeof(collisition_data.collision_part_name),
            ui_sprite_chipmunk_obj_body_name(target));
        
        cpe_str_dup(
            collisition_data.collision_other_name,
            sizeof(collisition_data.collision_other_name),
            plugin_particle_obj_emitter_name(emitter));
        
        collisition_data.collision_pos.x = pt ? pt->x : 0.0f;
        collisition_data.collision_pos.y = pt ? pt->y : 0.0f;
        
        collision_data_buf.m_data.m_meta = ui_sprite_chipmunk_module_collision_data_meta(ui_sprite_chipmunk_env_module(env->m_chipmunk_env));
        collision_data_buf.m_data.m_data = (void*)&collisition_data;
        collision_data_buf.m_data.m_size = sizeof(collisition_data);
        collision_data_buf.m_next = data_source;
        data_source = &collision_data_buf;

        /*构造事件 */
        evt = ui_sprite_fsm_action_build_event(
            from_action, env->m_module->m_alloc, body->m_with_collision->m_cfg_collision_event, data_source);
        if (evt == NULL) {
            CPE_ERROR(env->m_module->m_em, "particle_chipmunk: on collision: build event fail!");
            return 1;
        }

        /*发送事件 */
        ui_sprite_entity_send_event(
            ui_sprite_component_entity(
                ui_sprite_component_from_data(
                    ui_sprite_chipmunk_obj_body_obj(target))),
            evt->meta, evt->data, evt->size);

        mem_free(env->m_module->m_alloc, evt);
    }
    
    return 1;
}

ui_sprite_world_res_t ui_sprite_particle_chipmunk_env_load(void * ctx, ui_sprite_world_t world, cfg_t cfg) {
    ui_sprite_particle_chipmunk_module_t module = (ui_sprite_particle_chipmunk_module_t)ctx;
    ui_sprite_particle_chipmunk_env_t env = ui_sprite_particle_chipmunk_env_create(module, world);

    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create particle_chipmunk_env resource: create particle_chipmunk_env fail!",
            ui_sprite_particle_chipmunk_module_name(module));
        return NULL;
    }

    if (ui_sprite_particle_chipmunk_env_set_update_priority(env, cfg_get_int8(cfg, "update-priority", 0)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create particle_chipmunk_env resource: set update priority %d fail!",
            ui_sprite_particle_chipmunk_module_name(module), cfg_get_int8(cfg, "update-priority", 0));
        ui_sprite_particle_chipmunk_env_free(world);
        return NULL;
    }

    return ui_sprite_world_res_from_data(env);
}

int ui_sprite_particle_chipmunk_env_regist(ui_sprite_particle_chipmunk_module_t module) {
    if (ui_sprite_cfg_loader_add_resource_loader(
            module->m_loader, UI_SPRITE_PARTICLE_CHIPMUNK_ENV_NAME, ui_sprite_particle_chipmunk_env_load, module)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: %s register: register loader fail",
            ui_sprite_particle_chipmunk_module_name(module), UI_SPRITE_PARTICLE_CHIPMUNK_ENV_NAME);
        return -1;
    }

    return 0;
}

void ui_sprite_particle_chipmunk_env_unregist(ui_sprite_particle_chipmunk_module_t module) {
    if (ui_sprite_cfg_loader_remove_resource_loader(module->m_loader, UI_SPRITE_PARTICLE_CHIPMUNK_ENV_NAME) != 0) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: loader remove fail",
            ui_sprite_particle_chipmunk_module_name(module), UI_SPRITE_PARTICLE_CHIPMUNK_ENV_NAME);
    }
}

const char * UI_SPRITE_PARTICLE_CHIPMUNK_ENV_NAME = "ParticleChipmunkEnv";

#ifdef __cplusplus
}
#endif
