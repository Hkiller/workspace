#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui_sprite_chipmunk_env_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "ui_sprite_chipmunk_monitor_i.h"
#include "ui_sprite_chipmunk_monitor_binding_i.h"
#include "ui_sprite_chipmunk_touch_trace_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static void ui_sprite_chipmunk_env_update(ui_sprite_world_t world, void * ctx, float delta_s);
static cpBool ui_sprite_chipmunk_env_begin_collision(cpArbiter *arb, cpSpace *space, cpDataPointer userData);
static void ui_sprite_chipmunk_env_end_collision(cpArbiter *arb, cpSpace *space, cpDataPointer userData);

static void ui_sprite_chipmunk_env_clear(ui_sprite_world_res_t world_res, void * ctx) {
    ui_sprite_chipmunk_env_t env = (ui_sprite_chipmunk_env_t)ui_sprite_world_res_data(world_res);

    ui_sprite_chipmunk_touch_trace_free_all(env);

    if (env->m_process_touch) {
        ui_sprite_chipmunk_env_set_process_touch(env, 0);
        assert(!env->m_process_touch);
    }
    
    assert(TAILQ_EMPTY(&env->m_scopes));

    plugin_chipmunk_env_free(env->m_env);
}

ui_sprite_chipmunk_env_t
ui_sprite_chipmunk_env_create(ui_sprite_chipmunk_module_t module, ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res;
    ui_sprite_chipmunk_env_t env;
    cpCollisionHandler * collision_handelr;

    world_res =
        ui_sprite_world_res_create(
            world, UI_SPRITE_CHIPMUNK_ENV_NAME, sizeof(struct ui_sprite_chipmunk_env));

    if (world_res == NULL) {
        CPE_ERROR(module->m_em, "create chipmunk env: creat res fail!");
        return NULL;
    }

    env = (ui_sprite_chipmunk_env_t)ui_sprite_world_res_data(world_res);

    bzero(env, sizeof(*env));

    env->m_module = module;
    env->m_max_finger_count = 3;
    env->m_env = plugin_chipmunk_env_create(module->m_chipmunk_module);
    if (env->m_env == NULL) {
        CPE_ERROR(module->m_em, "create chipmunk env: creat obj mgr fail!");
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    if (plugin_chipmunk_env_register_collision_type(&env->m_collision_type, env->m_env, "object") != 0) {
        plugin_chipmunk_env_free(env->m_env);
        CPE_ERROR(module->m_em, "create chipmunk env: register chipmunk env fail!");
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    if (ui_sprite_world_add_updator(world, ui_sprite_chipmunk_env_update, env) != 0) {
        CPE_ERROR(module->m_em, "create chipmunk env: add updator fail!");
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    collision_handelr =
        cpSpaceAddCollisionHandler(
            (cpSpace*)plugin_chipmunk_env_space(env->m_env),
            env->m_collision_type, env->m_collision_type);
    assert(collision_handelr);
    collision_handelr->beginFunc = ui_sprite_chipmunk_env_begin_collision;
    collision_handelr->separateFunc = ui_sprite_chipmunk_env_end_collision;
    collision_handelr->userData = env;

    env->m_touch_filter.group = CP_NO_GROUP;
    env->m_touch_filter.categories = CP_ALL_CATEGORIES;
    env->m_touch_filter.mask = CP_ALL_CATEGORIES;
    
    TAILQ_INIT(&env->m_scopes);
    TAILQ_INIT(&env->m_touch_traces);
    
    ui_sprite_world_res_set_free_fun(world_res, ui_sprite_chipmunk_env_clear, NULL);

    return env;
}

void ui_sprite_chipmunk_env_free(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_CHIPMUNK_ENV_NAME);
    if (world_res) {
        ui_sprite_world_res_free(world_res);
    }
}

ui_sprite_chipmunk_env_t ui_sprite_chipmunk_env_find(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_CHIPMUNK_ENV_NAME);
    return world_res ? (ui_sprite_chipmunk_env_t)ui_sprite_world_res_data(world_res) : NULL;
}

ui_sprite_chipmunk_module_t ui_sprite_chipmunk_env_module(ui_sprite_chipmunk_env_t env) {
    return env->m_module;
}
    
plugin_chipmunk_env_t ui_sprite_chipmunk_env_env(ui_sprite_chipmunk_env_t env) {
    return env->m_env;
}

uint32_t ui_sprite_chipmunk_env_collision_type(ui_sprite_chipmunk_env_t env) {
    return env->m_collision_type;
}

int ui_sprite_chipmunk_env_set_update_priority(ui_sprite_chipmunk_env_t env, int8_t priority) {
    ui_sprite_world_res_t res = ui_sprite_world_res_from_data(env);
    ui_sprite_world_t world = ui_sprite_world_res_world(res);
    return ui_sprite_world_set_updator_priority(world, env, priority);
}

static void ui_sprite_chipmunk_env_update(ui_sprite_world_t world, void * ctx, float delta_s) {
    ui_sprite_chipmunk_env_t env = (ui_sprite_chipmunk_env_t)ctx;

    plugin_chipmunk_env_update(env->m_env, delta_s);
}

static void ui_sprite_chipmunk_env_on_collision(cpArbiter *arb, cpSpace *space, cpDataPointer userData, uint8_t state) {
    ui_sprite_chipmunk_obj_body_t body_a;
    ui_sprite_chipmunk_obj_body_t body_b;
    ui_sprite_chipmunk_monitor_binding_t binding;
    ui_sprite_entity_t entity_a;
    ui_sprite_entity_t entity_b;
    UI_SPRITE_CHIPMUNK_COLLISION_DATA collision_data;
    uint8_t collision_data_is_init;
    cpVect arb_impulse;

    body_a = (ui_sprite_chipmunk_obj_body_t)arb->a->userData;
    entity_a = ui_sprite_component_entity(ui_sprite_component_from_data(body_a->m_obj));
    body_b = (ui_sprite_chipmunk_obj_body_t)arb->b->userData;
    entity_b = ui_sprite_component_entity(ui_sprite_component_from_data(body_b->m_obj));

    collision_data_is_init = 0;
    TAILQ_FOREACH(binding, &body_a->m_monitor_bindings, m_next_for_body) {
        ui_sprite_chipmunk_monitor_t monitor = binding->m_monitor;
        if (monitor->m_on_collision == NULL) continue;
        if (monitor->m_collision_mask && !(monitor->m_collision_mask & arb->b->filter.categories)) continue;
        if (monitor->m_collision_category && !(monitor->m_collision_category & arb->a->filter.categories)) continue;
        
        if (!collision_data_is_init) {
            collision_data.collision_state = state;
            collision_data.collision_entity_id = ui_sprite_entity_id(entity_b);
            cpe_str_dup(collision_data.collision_part_name, sizeof(collision_data.collision_part_name), body_a->m_name);
            cpe_str_dup(collision_data.collision_other_name, sizeof(collision_data.collision_other_name), body_b->m_name);

            /*计算冲量 */
            arb_impulse = cpArbiterTotalImpulse(arb);
            collision_data.collision_impulse_pair.x = arb_impulse.x;
            collision_data.collision_impulse_pair.y = arb_impulse.y;
            collision_data.collision_impulse = cpvlength(arb_impulse);
            
            /*计算碰撞点 */
            if (cpArbiterGetCount(arb) > 0) {
                cpVect t = cpArbiterGetPointA(arb, 0);
                collision_data.collision_pos.x = t.x;
                collision_data.collision_pos.y = t.y;
            }
            else {
                collision_data.collision_pos.x = 0.0f;
                collision_data.collision_pos.y = 0.0f;
            }
            
            collision_data_is_init = 1;
        }

        monitor->m_on_collision(monitor->m_ctx, &collision_data, entity_a, body_a, entity_b, body_b);
    }
    
    collision_data_is_init = 0;
    TAILQ_FOREACH(binding, &body_b->m_monitor_bindings, m_next_for_body) {
        ui_sprite_chipmunk_monitor_t monitor = binding->m_monitor;
        if (monitor->m_on_collision == NULL) continue;
        if (monitor->m_collision_mask && !(monitor->m_collision_mask & arb->a->filter.categories)) continue;
        if (monitor->m_collision_category && !(monitor->m_collision_category & arb->b->filter.categories)) continue;
                                           
        if (!collision_data_is_init) {
            collision_data.collision_state = state;
            collision_data.collision_entity_id = ui_sprite_entity_id(entity_a);
            cpe_str_dup(collision_data.collision_part_name, sizeof(collision_data.collision_part_name), body_b->m_name);
            cpe_str_dup(collision_data.collision_other_name, sizeof(collision_data.collision_other_name), body_a->m_name);

            /*计算冲量 */
            arb_impulse = cpArbiterTotalImpulse(arb);
            collision_data.collision_impulse_pair.x = arb_impulse.x;
            collision_data.collision_impulse_pair.y = arb_impulse.y;
            collision_data.collision_impulse = cpvlength(arb_impulse);
            
            /*计算碰撞点 */
            if (cpArbiterGetCount(arb) > 0) {
                cpVect t = cpArbiterGetPointA(arb, 0);
                collision_data.collision_pos.x = t.x;
                collision_data.collision_pos.y = t.y;
            }
            else {
                collision_data.collision_pos.x = 0.0f;
                collision_data.collision_pos.y = 0.0f;
            }

            collision_data_is_init = 1;
        }

        monitor->m_on_collision(monitor->m_ctx, &collision_data, entity_b, body_b, entity_a, body_a);
    }
}

static cpBool ui_sprite_chipmunk_env_begin_collision(cpArbiter *arb, cpSpace *space, cpDataPointer userData) {
	if (!cpArbiterCallWildcardBeginA(arb, space)
        || !cpArbiterCallWildcardBeginB(arb, space))
    {
        return cpFalse;
    }

    ui_sprite_chipmunk_env_on_collision(arb, space, userData, UI_SPRITE_CHIPMUNK_COLLISION_STATE_BEGIN);

    return cpTrue;
}

static void ui_sprite_chipmunk_env_end_collision(cpArbiter *arb, cpSpace *space, cpDataPointer userData) {
    ui_sprite_chipmunk_env_on_collision(arb, space, userData, UI_SPRITE_CHIPMUNK_COLLISION_STATE_END);
}
    
int ui_sprite_chipmunk_env_regist(ui_sprite_chipmunk_module_t module) {
    if (ui_sprite_cfg_loader_add_resource_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_ENV_NAME, ui_sprite_chipmunk_env_load, module)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: %s register: register loader fail",
            ui_sprite_chipmunk_module_name(module), UI_SPRITE_CHIPMUNK_ENV_NAME);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_env_unregist(ui_sprite_chipmunk_module_t module) {
    if (ui_sprite_cfg_loader_remove_resource_loader(module->m_loader, UI_SPRITE_CHIPMUNK_ENV_NAME) != 0) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: loader remove fail",
            ui_sprite_chipmunk_module_name(module), UI_SPRITE_CHIPMUNK_ENV_NAME);
    }
}

const char * UI_SPRITE_CHIPMUNK_ENV_NAME = "ChipmunkEnv";

#ifdef __cplusplus
}
#endif
