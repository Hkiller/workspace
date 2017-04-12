#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "render/utils/ui_vector_2.h"
#include "plugin/chipmunk/plugin_chipmunk_data_body.h"
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "ui_sprite_chipmunk_obj_body_group_i.h"
#include "ui_sprite_chipmunk_obj_shape_group_i.h"
#include "ui_sprite_chipmunk_monitor_binding_i.h"
#include "ui_sprite_chipmunk_monitor_i.h"
#include "ui_sprite_chipmunk_obj_constraint_i.h"
#include "ui_sprite_chipmunk_obj_runtime_group_i.h"
#include "ui_sprite_chipmunk_obj_updator_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static void ui_sprite_chipmunk_obj_body_velocity_update_fun(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt);
    
ui_sprite_chipmunk_obj_body_t
ui_sprite_chipmunk_obj_body_create(ui_sprite_chipmunk_obj_t chipmunk_obj, uint32_t id, const char * name, uint8_t is_runtime) {
    ui_sprite_chipmunk_module_t module = chipmunk_obj->m_env->m_module;
    ui_sprite_chipmunk_obj_body_t body;

    body = (ui_sprite_chipmunk_obj_body_t)mem_calloc(module->m_alloc, sizeof(struct ui_sprite_chipmunk_obj_body));
    if (body == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_body_create: alloc fail!");
        return NULL;
    }

    body->m_obj = chipmunk_obj;
    body->m_load_from = NULL;
    body->m_is_runtime = is_runtime;
    body->m_is_in_space = 0;
    body->m_id = id;
    cpe_str_dup(body->m_name, sizeof(body->m_name), name);
    bzero(&body->m_body_attrs, sizeof(body->m_body_attrs));
    body->m_body_attrs.m_gravity.type = UI_SPRITE_CHIPMUNK_GRAVITY_TYPE_NATIVE;

    TAILQ_INIT(&body->m_constraints_as_a);
    TAILQ_INIT(&body->m_constraints_as_b);
    TAILQ_INIT(&body->m_shape_groups);
    TAILQ_INIT(&body->m_monitor_bindings);
    TAILQ_INIT(&body->m_runtime_groups);
    TAILQ_INIT(&body->m_body_groups);
    
    chipmunk_obj->m_body_count++;
    TAILQ_INSERT_TAIL(&chipmunk_obj->m_bodies, body, m_next_for_obj);
    
    return body;
}

ui_sprite_chipmunk_obj_body_t
ui_sprite_chipmunk_obj_body_clone(ui_sprite_chipmunk_obj_t chipmunk_obj, ui_sprite_chipmunk_obj_body_t from_body) {
    ui_sprite_chipmunk_obj_body_t to_body;
    ui_sprite_chipmunk_obj_shape_group_t from_group;

    to_body = ui_sprite_chipmunk_obj_body_create(chipmunk_obj, from_body->m_id, from_body->m_name, from_body->m_is_runtime);
    if (to_body == NULL) return NULL;

    to_body->m_load_from = from_body->m_load_from;
    to_body->m_is_runtime = from_body->m_is_runtime;
    to_body->m_body_attrs = from_body->m_body_attrs;

    TAILQ_FOREACH(from_group, &from_body->m_shape_groups, m_next_for_body) {
        if (ui_sprite_chipmunk_obj_shape_group_clone(to_body, from_group) == NULL) {
            ui_sprite_chipmunk_obj_body_free(to_body);
            return NULL;
        }
    }

    return to_body;
}

ui_sprite_chipmunk_obj_body_t
ui_sprite_chipmunk_obj_body_create_from_data(ui_sprite_chipmunk_obj_t chipmunk_obj, plugin_chipmunk_data_body_t data_body, uint8_t is_runtime) {
    CHIPMUNK_BODY const * body_data = plugin_chipmunk_data_body_data(data_body);
    ui_sprite_chipmunk_obj_body_t body;
    struct plugin_chipmunk_data_fixture_it fixture_it;
    plugin_chipmunk_data_fixture_t fixture;

    body = ui_sprite_chipmunk_obj_body_create(chipmunk_obj, body_data->id, body_data->name, is_runtime);
    if (body == NULL) return NULL;

    body->m_body_attrs.m_type = (chipmunk_obj_type_t)body_data->type;
    body->m_body_attrs.m_attr_flags |= CHIPMUNK_BODY_ATTR_ID_TYPE;

    body->m_body_attrs.m_position.x = body_data->anchorpoint.x;
    body->m_body_attrs.m_position.y = body_data->anchorpoint.y;
    
    body->m_load_from = data_body;
    body->m_is_runtime = is_runtime;

    //CHIPMUNK_PAIR anchorpoint;
    plugin_chipmunk_data_body_fixtures(&fixture_it, data_body);
    while((fixture = plugin_chipmunk_data_fixture_it_next(&fixture_it))) {
        if (ui_sprite_chipmunk_obj_shape_group_create_from_data(body, fixture) == NULL) {
            ui_sprite_chipmunk_obj_body_free(body);
            return NULL;
        }
    }

    return body;
}
    
void ui_sprite_chipmunk_obj_body_free(ui_sprite_chipmunk_obj_body_t body) {
    ui_sprite_chipmunk_obj_t chipmunk_obj = body->m_obj;
    ui_sprite_chipmunk_module_t module = chipmunk_obj->m_env->m_module;

    if (body->m_is_in_space) {
        ui_sprite_chipmunk_obj_body_remove_from_space(body);
    }

    assert(!body->m_is_in_space);

    if (chipmunk_obj->m_main_body == body) {
        ui_sprite_chipmunk_obj_body_set_is_main(body, 0);
    }
    
    assert(chipmunk_obj->m_main_body != body);
    assert(TAILQ_EMPTY(&body->m_monitor_bindings));

    while(!TAILQ_EMPTY(&body->m_constraints_as_a)) {
        ui_sprite_chipmunk_obj_constraint_free(TAILQ_FIRST(&body->m_constraints_as_a));
    }

    while(!TAILQ_EMPTY(&body->m_constraints_as_b)) {
        ui_sprite_chipmunk_obj_constraint_free(TAILQ_FIRST(&body->m_constraints_as_b));
    }

    while(!TAILQ_EMPTY(&body->m_shape_groups)) {
        ui_sprite_chipmunk_obj_shape_group_free(TAILQ_FIRST(&body->m_shape_groups));
    }

    while(!TAILQ_EMPTY(&body->m_runtime_groups)) {
        ui_sprite_chipmunk_obj_runtime_group_unbind(TAILQ_FIRST(&body->m_runtime_groups), body);
    }

    chipmunk_obj->m_body_count--;
    TAILQ_REMOVE(&chipmunk_obj->m_bodies, body, m_next_for_obj);

    while(!TAILQ_EMPTY(&body->m_body_groups)) {
        ui_sprite_chipmunk_obj_body_group_binding_free(TAILQ_FIRST(&body->m_body_groups));
    }

    mem_free(module->m_alloc, body);
}

chipmunk_obj_type_t ui_sprite_chipmunk_obj_body_type(ui_sprite_chipmunk_obj_body_t chipmunk_obj_body) {
    return (chipmunk_obj_type_t)cpBodyGetType(&chipmunk_obj_body->m_body);
}

int ui_sprite_chipmunk_obj_body_set_type(ui_sprite_chipmunk_obj_body_t body, chipmunk_obj_type_t obj_type) {
    body->m_body_attrs.m_type = obj_type;
    body->m_body_attrs.m_attr_flags |= (0x1u << CHIPMUNK_BODY_ATTR_ID_TYPE);
    if (body->m_is_in_space) {
        cpBodySetType(&body->m_body, (cpBodyType)obj_type);
    }

    return 0;
}
    
void ui_sprite_chipmunk_obj_body_set_linear_velocity(ui_sprite_chipmunk_obj_body_t body, float angle, float velocity) {
    if (velocity == 0) {
        cpBodySetVelocity(&body->m_body, cpvzero);
    }
    else {
        float radians = cpe_math_angle_to_radians(angle);
        cpBodySetVelocity(&body->m_body, cpv(velocity * cpe_cos_radians(radians), velocity * cpe_sin_radians(radians)));
    }
}

void ui_sprite_chipmunk_obj_body_set_linear_velocity_pair(ui_sprite_chipmunk_obj_body_t body, ui_vector_2 const * velocity_pair) {
    cpBodySetVelocity(&body->m_body, cpv(velocity_pair->x, velocity_pair->y));
}

ui_vector_2 ui_sprite_chipmunk_obj_body_linear_velocity_pair(ui_sprite_chipmunk_obj_body_t body) {
    cpVect v = cpBodyGetVelocity(&body->m_body);
    ui_vector_2 r = UI_VECTOR_2_INITLIZER( v.x, v.y );
    return r;
}

float ui_sprite_chipmunk_obj_body_linear_velocity(ui_sprite_chipmunk_obj_body_t body) {
    cpVect v = cpBodyGetVelocity(&body->m_body);
    return cpe_math_distance(0, 0, v.x, v.y);
}
    
float ui_sprite_chipmunk_obj_body_linear_velocity_angle(ui_sprite_chipmunk_obj_body_t body) {
    cpVect v = cpBodyGetVelocity(&body->m_body);

    if (v.x == 0.0f && v.y == 0.0f) return 0.0f;

    return cpe_math_angle(0, 0, v.x, v.y);
}

ui_vector_2 ui_sprite_chipmunk_obj_body_world_pos(ui_sprite_chipmunk_obj_body_t body) {
    cpVect p = cpBodyGetPosition(&body->m_body);
    ui_vector_2 r = UI_VECTOR_2_INITLIZER( p.x, p.y );
    return r;
}

int ui_sprite_chipmunk_obj_body_add_to_space(ui_sprite_chipmunk_obj_body_t body) {
    cpSpace * space = (cpSpace * )plugin_chipmunk_env_space(body->m_obj->m_env->m_env);
    ui_sprite_entity_t entity = ui_sprite_component_entity(ui_sprite_component_from_data(body->m_obj));
    ui_sprite_2d_transform_t transform;

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            body->m_obj->m_env->m_module->m_em,
            "entity %d(%s): chipmunk body add to space: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    return ui_sprite_chipmunk_obj_body_add_to_space_i(body, space, transform);
}

int ui_sprite_chipmunk_obj_body_set_local_pos(ui_sprite_chipmunk_obj_body_t body, ui_vector_2_t pos) {
    ui_sprite_entity_t entity;
    ui_sprite_2d_transform_t transform;
    ui_vector_2 entity_pos;
    ui_vector_2 adj_pos;
    
    body->m_body_attrs.m_position.x = pos->x;
    body->m_body_attrs.m_position.y = pos->y;

    if (!body->m_is_in_space) return 0;
    
    entity = ui_sprite_component_entity(ui_sprite_component_from_data(body->m_obj));

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            body->m_obj->m_env->m_module->m_em,
            "entity %d(%s): chipmunk body set local pos: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    entity_pos = ui_sprite_2d_transform_origin_pos(transform);
    
    adj_pos.x = pos->x;
    adj_pos.y = pos->y;
    adj_pos = ui_sprite_2d_transform_adj_local_pos(transform, adj_pos, UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL);

    cpBodySetPosition(&body->m_body, cpv(entity_pos.x + adj_pos.x, entity_pos.y + adj_pos.y));

    return 0;
}
    
int ui_sprite_chipmunk_obj_body_set_local_angle(ui_sprite_chipmunk_obj_body_t body, float angle) {
    ui_sprite_entity_t entity;
    ui_sprite_2d_transform_t transform;

    body->m_body_attrs.m_angle = angle;
    
    if (!body->m_is_in_space) return 0;
    
    entity = ui_sprite_component_entity(ui_sprite_component_from_data(body->m_obj));

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            body->m_obj->m_env->m_module->m_em,
            "entity %d(%s): chipmunk body set local pos: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    cpBodySetAngle(&body->m_body, cpe_math_angle_to_radians(ui_sprite_2d_transform_angle(transform) + angle));

    return 0;
}
    
int ui_sprite_chipmunk_obj_body_add_to_space_i(
    ui_sprite_chipmunk_obj_body_t body, cpSpace * space, ui_sprite_2d_transform_t transform)
{
    //ui_sprite_chipmunk_module_t module = body->m_obj->m_env->m_module;
    ui_sprite_chipmunk_obj_shape_group_t group;
    ui_sprite_chipmunk_obj_constraint_t constraint;
    ui_sprite_chipmunk_monitor_t monitor;
    ui_vector_2 entity_pos;
    float mass = 0.0f;
    float moment = 0.0f;
    ui_vector_2 adj_pos;
    
    assert(!body->m_is_in_space);

    if (body->m_body_attrs.m_attr_flags | (0x1u << CHIPMUNK_BODY_ATTR_ID_MASS)) {
        mass = body->m_body_attrs.m_mass;
    }

    if (body->m_body_attrs.m_attr_flags | (0x1u << CHIPMUNK_BODY_ATTR_ID_MOMENT)) {
        moment = body->m_body_attrs.m_moment;
    }

    bzero(&body->m_body, sizeof(body->m_body));
    cpBodyInit(&body->m_body, mass, moment);
    body->m_body.userData = body;

    if (body->m_body_attrs.m_attr_flags | 0x1u << CHIPMUNK_BODY_ATTR_ID_TYPE) {
        cpBodySetType(&body->m_body, (cpBodyType)body->m_body_attrs.m_type);
    }

    entity_pos = ui_sprite_2d_transform_origin_pos(transform);

    adj_pos.x = body->m_body_attrs.m_position.x;
    adj_pos.y = body->m_body_attrs.m_position.y;
    adj_pos = ui_sprite_2d_transform_adj_local_pos(transform, adj_pos, UI_SPRITE_2D_TRANSFORM_POS_ADJ_ALL);

    cpBodySetPosition(&body->m_body, cpv(entity_pos.x + adj_pos.x, entity_pos.y + adj_pos.y));
    cpBodySetAngle(&body->m_body, cpe_math_angle_to_radians(body->m_body_attrs.m_angle + ui_sprite_2d_transform_angle(transform)));

    if (ui_sprite_chipmunk_obj_body_need_update(body)) {
        cpSpaceAddBody(space, &body->m_body);
    }
    body->m_is_in_space = 1;

    TAILQ_FOREACH(group, &body->m_shape_groups, m_next_for_body) {
        if (ui_sprite_chipmunk_obj_shape_group_init_shape(group, space, transform) != 0) {
            ui_sprite_chipmunk_obj_body_remove_from_space(body);
            return -1;
        }
    }

    if (cpBodyGetType(&body->m_body) == CP_BODY_TYPE_DYNAMIC) {
        if (mass == 0.0f) {
            cpBodyAccumulateMassFromShapes(&body->m_body);
            assert(body->m_body.m != 0.0f);
        }
    }

    cpBodySetVelocityUpdateFunc(&body->m_body,  ui_sprite_chipmunk_obj_body_velocity_update_fun);
    
    TAILQ_FOREACH(constraint, &body->m_constraints_as_a, m_next_for_body_a) {
        assert(!constraint->m_is_in_space);

        if (constraint->m_body_b && !constraint->m_body_b->m_is_in_space) continue;

        if (ui_sprite_chipmunk_obj_constraint_add_to_space(constraint, space) != 0) {
            ui_sprite_chipmunk_obj_body_remove_from_space(body);
            return -1;
        }
    }

    TAILQ_FOREACH(constraint, &body->m_constraints_as_b, m_next_for_body_b) {
        assert(!constraint->m_is_in_space);

        if (constraint->m_body_a && !constraint->m_body_a->m_is_in_space) continue;

        if (ui_sprite_chipmunk_obj_constraint_add_to_space(constraint, space) != 0) {
            ui_sprite_chipmunk_obj_body_remove_from_space(body);
            return -1;
        }
    }

    TAILQ_FOREACH(monitor, &body->m_obj->m_monitors, m_next_for_obj) {
        if (monitor->m_bodies == NULL) {
            if (ui_sprite_chipmunk_monitor_binding_create(body, monitor) == NULL) {
                ui_sprite_chipmunk_obj_body_remove_from_space(body);
                return -1;
            }
        }
        else {
            if (cpe_str_is_in_list(body->m_name, monitor->m_bodies, ',')) {
                if (ui_sprite_chipmunk_monitor_binding_create(body, monitor) == NULL) {
                    ui_sprite_chipmunk_obj_body_remove_from_space(body);
                    return -1;
                }
            }
        }
    }
    
    if (body->m_body_attrs.m_is_main) {
        if (ui_sprite_component_obj_set_main_body(body->m_obj, body) != 0) {
            ui_sprite_chipmunk_obj_body_remove_from_space(body);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_chipmunk_obj_body_remove_from_space(ui_sprite_chipmunk_obj_body_t body)  {
    ui_sprite_chipmunk_obj_shape_group_t group;
    ui_sprite_chipmunk_obj_constraint_t constraint;

    assert(body->m_is_in_space);

    TAILQ_FOREACH(constraint, &body->m_constraints_as_a, m_next_for_body_a) {
        if(!constraint->m_is_in_space) continue;
        ui_sprite_chipmunk_obj_constraint_remove_from_space(constraint);
    }

    TAILQ_FOREACH(constraint, &body->m_constraints_as_b, m_next_for_body_b) {
        if(!constraint->m_is_in_space) continue;
        ui_sprite_chipmunk_obj_constraint_remove_from_space(constraint);
    }
    
    if(cpBodyGetSpace(&body->m_body)) {
        cpSpaceRemoveBody(cpBodyGetSpace(&body->m_body), &body->m_body);
        assert(body->m_body.space == NULL);
    }

    while(!TAILQ_EMPTY(&body->m_monitor_bindings)) {
        ui_sprite_chipmunk_monitor_binding_free(TAILQ_FIRST(&body->m_monitor_bindings));
    }
    
    TAILQ_FOREACH(group, &body->m_shape_groups, m_next_for_body) {
        ui_sprite_chipmunk_obj_shape_group_fini_shape(group);
    }

    cpBodyDestroy(&body->m_body);

    body->m_is_in_space = 0;

    if (body->m_obj->m_main_body == body) {
        ui_sprite_component_obj_set_main_body(body->m_obj, NULL);
    }
}

const char * ui_sprite_chipmunk_obj_body_name(ui_sprite_chipmunk_obj_body_t body) {
    return body->m_name;
}

void * ui_sprite_chipmunk_obj_body_cp_body(ui_sprite_chipmunk_obj_body_t body) {
    return &body->m_body;
}

ui_sprite_chipmunk_obj_t ui_sprite_chipmunk_obj_body_obj(ui_sprite_chipmunk_obj_body_t body) {
    return body->m_obj;
}

uint8_t ui_sprite_chipmunk_obj_body_is_in_space(ui_sprite_chipmunk_obj_body_t body) {
    return body->m_is_in_space;
}

uint8_t ui_sprite_chipmunk_obj_body_is_runtime(ui_sprite_chipmunk_obj_body_t body) {
    return body->m_is_runtime;
}

uint8_t ui_sprite_chipmunk_obj_body_is_main(ui_sprite_chipmunk_obj_body_t body) {
    return body == body->m_obj->m_main_body ? 1 : 0;
}

int ui_sprite_chipmunk_obj_body_set_is_main(ui_sprite_chipmunk_obj_body_t body, uint8_t is_main) {
    body->m_body_attrs.m_is_main = is_main;

    if (ui_sprite_component_is_active(ui_sprite_component_from_data(body->m_obj))) {
        if (is_main && body->m_obj->m_main_body != body) {
            return ui_sprite_component_obj_set_main_body(body->m_obj, body);
        }

        if (!is_main && body->m_obj->m_main_body == body) {
            return ui_sprite_component_obj_set_main_body(body->m_obj, NULL);
        }
    }

    return 0;
}

uint8_t ui_sprite_chipmunk_obj_body_runing_mode(ui_sprite_chipmunk_obj_body_t body) {
    return body->m_body_attrs.m_runing_mode;
}

int ui_sprite_chipmunk_obj_body_set_runing_mode(ui_sprite_chipmunk_obj_body_t body, uint8_t runing_mode) {
    body->m_body_attrs.m_runing_mode = (ui_sprite_chipmunk_runing_mode_t)runing_mode;

    if (ui_sprite_component_is_active(ui_sprite_component_from_data(body->m_obj))) {
        if (body->m_obj->m_main_body != body) {
            ui_sprite_chipmunk_obj_update_move_policy(body->m_obj);
        }
    }

    return 0;
}
    
ui_sprite_chipmunk_obj_body_t
ui_sprite_chipmunk_obj_body_find(ui_sprite_chipmunk_obj_t obj, const char * name) {
    ui_sprite_chipmunk_obj_body_t body;

    if (name == NULL || name[0] == 0) return obj->m_main_body;
    
    TAILQ_FOREACH(body, &obj->m_bodies, m_next_for_obj) {
        if (strcmp(body->m_name, name) == 0) return body;
    }

    return NULL;
}

UI_SPRITE_CHIPMUNK_GRAVITY const * ui_sprite_chipmunk_obj_body_gravity(ui_sprite_chipmunk_obj_body_t body) {
    return &body->m_body_attrs.m_gravity;
}

static void ui_sprite_chipmunk_obj_body_velocity_update_fun(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt) {
    ui_sprite_chipmunk_obj_body_t obj_body = (ui_sprite_chipmunk_obj_body_t)body->userData;

    switch(obj_body->m_body_attrs.m_gravity.type) {
    case UI_SPRITE_CHIPMUNK_GRAVITY_TYPE_NATIVE:
        break;
    case UI_SPRITE_CHIPMUNK_GRAVITY_TYPE_FIX_VALUE:
        gravity.x = obj_body->m_body_attrs.m_gravity.data.fix_value.gravity.x;
        gravity.y = obj_body->m_body_attrs.m_gravity.data.fix_value.gravity.y;
        break;
    case UI_SPRITE_CHIPMUNK_GRAVITY_TYPE_FIX_SIZE_VALUE: {
        cpVect v = cpBodyGetVelocity(body);
        float speed_raduas = cpe_math_radians(0, 0, v.x, v.y);
        gravity.x = obj_body->m_body_attrs.m_gravity.data.fix_size_value.gravity * cpe_cos_radians(speed_raduas);
        gravity.y = obj_body->m_body_attrs.m_gravity.data.fix_size_value.gravity * cpe_sin_radians(speed_raduas);
        break;
    }
    case UI_SPRITE_CHIPMUNK_GRAVITY_TYPE_ADJ_VALUE: {
        if (obj_body->m_body_attrs.m_gravity.data.adj_value.adj_value > 0.0f) {
            float gravity_raduas = cpe_math_radians(0, 0, gravity.x, gravity.y);
            float gravity_value = cpe_math_distance(0, 0, gravity.x, gravity.y)
                * obj_body->m_body_attrs.m_gravity.data.adj_value.adj_value;
            gravity.x = gravity_value * cpe_cos_radians(gravity_raduas);
            gravity.y = gravity_value * cpe_sin_radians(gravity_raduas);
        }
        else {
            gravity.x = gravity.y = 0.0f;
        }
        break;
    }
    default:
        break;
    }

    if (ui_sprite_chipmunk_obj_body_is_main(obj_body)) {
        ui_sprite_chipmunk_obj_updator_t updator;

        TAILQ_FOREACH(updator, &obj_body->m_obj->m_updators, m_next_for_obj) {
            UI_SPRITE_CHIPMUNK_PAIR addition_gravity = { 0.0f, 0.0f };
            float addition_damping = 1.0f;

            assert(updator->m_update_fun);
            updator->m_update_fun(updator, obj_body, &addition_gravity, &addition_damping);

            gravity.x += addition_gravity.x;
            gravity.y += addition_gravity.y;
            damping *= addition_damping;
        }
    }

    cpBodyUpdateVelocity(body, gravity, damping, dt);
}

void ui_sprite_chipmunk_obj_body_set_gravity(ui_sprite_chipmunk_obj_body_t body, UI_SPRITE_CHIPMUNK_GRAVITY const * gravity) {
    body->m_body_attrs.m_gravity = *gravity;
}
    
void ui_sprite_chipmunk_obj_body_set_collision_category(ui_sprite_chipmunk_obj_body_t body, uint32_t category) {
    body->m_body_attrs.m_attr_flags |= 0x1u << CHIPMUNK_BODY_ATTR_ID_CATEGORY;
    body->m_body_attrs.m_category = category;
    ui_sprite_chipmunk_obj_body_update_collision(body);
}
    
void ui_sprite_chipmunk_obj_body_set_collision_mask(ui_sprite_chipmunk_obj_body_t body, uint32_t mask) {
    body->m_body_attrs.m_attr_flags |= 0x1u << CHIPMUNK_BODY_ATTR_ID_MASK;
    body->m_body_attrs.m_mask = mask;
    ui_sprite_chipmunk_obj_body_update_collision(body);
}

void ui_sprite_chipmunk_obj_body_visit_shapes(
    ui_sprite_chipmunk_obj_body_t chipmunk_obj_body, ui_sprite_chipmunk_obj_shape_visit_fun_t fun, void * ctx)
{
    ui_sprite_chipmunk_obj_shape_group_t shape_group;

    TAILQ_FOREACH(shape_group, &chipmunk_obj_body->m_shape_groups, m_next_for_body) {
        ui_sprite_chipmunk_obj_shape_group_visit_shapes(shape_group, fun, ctx);
    }
}

struct update_collision_ctx {
    uint8_t m_collision_group;
    uint32_t m_collision_category;
    uint32_t m_collision_mask;
};
    
void ui_sprite_chipmunk_obj_body_collision_info(
    ui_sprite_chipmunk_obj_body_t body, uint32_t * category, uint32_t * mask, uint32_t * group_id)
{
    if (body->m_body_attrs.m_attr_flags | 0x1u << CHIPMUNK_BODY_ATTR_ID_CATEGORY) {
        *category = body->m_body_attrs.m_category;
    }
    else {
        *category = 0;
    }
    
    if (body->m_body_attrs.m_attr_flags | 0x1u << CHIPMUNK_BODY_ATTR_ID_MASK) {
        *mask = body->m_body_attrs.m_mask;
    }
    else {
        *mask = 0;
    }

    if (!TAILQ_EMPTY(&body->m_runtime_groups)) {
        *group_id = TAILQ_FIRST(&body->m_runtime_groups)->m_group_id;
    }
    else if (body->m_body_attrs.m_attr_flags | 0x1u << CHIPMUNK_BODY_ATTR_ID_GROUP) {
        *group_id = body->m_body_attrs.m_group;
    }
    else {
        *group_id = 0;
    }
}

static void ui_sprite_chipmunk_obj_body_do_update_collision(ui_sprite_chipmunk_obj_shape_t shape, void * input_ctx) {
    struct update_collision_ctx * ctx = (struct update_collision_ctx *)input_ctx;

    cpShapeSetFilter(
        (cpShape*)shape,
        cpShapeFilterNew(
            ctx->m_collision_group ? ctx->m_collision_group : shape->m_fixture_data->collision_group,
            shape->m_fixture_data->collision_category | ctx->m_collision_category,
            shape->m_fixture_data->collision_mask | ctx->m_collision_mask));
}
    
void ui_sprite_chipmunk_obj_body_update_collision(ui_sprite_chipmunk_obj_body_t body) {
    if (body->m_is_in_space) {
        struct update_collision_ctx ctx;
        ui_sprite_chipmunk_obj_body_collision_info(body, &ctx.m_collision_category, &ctx.m_collision_mask, (uint32_t *)&ctx.m_collision_group);
        ui_sprite_chipmunk_obj_body_visit_shapes(body, ui_sprite_chipmunk_obj_body_do_update_collision, &ctx);
    }
}

uint8_t ui_sprite_chipmunk_obj_body_need_update(ui_sprite_chipmunk_obj_body_t body) {
    return (body->m_body_attrs.m_runing_mode == ui_sprite_chipmunk_runing_mode_active
        && ui_sprite_chipmunk_obj_body_type(body) != chipmunk_obj_type_static)
        ? 1
        : 0;
}    

#ifdef __cplusplus
}
#endif

