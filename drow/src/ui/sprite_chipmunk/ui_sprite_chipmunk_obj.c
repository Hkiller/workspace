#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "gd/app/app_context.h"
#include "render/utils/ui_rect.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "ui_sprite_chipmunk_obj_updator_i.h"
#include "ui_sprite_chipmunk_obj_constraint_i.h"
#include "ui_sprite_chipmunk_tri_scope_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static void ui_sprite_chipmunk_obj_on_set_linear_velocity(void * ctx);

ui_sprite_chipmunk_obj_t ui_sprite_chipmunk_obj_create(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_create(entity, UI_SPRITE_CHIPMUNK_OBJ_NAME);
    return component ? (ui_sprite_chipmunk_obj_t)ui_sprite_component_data(component) : NULL;
};

ui_sprite_chipmunk_obj_t ui_sprite_chipmunk_obj_find(ui_sprite_entity_t entity) {
    ui_sprite_component_t component = ui_sprite_component_find(entity, UI_SPRITE_CHIPMUNK_OBJ_NAME);
    return component ? (ui_sprite_chipmunk_obj_t)ui_sprite_component_data(component) : NULL;
};

void ui_sprite_chipmunk_obj_free(ui_sprite_chipmunk_obj_t chipmunk_obj) {
    ui_sprite_component_t component = ui_sprite_component_from_data(chipmunk_obj);
    if (component) {
        ui_sprite_component_free(component);
    }
}

ui_sprite_chipmunk_env_t ui_sprite_chipmunk_obj_env(ui_sprite_chipmunk_obj_t chipmunk_obj) {
    return chipmunk_obj->m_env;
}
    
void ui_sprite_chipmunk_obj_set_linear_velocity(ui_sprite_chipmunk_obj_t chipmunk_obj, float angle, float velocity) {
    if (chipmunk_obj->m_main_body) {
        ui_sprite_chipmunk_obj_body_set_linear_velocity(chipmunk_obj->m_main_body, angle, velocity);
    }
    else {
        chipmunk_obj->m_data.setter.linear_velocity_angle.setted = 1;
        chipmunk_obj->m_data.setter.linear_velocity_angle.value = angle;

        chipmunk_obj->m_data.setter.linear_velocity_value.setted = 1;
        chipmunk_obj->m_data.setter.linear_velocity_value.value = velocity;
    }
}
    
ui_sprite_chipmunk_obj_body_t ui_sprite_chipmunk_obj_main_body(ui_sprite_chipmunk_obj_t chipmunk_obj) {
    return chipmunk_obj->m_main_body;
}
    
static int ui_sprite_chipmunk_obj_do_init(ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_obj_t obj, ui_sprite_entity_t entity) {
    ui_sprite_chipmunk_env_t env = ui_sprite_chipmunk_env_find(ui_sprite_entity_world(entity));

    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk obj init: no chipmunt_env!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    bzero(obj, sizeof(*obj));

    obj->m_env = env;
    obj->m_body_count = 0;
    
    TAILQ_INIT(&obj->m_bodies);
    TAILQ_INIT(&obj->m_updators);
    TAILQ_INIT(&obj->m_monitors);
    TAILQ_INIT(&obj->m_scopes);
    TAILQ_INIT(&obj->m_responsers);

    return 0;
}

static int ui_sprite_chipmunk_obj_init(ui_sprite_component_t component, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_chipmunk_obj_t chipmunk_obj = (ui_sprite_chipmunk_obj_t)ui_sprite_component_data(component);

    if (ui_sprite_chipmunk_obj_do_init(module, chipmunk_obj, entity) != 0) return -1;

    return 0;
}

static void ui_sprite_chipmunk_obj_fini(ui_sprite_component_t component, void * ctx) {
    ui_sprite_chipmunk_obj_t chipmunk_obj = (ui_sprite_chipmunk_obj_t)ui_sprite_component_data(component);

    while(!TAILQ_EMPTY(&chipmunk_obj->m_updators)) {
        ui_sprite_chipmunk_obj_updator_free(TAILQ_FIRST(&chipmunk_obj->m_updators));
    }
    
    while(!TAILQ_EMPTY(&chipmunk_obj->m_bodies)) {
        ui_sprite_chipmunk_obj_body_free(TAILQ_FIRST(&chipmunk_obj->m_bodies));
    }

    assert(chipmunk_obj->m_body_count == 0);
    assert(TAILQ_EMPTY(&chipmunk_obj->m_responsers));
}

static int ui_sprite_chipmunk_obj_copy(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(to);
    ui_sprite_chipmunk_obj_t from_chipmunk_obj = (ui_sprite_chipmunk_obj_t)ui_sprite_component_data(from);
    ui_sprite_chipmunk_obj_t to_chipmunk_obj = (ui_sprite_chipmunk_obj_t)ui_sprite_component_data(to);
    ui_sprite_chipmunk_obj_body_t from_body;
    ui_sprite_chipmunk_obj_body_t to_body;

    if (ui_sprite_chipmunk_obj_do_init(module, to_chipmunk_obj, entity) != 0) return -1;

    TAILQ_FOREACH(from_body, &from_chipmunk_obj->m_bodies, m_next_for_obj) {
        if (from_body->m_is_runtime) continue;

        to_body = ui_sprite_chipmunk_obj_body_clone(to_chipmunk_obj, from_body);
        if (to_body == NULL) {
            while(!TAILQ_EMPTY(&to_chipmunk_obj->m_bodies)) {
                ui_sprite_chipmunk_obj_body_free(TAILQ_FIRST(&to_chipmunk_obj->m_bodies));
            }
            return -1;
        }
    }
    
    return 0;
}

static void ui_sprite_chipmunk_obj_exit(ui_sprite_component_t component, void * ctx) {
    ui_sprite_chipmunk_obj_t chipmunk_obj = (ui_sprite_chipmunk_obj_t)ui_sprite_component_data(component);
    ui_sprite_chipmunk_obj_body_t body;
    
    assert(TAILQ_EMPTY(&chipmunk_obj->m_monitors));

    while(!TAILQ_EMPTY(&chipmunk_obj->m_scopes)) {
        ui_sprite_chipmunk_tri_scope_t scope = TAILQ_FIRST(&chipmunk_obj->m_scopes);
        assert(scope->m_obj == chipmunk_obj);
        
        TAILQ_REMOVE(&chipmunk_obj->m_env->m_scopes, scope, m_next_for_env);
        TAILQ_REMOVE(&chipmunk_obj->m_scopes, scope, m_next_for_obj);
        scope->m_obj = NULL;
    }
    
    TAILQ_FOREACH(body, &chipmunk_obj->m_bodies, m_next_for_obj) {
        if (body->m_is_in_space) ui_sprite_chipmunk_obj_body_remove_from_space(body);
    }
}

static void ui_sprite_chipmunk_obj_on_transform_update(void * ctx) {
    ui_sprite_chipmunk_obj_t chipmunk_obj = (ui_sprite_chipmunk_obj_t)ctx;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(chipmunk_obj));
    ui_sprite_2d_transform_t transform;
    ui_sprite_chipmunk_obj_body_t chipmunk_body;

    if (chipmunk_obj->m_main_body) {
        chipmunk_obj->m_data.linear_velocity.x = chipmunk_obj->m_main_body->m_body.v.x;
        chipmunk_obj->m_data.linear_velocity.y = chipmunk_obj->m_main_body->m_body.v.y;
    }

    if ((transform = ui_sprite_2d_transform_find(entity))) {
        ui_vector_2 entity_pos = ui_sprite_2d_transform_origin_pos(transform);
        float angle = ui_sprite_2d_transform_angle(transform);

        TAILQ_FOREACH(chipmunk_body, &chipmunk_obj->m_bodies, m_next_for_obj) {
            if (!chipmunk_body->m_is_in_space) continue;
            
            if (chipmunk_body == chipmunk_obj->m_main_body || !chipmunk_body->m_body_attrs.m_is_free) {
                ui_vector_2 adj_pos;
                adj_pos.x = chipmunk_body->m_body_attrs.m_position.x;
                adj_pos.y = chipmunk_body->m_body_attrs.m_position.y;
                adj_pos = ui_sprite_2d_transform_adj_local_pos(transform, adj_pos, UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL);
                
                cpBodySetPosition(&chipmunk_body->m_body, cpv(entity_pos.x + adj_pos.x, entity_pos.y + adj_pos.y));
                cpBodySetAngle(&chipmunk_body->m_body, cpe_math_angle_to_radians(angle));

                if (cpBodyGetType(&chipmunk_body->m_body) == CP_BODY_TYPE_STATIC) {
                    cpSpaceReindexShapesForBody((cpSpace * )plugin_chipmunk_env_space(chipmunk_obj->m_env->m_env), &chipmunk_body->m_body);
                }
            }
        }
    }
}

static void ui_sprite_chipmunk_obj_update(ui_sprite_component_t component, void * ctx, float delta) {
    //ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_obj_t chipmunk_obj = (ui_sprite_chipmunk_obj_t)ui_sprite_component_data(component);
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_2d_transform_t transform;

    if ((transform = ui_sprite_2d_transform_find(entity))) {
        cpVect pos = cpBodyGetPosition(&chipmunk_obj->m_main_body->m_body);
        float angle = cpe_math_angle_regular(cpe_math_radians_to_angle(cpBodyGetAngle(&chipmunk_obj->m_main_body->m_body)));
        ui_vector_2 old_pos = ui_sprite_2d_transform_origin_pos(transform);
        ui_vector_2 adj_pos;
        
        if (angle != ui_sprite_2d_transform_angle(transform)) {
            ui_sprite_2d_transform_set_angle(transform, angle);
        }

        adj_pos.x = chipmunk_obj->m_main_body->m_body_attrs.m_position.x;
        adj_pos.y = chipmunk_obj->m_main_body->m_body_attrs.m_position.y;
        adj_pos = ui_sprite_2d_transform_adj_local_pos(transform, adj_pos, UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL);

        pos.x -= adj_pos.x;
        pos.y -= adj_pos.y;
        
        if (fabs(pos.x - old_pos.x) > 0.01f || fabs(pos.y - old_pos.y) > 0.01f) {
            ui_sprite_chipmunk_obj_body_t chipmunk_body;
            ui_vector_2 new_pos = UI_VECTOR_2_INITLIZER( pos.x, pos.y );
            ui_sprite_2d_transform_set_origin_pos(transform, new_pos);

            TAILQ_FOREACH(chipmunk_body, &chipmunk_obj->m_bodies, m_next_for_obj) {
                ui_vector_2 adj_pos;
                
                if (chipmunk_obj == (ui_sprite_chipmunk_obj_t)chipmunk_obj->m_main_body) continue;
                if (chipmunk_body->m_body_attrs.m_is_free) continue;
                
                adj_pos.x = chipmunk_body->m_body_attrs.m_position.x;
                adj_pos.y = chipmunk_body->m_body_attrs.m_position.y;
                adj_pos = ui_sprite_2d_transform_adj_local_pos(transform, adj_pos, UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL);
                cpBodySetPosition(&chipmunk_body->m_body, cpv(new_pos.x + adj_pos.x, new_pos.y + adj_pos.y));
                cpBodySetAngle(&chipmunk_body->m_body, cpe_math_angle_to_radians(angle));
            }
        }
    }
}

int ui_sprite_component_obj_set_main_body(ui_sprite_chipmunk_obj_t chipmunk_obj, ui_sprite_chipmunk_obj_body_t body) {
    ui_sprite_chipmunk_module_t module = chipmunk_obj->m_env->m_module;

    if (chipmunk_obj->m_main_body == body) return 0;

    if (chipmunk_obj->m_main_body) {
        if (!chipmunk_obj->m_data.setter.linear_velocity_angle.setted) {
            chipmunk_obj->m_data.setter.linear_velocity_angle.setted = 1;
            chipmunk_obj->m_data.setter.linear_velocity_angle.value
                = ui_sprite_chipmunk_obj_body_linear_velocity_angle(chipmunk_obj->m_main_body);
        }

        if (!chipmunk_obj->m_data.setter.linear_velocity_value.setted) {
            chipmunk_obj->m_data.setter.linear_velocity_value.setted = 1;
            chipmunk_obj->m_data.setter.linear_velocity_value.value
                = ui_sprite_chipmunk_obj_body_linear_velocity(chipmunk_obj->m_main_body);
        }
    }
    
    chipmunk_obj->m_main_body = body;

    if (ui_sprite_component_is_active(ui_sprite_component_from_data(chipmunk_obj))) {
        if (chipmunk_obj->m_main_body) {
            ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(chipmunk_obj));
            ui_sprite_2d_transform_t transform;
            ui_vector_2 entity_pos;
            
            transform = ui_sprite_2d_transform_find(entity);
            if (transform == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): chipmunk obj: set main body: no transform!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                return -1;
            }

            entity_pos = ui_sprite_2d_transform_origin_pos(transform);
            cpBodySetPosition(&chipmunk_obj->m_main_body->m_body, cpv(entity_pos.x, entity_pos.y));
            cpBodySetAngle(&chipmunk_obj->m_main_body->m_body, cpe_math_angle_to_radians(ui_sprite_2d_transform_angle(transform)));
        }

        ui_sprite_chipmunk_obj_update_move_policy(chipmunk_obj);
        if (chipmunk_obj->m_main_body) { ui_sprite_chipmunk_obj_on_set_linear_velocity(chipmunk_obj);
        }
    }

    return 0;
}
    
static int ui_sprite_chipmunk_obj_enter(ui_sprite_component_t component, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_obj_t chipmunk_obj = (ui_sprite_chipmunk_obj_t)ui_sprite_component_data(component);
    cpSpace * space = (cpSpace * )plugin_chipmunk_env_space(chipmunk_obj->m_env->m_env);
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_2d_transform_t transform;
    ui_sprite_chipmunk_obj_body_t body;

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk obj enter: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    TAILQ_FOREACH(body, &chipmunk_obj->m_bodies, m_next_for_obj) {
        if (ui_sprite_chipmunk_obj_body_add_to_space_i(body, space, transform) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk obj enter: body %s add to space fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), body->m_name);

            for(body = TAILQ_PREV(body, ui_sprite_chipmunk_obj_body_list, m_next_for_obj);
                body;
                body = TAILQ_PREV(body, ui_sprite_chipmunk_obj_body_list, m_next_for_obj))
            {
                ui_sprite_chipmunk_obj_body_remove_from_space(body);
            }

            return -1;
        }
    }

    return 0;
}

static void ui_sprite_chipmunk_obj_on_shape_update(void * ctx) {
    ui_sprite_chipmunk_obj_t chipmunk_obj = (ui_sprite_chipmunk_obj_t)ctx;
    cpSpace * space = (cpSpace * )plugin_chipmunk_env_space(chipmunk_obj->m_env->m_env);
    ui_sprite_chipmunk_module_t module = chipmunk_obj->m_env->m_module;
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(chipmunk_obj));
    ui_sprite_2d_transform_t transform;
    ui_sprite_chipmunk_obj_body_t body;

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk obj on shape update: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    TAILQ_FOREACH(body, &chipmunk_obj->m_bodies, m_next_for_obj) {
        if (body->m_is_in_space) ui_sprite_chipmunk_obj_body_remove_from_space(body);
    }

    TAILQ_FOREACH(body, &chipmunk_obj->m_bodies, m_next_for_obj) {
        if (ui_sprite_chipmunk_obj_body_add_to_space_i(body, space, transform) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk obj on shape update: body %s add to space fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), body->m_name);
        }
    }
}

void ui_sprite_chipmunk_obj_update_move_policy(ui_sprite_chipmunk_obj_t chipmunk_obj) {
    ui_sprite_chipmunk_module_t module = chipmunk_obj->m_env->m_module;
    ui_sprite_component_t component = ui_sprite_component_from_data(chipmunk_obj);
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);

    assert(ui_sprite_component_is_active(ui_sprite_component_from_data(chipmunk_obj)));

    ui_sprite_component_clear_attr_monitors(component);

    if (chipmunk_obj->m_main_body) {
        if (ui_sprite_component_add_attr_monitor(
                component, "setter.linear_velocity_angle,setter.linear_velocity_value",
                ui_sprite_chipmunk_obj_on_set_linear_velocity, chipmunk_obj)
            == NULL)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk obj enter: add attr set linear velocity montor fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        }
    
        if (ui_sprite_component_add_attr_monitor(
                component, "transform.scale,transform.flip_x,transform.flip_y",
                ui_sprite_chipmunk_obj_on_shape_update, chipmunk_obj)
            == NULL)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk obj enter: add attr shape update montor fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        }

        if (ui_sprite_chipmunk_obj_body_need_update(chipmunk_obj->m_main_body)) {
            ui_sprite_component_sync_update(component, 1);

            if (chipmunk_obj->m_main_body->m_is_in_space && cpBodyGetSpace(&chipmunk_obj->m_main_body->m_body) == NULL) {
                cpSpaceAddBody(
                    (cpSpace*)plugin_chipmunk_env_space(chipmunk_obj->m_env->m_env),
                    &chipmunk_obj->m_main_body->m_body);
                cpBodyActivate(&chipmunk_obj->m_main_body->m_body);
            }
        }
        else {
            ui_sprite_component_sync_update(component, 0);

            if (ui_sprite_component_add_attr_monitor(
                    component, "transform.pos,transform.angle",
                    ui_sprite_chipmunk_obj_on_transform_update, chipmunk_obj)
                == NULL)
            {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): chipmunk obj enter: add attr transform update montor fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            }

                CPE_ERROR(
                    module->m_em, "entity %d(%s): chipmunk obj enter: add attr transform update montor!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            
            if (chipmunk_obj->m_main_body->m_is_in_space && cpBodyGetSpace(&chipmunk_obj->m_main_body->m_body) != NULL) {
                cpSpaceRemoveBody(
                    (cpSpace*)plugin_chipmunk_env_space(chipmunk_obj->m_env->m_env),
                    &chipmunk_obj->m_main_body->m_body);
            }
        }
    }
    else {
        ui_sprite_component_sync_update(component, 0);
    }
}

static void ui_sprite_chipmunk_obj_on_set_linear_velocity(void * ctx) {
    ui_sprite_chipmunk_obj_t chipmunk_obj = (ui_sprite_chipmunk_obj_t)ctx;

    if (chipmunk_obj->m_data.setter.linear_velocity_angle.setted || chipmunk_obj->m_data.setter.linear_velocity_value.setted) {
        cpVect old_v = cpBodyGetVelocity(&chipmunk_obj->m_main_body->m_body);
        float angle;
        float value;
        
        if (chipmunk_obj->m_data.setter.linear_velocity_angle.setted) {
            angle = chipmunk_obj->m_data.setter.linear_velocity_angle.value;
            chipmunk_obj->m_data.setter.linear_velocity_angle.setted = 0;
        }
        else {
            angle = cpe_math_angle(0, 0, old_v.x, old_v.y);
        }

        if (chipmunk_obj->m_data.setter.linear_velocity_value.setted) {
            value = chipmunk_obj->m_data.setter.linear_velocity_value.value;
            chipmunk_obj->m_data.setter.linear_velocity_value.setted = 0;
        }
        else {
            value = cpe_math_distance(0, 0, old_v.x, old_v.y);
        }

        ui_sprite_chipmunk_obj_body_set_linear_velocity(chipmunk_obj->m_main_body, angle, value);
    }
}

uint8_t ui_sprite_chipmunk_obj_is_colllision_with(ui_sprite_chipmunk_obj_t chipmunk_obj, uint32_t mask) {
    cpArbiter *arb;
    ui_sprite_chipmunk_obj_body_t body;

    TAILQ_FOREACH(body, &chipmunk_obj->m_bodies, m_next_for_obj) {
        if (!body->m_is_in_space) continue;

        for(arb = body->m_body.arbiterList; arb; arb = cpArbiterNext(arb, &body->m_body)) {
            if (arb->body_a == &body->m_body) {
                if (arb->b->filter.categories & mask) return 1;
            }
            else {
                assert(arb->body_b == &body->m_body);
                if (arb->a->filter.categories & mask) return 1;
            }
        }
    }

    return 0;
}

uint8_t ui_sprite_chipmunk_obj_is_colllision_with_entity(ui_sprite_chipmunk_obj_t chipmunk_obj, ui_sprite_entity_t entity) {
	ui_sprite_chipmunk_env_t env = chipmunk_obj->m_env;
    cpArbiter *arb;
    ui_sprite_chipmunk_obj_body_t body;

    TAILQ_FOREACH(body, &chipmunk_obj->m_bodies, m_next_for_obj) {
        if (!body->m_is_in_space) continue;

        for(arb = body->m_body.arbiterList; arb; arb = cpArbiterNext(arb, &body->m_body)) {
            ui_sprite_chipmunk_obj_body_t body_o = NULL;

            if (arb->body_a == &body->m_body && arb->b->type == env->m_collision_type) {
              body_o = (ui_sprite_chipmunk_obj_body_t)arb->b->userData;
            }
            else if (arb->a->type == env->m_collision_type) {
                assert(arb->body_b == &body->m_body);
                body_o = (ui_sprite_chipmunk_obj_body_t)arb->a->userData;
            }

            if (body_o && ui_sprite_component_entity(ui_sprite_component_from_data(body_o->m_obj)) == entity) {
              return 1;
            }            
        }
    }

    return 0;
}

ui_sprite_group_t
ui_sprite_chipmunk_obj_find_collision_entities(ui_sprite_chipmunk_obj_t chipmunk_obj, uint32_t mask) {
    ui_sprite_chipmunk_env_t env = chipmunk_obj->m_env;
    ui_sprite_chipmunk_module_t module = env->m_module;
    ui_sprite_component_t component = ui_sprite_component_from_data(chipmunk_obj);
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_sprite_group_t group;
    cpArbiter *arb;
    ui_sprite_chipmunk_obj_body_t body;

    group = ui_sprite_group_create(ui_sprite_entity_world(entity), NULL);
    if (group == NULL) {
        CPE_ERROR(module->m_em, "%s: find collision entities: create group fail", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }
    
    TAILQ_FOREACH(body, &chipmunk_obj->m_bodies, m_next_for_obj) {
        if (!body->m_is_in_space) continue;

        for(arb = body->m_body.arbiterList; arb; arb = cpArbiterNext(arb, &body->m_body)) {
            ui_sprite_chipmunk_obj_body_t body_o = NULL;
            ui_sprite_entity_t chipmunk_entity;
            
            if (arb->body_a == &body->m_body) {
                if (arb->b->type == env->m_collision_type && arb->b->filter.categories & mask) {
                    body_o = (ui_sprite_chipmunk_obj_body_t)arb->b->userData;
                }
            }
            else {
                assert(arb->body_b == &body->m_body);
                if (arb->a->type == env->m_collision_type && arb->a->filter.categories & mask) {
                    body_o = (ui_sprite_chipmunk_obj_body_t)arb->a->userData;
                }
            }

            chipmunk_entity = ui_sprite_component_entity(ui_sprite_component_from_data(body_o->m_obj));
            if (chipmunk_entity != entity) {
                ui_sprite_group_add_entity(group, chipmunk_entity);
            }
        }
    }

    return group;
}

static ui_sprite_chipmunk_obj_body_t ui_sprite_chipmunk_obj_body_next(struct ui_sprite_chipmunk_obj_body_it * it) {
    ui_sprite_chipmunk_obj_body_t * data = (ui_sprite_chipmunk_obj_body_t *)(it->m_data);
    ui_sprite_chipmunk_obj_body_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_obj);

    return r;
}

void ui_sprite_chipmunk_obj_bodies(ui_sprite_chipmunk_obj_body_it_t body_it, ui_sprite_chipmunk_obj_t chipmunk_obj) {
    *(ui_sprite_chipmunk_obj_body_t *)(body_it->m_data) = TAILQ_FIRST(&chipmunk_obj->m_bodies);
    body_it->next = ui_sprite_chipmunk_obj_body_next;
}

struct ui_sprite_chipmunk_obj_constraint_it_data {
    ui_sprite_chipmunk_obj_body_t m_cur_body;
    uint8_t m_step;
    ui_sprite_chipmunk_obj_constraint_t m_next_constraint;
};

static uint8_t ui_sprite_chipmunk_obj_constraint_it_need_process(struct ui_sprite_chipmunk_obj_constraint_it_data * data) {
    assert(data->m_next_constraint);

    if (!data->m_next_constraint->m_is_in_space) return 0;

    if(data->m_next_constraint->m_body_a == NULL || data->m_next_constraint->m_body_b == NULL) return 1;

    if (data->m_next_constraint->m_body_a->m_obj == data->m_next_constraint->m_body_b->m_obj) {
        return data->m_next_constraint->m_body_a == data->m_cur_body ? 1 : 0;
    }
    else {
        return 1;
    }
}

static void ui_sprite_chipmunk_obj_constraint_it_search_next(struct ui_sprite_chipmunk_obj_constraint_it_data * data) {
GO_NEXT:    
    if (data->m_next_constraint == NULL) {
        if (data->m_cur_body == NULL) return;
        
        if (data->m_step == 0) {
            data->m_step = 1;
            data->m_next_constraint = TAILQ_FIRST(&data->m_cur_body->m_constraints_as_a);
        }
        else if (data->m_step == 1) {
            data->m_step = 2;
            data->m_next_constraint = TAILQ_FIRST(&data->m_cur_body->m_constraints_as_b);
        }
        else {
            data->m_cur_body = TAILQ_NEXT(data->m_cur_body, m_next_for_obj);
            data->m_step = 0;
        }
    }
    else {
        assert(data->m_step == 1 || data->m_step == 2);
        
        if (data->m_step == 1) {
            data->m_next_constraint = TAILQ_NEXT(data->m_next_constraint, m_next_for_body_a);
        }
        else {
            data->m_next_constraint = TAILQ_NEXT(data->m_next_constraint, m_next_for_body_b);
        }
    }

    if (data->m_next_constraint == NULL) goto GO_NEXT;

    if (ui_sprite_chipmunk_obj_constraint_it_need_process(data)) return;

    goto GO_NEXT;
}

static ui_sprite_chipmunk_obj_constraint_t ui_sprite_chipmunk_obj_constraint_next(struct ui_sprite_chipmunk_obj_constraint_it * it) {
    struct ui_sprite_chipmunk_obj_constraint_it_data * data = (struct ui_sprite_chipmunk_obj_constraint_it_data *)(it->m_data);
    ui_sprite_chipmunk_obj_constraint_t r;

    r = data->m_next_constraint;

    ui_sprite_chipmunk_obj_constraint_it_search_next(data);

    return r;
}

void ui_sprite_chipmunk_obj_constraints(ui_sprite_chipmunk_obj_constraint_it_t constraint_it, ui_sprite_chipmunk_obj_t chipmunk_obj) {
    struct ui_sprite_chipmunk_obj_constraint_it_data * data = (struct ui_sprite_chipmunk_obj_constraint_it_data *)(constraint_it->m_data);
    data->m_cur_body = TAILQ_FIRST(&chipmunk_obj->m_bodies);
    data->m_step = 0;
    data->m_next_constraint = NULL;
    constraint_it->next = ui_sprite_chipmunk_obj_constraint_next;
    
    ui_sprite_chipmunk_obj_constraint_it_search_next(data);
}

struct ui_sprite_chipmunk_obj_check_in_rect_ctx {
    ui_rect_t m_rect;
    uint8_t m_in_rect;
};
    
static void ui_sprite_chipmunk_obj_check_in_rect(cpBody *body, cpShape *shape, void *data) {
    struct ui_sprite_chipmunk_obj_check_in_rect_ctx * ctx = (struct ui_sprite_chipmunk_obj_check_in_rect_ctx*)data;
    cpBB shapeBB;
    if (ctx->m_in_rect) return;

    shapeBB = cpShapeGetBB(shape);

    if ( ((shapeBB.l >= ctx->m_rect->lt.x && shapeBB.l <= ctx->m_rect->rb.x) || (shapeBB.r >= ctx->m_rect->lt.x && shapeBB.r <= ctx->m_rect->rb.x) )
         && ((shapeBB.t >= ctx->m_rect->lt.y && shapeBB.t <= ctx->m_rect->rb.y) || (shapeBB.b >= ctx->m_rect->lt.y && shapeBB.b <= ctx->m_rect->rb.y) ) )
    {
        ctx->m_in_rect = 1;
    }
}
    
uint8_t ui_sprite_chipmunk_obj_in_rect_bb(ui_sprite_chipmunk_obj_t chipmunk_obj, ui_rect_t rect) {
    ui_sprite_chipmunk_obj_body_t body;
    struct ui_sprite_chipmunk_obj_check_in_rect_ctx ctx;

    ctx.m_rect = rect;
    
    TAILQ_FOREACH(body, &chipmunk_obj->m_bodies, m_next_for_obj) {

        if (!body->m_is_in_space) continue;

        ctx.m_in_rect = 0;
        cpBodyEachShape(&body->m_body, ui_sprite_chipmunk_obj_check_in_rect, &ctx);
        if (ctx.m_in_rect) return 1;
    }

    return 0;
}

int ui_sprite_chipmunk_obj_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_component_meta_t meta;

    meta = ui_sprite_component_meta_create(
        module->m_repo,
        UI_SPRITE_CHIPMUNK_OBJ_NAME, sizeof(struct ui_sprite_chipmunk_obj));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: %s register: meta create fail",
            ui_sprite_chipmunk_module_name(module), UI_SPRITE_CHIPMUNK_OBJ_NAME);
        return -1;
    }

    ui_sprite_component_meta_set_init_fun(meta, ui_sprite_chipmunk_obj_init, module);
    ui_sprite_component_meta_set_copy_fun(meta, ui_sprite_chipmunk_obj_copy, module);
    ui_sprite_component_meta_set_enter_fun(meta, ui_sprite_chipmunk_obj_enter, module);
    ui_sprite_component_meta_set_exit_fun(meta, ui_sprite_chipmunk_obj_exit, module);
    ui_sprite_component_meta_set_free_fun(meta, ui_sprite_chipmunk_obj_fini, module);
    ui_sprite_component_meta_set_update_fun(meta, ui_sprite_chipmunk_obj_update, module);

    ui_sprite_component_meta_set_data_meta(
        meta,
        module->m_meta_chipmunk_obj_data,
        CPE_ENTRY_START(ui_sprite_chipmunk_obj, m_data),
        CPE_ENTRY_SIZE(ui_sprite_chipmunk_obj, m_data));
    
    if (ui_sprite_cfg_loader_add_comp_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_OBJ_NAME, ui_sprite_chipmunk_obj_load, module)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "%s: %s register: register loader fail",
            ui_sprite_chipmunk_module_name(module), UI_SPRITE_CHIPMUNK_OBJ_NAME);
        ui_sprite_component_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_obj_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_component_meta_t meta;

    if ((meta = ui_sprite_component_meta_find(module->m_repo, UI_SPRITE_CHIPMUNK_OBJ_NAME))) {
        ui_sprite_component_meta_free(meta);
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module), UI_SPRITE_CHIPMUNK_OBJ_NAME);
    }

    if (ui_sprite_cfg_loader_remove_comp_loader(module->m_loader, UI_SPRITE_CHIPMUNK_OBJ_NAME) != 0) {
        CPE_ERROR(
            module->m_em, "%s: %s unregister: loader remove fail",
            ui_sprite_chipmunk_module_name(module), UI_SPRITE_CHIPMUNK_OBJ_NAME);
    }
}

const char * UI_SPRITE_CHIPMUNK_OBJ_NAME = "ChipmunkObj";

#ifdef __cplusplus
}
#endif

