#include <assert.h>
#include "chipmunk/chipmunk_private.h"
#include "cpe/pal/pal_strings.h"
#include "gd/app/app_context.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "plugin/barrage/plugin_barrage_bullet.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_env.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_obj.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_obj_body.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_obj_shape_group.h"
#include "ui_sprite_barrage_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static void ui_sprite_barrage_env_update(ui_sprite_world_t world, void * ctx, float delta_s);
static cpBool ui_sprite_barrage_env_begin_collision(cpArbiter *arb, cpSpace *space, cpDataPointer userData);

static void ui_sprite_barrage_env_clear(ui_sprite_world_res_t world_res, void * ctx) {
    ui_sprite_barrage_env_t env = (ui_sprite_barrage_env_t)ui_sprite_world_res_data(world_res);

    plugin_barrage_env_free(env->m_env);
}

ui_sprite_barrage_env_t
ui_sprite_barrage_env_create(ui_sprite_barrage_module_t module, ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res;
    ui_sprite_barrage_env_t env;
    ui_sprite_chipmunk_env_t chipmunk_env;
    cpCollisionHandler * collision_handelr;

    chipmunk_env = ui_sprite_chipmunk_env_find(world);
    if (chipmunk_env == NULL) {
        CPE_ERROR(module->m_em, "create barrage env: chipmunk env not exist!");
        return NULL;
    }

    world_res =
        ui_sprite_world_res_create(
            world, UI_SPRITE_BARRAGE_ENV_NAME, sizeof(struct ui_sprite_barrage_env));

    if (world_res == NULL) {
        CPE_ERROR(module->m_em, "create barrage env: creat res fail!");
        return NULL;
    }

    env = (ui_sprite_barrage_env_t)ui_sprite_world_res_data(world_res);

    bzero(env, sizeof(*env));

    env->m_module = module;
    env->m_env = plugin_barrage_env_create(module->m_barrage_module, ui_sprite_chipmunk_env_env(chipmunk_env));
    if (env->m_env == NULL) {
        CPE_ERROR(module->m_em, "create barrage env: creat obj mgr fail!");
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    if (ui_sprite_world_add_updator(world, ui_sprite_barrage_env_update, env) != 0) {
        CPE_ERROR(module->m_em, "create barrage env: add updator fail!");
        ui_sprite_world_res_free(world_res);
        return NULL;
    }

    collision_handelr =
        cpSpaceAddCollisionHandler(
            (cpSpace*)plugin_chipmunk_env_space(ui_sprite_chipmunk_env_env(chipmunk_env)),
            plugin_barrage_env_collision_type(env->m_env),
            ui_sprite_chipmunk_env_collision_type(chipmunk_env));
    assert(collision_handelr);
    collision_handelr->beginFunc = ui_sprite_barrage_env_begin_collision;
    collision_handelr->userData = env;
    
    ui_sprite_world_res_set_free_fun(world_res, ui_sprite_barrage_env_clear, NULL);

    return env;
}

void ui_sprite_barrage_env_free(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = ui_sprite_world_res_find(world, UI_SPRITE_BARRAGE_ENV_NAME);
    if (world_res) {
        ui_sprite_world_res_free(world_res);
    }
}

ui_sprite_barrage_env_t ui_sprite_barrage_env_find(ui_sprite_world_t world) {
    ui_sprite_world_res_t world_res = (ui_sprite_world_res_t)ui_sprite_world_res_find(world, UI_SPRITE_BARRAGE_ENV_NAME);
    return world_res ? (ui_sprite_barrage_env_t)ui_sprite_world_res_data(world_res) : NULL;
}

plugin_barrage_env_t ui_sprite_barrage_env_env(ui_sprite_barrage_env_t env) {
    return env->m_env;
}

int ui_sprite_barrage_env_set_update_priority(ui_sprite_barrage_env_t env, int8_t priority) {
    ui_sprite_world_res_t res = ui_sprite_world_res_from_data(env);
    ui_sprite_world_t world = ui_sprite_world_res_world(res);
    return ui_sprite_world_set_updator_priority(world, env, priority);
}

static void ui_sprite_barrage_env_update(ui_sprite_world_t world, void * ctx, float delta_s) {
    ui_sprite_barrage_env_t env = (ui_sprite_barrage_env_t)ctx;
    ui_sprite_render_env_t render_env = ui_sprite_render_env_find(world);

	if (render_env) {
		ui_vector_2_t screen_size = ui_sprite_render_env_size(render_env);
		BARRAGE_RECT barrage_rect;

		barrage_rect.lt.x = 0.0f;
		barrage_rect.lt.y = 0.0F;
		barrage_rect.rb.x = barrage_rect.lt.x + screen_size->x;
		barrage_rect.rb.y = barrage_rect.lt.x + screen_size->y;
        
        plugin_barrage_env_update(env->m_env, delta_s, &barrage_rect);
    }
    else {
        plugin_barrage_env_update(env->m_env, delta_s, NULL);
    }
}

cpBool ui_sprite_barrage_env_begin_collision(cpArbiter *arb, cpSpace *space, cpDataPointer userData) {
    ui_sprite_barrage_env_t env = (ui_sprite_barrage_env_t)userData;
    plugin_barrage_bullet_t bullet;
    ui_sprite_chipmunk_obj_body_t body;
    dr_data_t carry_data;

	if (!cpArbiterCallWildcardBeginA(arb, space)
        || !cpArbiterCallWildcardBeginB(arb, space))
    {
        return cpFalse;
    }

    if (cpShapeGetCollisionType(arb->a) == plugin_barrage_env_collision_type(env->m_env)) {
        bullet = (plugin_barrage_bullet_t)arb->a->userData;
        body = (ui_sprite_chipmunk_obj_body_t)arb->b->userData;
    }
    else {
        bullet = (plugin_barrage_bullet_t)arb->b->userData;
        body = (ui_sprite_chipmunk_obj_body_t)arb->a->userData;
    }

    assert(bullet);
    assert(body);

    carry_data = plugin_barrage_bullet_carray_data(bullet);
    if (carry_data == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_barrage_env_begin_collision: no carry data!");
        return 1;
    }

    ui_sprite_entity_send_event(
        ui_sprite_component_entity(
            ui_sprite_component_from_data(
                ui_sprite_chipmunk_obj_body_obj(body))),
        carry_data->m_meta, carry_data->m_data, carry_data->m_size);

    return 1;
}
    
int ui_sprite_barrage_env_regist(ui_sprite_barrage_module_t module) {
    if (ui_sprite_cfg_loader_add_resource_loader(
            module->m_loader, UI_SPRITE_BARRAGE_ENV_NAME, ui_sprite_barrage_env_load, module)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: %s register: register loader fail",
            ui_sprite_barrage_module_name(module), UI_SPRITE_BARRAGE_ENV_NAME);
        return -1;
    }

    return 0;
}

void ui_sprite_barrage_env_unregist(ui_sprite_barrage_module_t module) {
    if (ui_sprite_cfg_loader_remove_resource_loader(module->m_loader, UI_SPRITE_BARRAGE_ENV_NAME) != 0) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: loader remove fail",
            ui_sprite_barrage_module_name(module), UI_SPRITE_BARRAGE_ENV_NAME);
    }
}

const char * UI_SPRITE_BARRAGE_ENV_NAME = "BarrageEnv";

#ifdef __cplusplus
}
#endif
