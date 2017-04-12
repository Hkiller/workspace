#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_chipmunk_obj_constraint_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_obj_constraint_t
ui_sprite_chipmunk_obj_constraint_create(
    ui_sprite_chipmunk_obj_body_t body_a, ui_sprite_chipmunk_obj_body_t body_b,
    const char * name, uint8_t constraint_type, CHIPMUNK_CONSTRAINT_DATA const * constraint_data, uint8_t is_runtime)
{
	ui_sprite_chipmunk_module_t module = body_a->m_obj->m_env->m_module;
	ui_sprite_chipmunk_obj_constraint_t constraint;

	constraint = (ui_sprite_chipmunk_obj_constraint_t)mem_calloc(module->m_alloc, sizeof(struct ui_sprite_chipmunk_obj_constraint));
	if (constraint == NULL) {
		CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_constraint_create: alloc fail!");
		return NULL;
	}

	cpe_str_dup(constraint->m_name, sizeof(constraint->m_name), name);

	constraint->m_body_a = body_a;
	if (constraint->m_body_a) {
		TAILQ_INSERT_TAIL(&constraint->m_body_a->m_constraints_as_a, constraint, m_next_for_body_a);
	}

	constraint->m_body_b = body_b;
	if (constraint->m_body_b) {
		TAILQ_INSERT_TAIL(&constraint->m_body_b->m_constraints_as_b, constraint, m_next_for_body_b);
	}

    constraint->m_creator = NULL;
    constraint->m_is_runtime = is_runtime;
    constraint->m_is_in_space = 0;
    constraint->m_error_bias = 0.0f;
	constraint->m_max_force = 0.0f;
	constraint->m_max_bias = 0.0f;

    constraint->m_constraint_type = constraint_type;
    constraint->m_constraint_data = *constraint_data;
    
    return constraint;
}

void ui_sprite_chipmunk_obj_constraint_free(ui_sprite_chipmunk_obj_constraint_t constraint) {
    ui_sprite_chipmunk_module_t module = constraint->m_body_a->m_obj->m_env->m_module;

    assert(!constraint->m_is_in_space);

    if (constraint->m_body_a) {
        TAILQ_REMOVE(&constraint->m_body_a->m_constraints_as_a, constraint, m_next_for_body_a);
    }

    if (constraint->m_body_b) {
        TAILQ_REMOVE(&constraint->m_body_b->m_constraints_as_b, constraint, m_next_for_body_b);
    }
    
    mem_free(module->m_alloc, constraint);
}

const char * ui_sprite_chipmunk_obj_constraint_name(ui_sprite_chipmunk_obj_constraint_t constraint) {
    return constraint->m_name;
}

uint8_t ui_sprite_chipmunk_obj_constraint_type(ui_sprite_chipmunk_obj_constraint_t constraint) {
    return constraint->m_constraint_type;
}
    
CHIPMUNK_CONSTRAINT_DATA * ui_sprite_chipmunk_obj_constraint_data(ui_sprite_chipmunk_obj_constraint_t constraint) {
    return &constraint->m_constraint_data;
}

ui_sprite_chipmunk_obj_constraint_t
ui_sprite_chipmunk_obj_constraint_find(ui_sprite_chipmunk_obj_body_t body, const char * name) {
    ui_sprite_chipmunk_obj_constraint_t constraint;

    constraint = ui_sprite_chipmunk_obj_constraint_find_as_a(body, name);
    if (constraint) return constraint;

    constraint = ui_sprite_chipmunk_obj_constraint_find_as_b(body, name);
    if (constraint) return constraint;

    return NULL;
}
    
ui_sprite_chipmunk_obj_constraint_t ui_sprite_chipmunk_obj_constraint_find_as_a(ui_sprite_chipmunk_obj_body_t body, const char * name) {
    ui_sprite_chipmunk_obj_constraint_t constraint;
    
    TAILQ_FOREACH(constraint, &body->m_constraints_as_a, m_next_for_body_a) {
        if (strcmp(constraint->m_name, name) == 0) return constraint;
    }

    return NULL;
}
        
ui_sprite_chipmunk_obj_constraint_t ui_sprite_chipmunk_obj_constraint_find_as_b(ui_sprite_chipmunk_obj_body_t body, const char * name) {
    ui_sprite_chipmunk_obj_constraint_t constraint;
    
    TAILQ_FOREACH(constraint, &body->m_constraints_as_b, m_next_for_body_b) {
        if (strcmp(constraint->m_name, name) == 0) return constraint;
    }

    return NULL;
}

int ui_sprite_chipmunk_obj_constraint_add_to_space(
    ui_sprite_chipmunk_obj_constraint_t constraint, cpSpace * space)
{
    ui_sprite_chipmunk_module_t module = constraint->m_body_a->m_obj->m_env->m_module;

    ui_sprite_entity_t entity_a =
        constraint->m_body_a
        ? ui_sprite_component_entity(ui_sprite_component_from_data(constraint->m_body_a->m_obj))
        : NULL;
    ui_sprite_2d_transform_t transform_a = entity_a ? ui_sprite_2d_transform_find(entity_a) : NULL;
    
    ui_sprite_entity_t entity_b =
        constraint->m_body_b
        ? ui_sprite_component_entity(ui_sprite_component_from_data(constraint->m_body_b->m_obj))
        : NULL;
    ui_sprite_2d_transform_t transform_b = entity_b ? ui_sprite_2d_transform_find(entity_b) : NULL;

    uint8_t flip_type = UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_FLIP | UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_SCALE;
    
    assert(!constraint->m_is_in_space);

    if (constraint->m_body_a == NULL && constraint->m_body_b == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_chipmunk_obj_constraint_add_to_space: no any body!");
        return -1;
    }

    if (constraint->m_body_a && !constraint->m_body_a->m_is_in_space) {
        CPE_ERROR(
            module->m_em, "ui_sprite_chipmunk_obj_constraint_add_to_space: body a %s not in space!",
            constraint->m_body_a->m_name);
        return -1;
    }

    if (constraint->m_body_b && !constraint->m_body_b->m_is_in_space) {
        CPE_ERROR(
            module->m_em, "ui_sprite_chipmunk_obj_constraint_add_to_space: body b %s not in space!",
            constraint->m_body_b->m_name);
        return -1;
    }

    switch(constraint->m_constraint_type) {
    case chipmunk_constraint_type_pivot_joint: {
        ui_vector_2 anchor_a = UI_VECTOR_2_INITLIZER( constraint->m_constraint_data.pivot_joint.anchor_a.x, constraint->m_constraint_data.pivot_joint.anchor_a.y );
        ui_vector_2 anchor_b = UI_VECTOR_2_INITLIZER( constraint->m_constraint_data.pivot_joint.anchor_b.x, constraint->m_constraint_data.pivot_joint.anchor_b.y );

        if (transform_a) anchor_a = ui_sprite_2d_transform_adj_local_pos(transform_a, anchor_a, flip_type);
        if (transform_b) anchor_b = ui_sprite_2d_transform_adj_local_pos(transform_b, anchor_b, flip_type);

        if (cpPivotJointInit(
                &constraint->m_pivot,
                constraint->m_body_a ? &constraint->m_body_a->m_body : cpSpaceGetStaticBody(space),
                constraint->m_body_b ? &constraint->m_body_b->m_body : cpSpaceGetStaticBody(space),
                cpv(anchor_a.x, anchor_a.y), cpv(anchor_b.x, anchor_b.y))
            == NULL)
        {
            CPE_ERROR(
                module->m_em, "ui_sprite_chipmunk_obj_constraint_add_to_space: init pivot constraint fail!");
            return -1;
        }

        break;
    }
    case chipmunk_constraint_type_pin_joint: {
        ui_vector_2 anchor_a = UI_VECTOR_2_INITLIZER(constraint->m_constraint_data.pin_joint.anchor_a.x, constraint->m_constraint_data.pin_joint.anchor_a.y );
        ui_vector_2 anchor_b = UI_VECTOR_2_INITLIZER(constraint->m_constraint_data.pin_joint.anchor_b.x, constraint->m_constraint_data.pin_joint.anchor_b.y);

        if (transform_a) anchor_a = ui_sprite_2d_transform_adj_local_pos(transform_a, anchor_a, flip_type);
        if (transform_b) anchor_b = ui_sprite_2d_transform_adj_local_pos(transform_b, anchor_b, flip_type);
        
        if (cpPinJointInit(
                &constraint->m_pin,
                constraint->m_body_a ? &constraint->m_body_a->m_body : cpSpaceGetStaticBody(space),
                constraint->m_body_b ? &constraint->m_body_b->m_body : cpSpaceGetStaticBody(space),
                cpv(anchor_a.x, anchor_a.y), cpv(anchor_b.x, anchor_b.y))
            == NULL)
        {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_constraint_add_to_space: init pin constraint fail!");
            return -1;
        }

        break;
    }
    case chipmunk_constraint_type_slide_joint: {
        ui_vector_2 anchor_a = UI_VECTOR_2_INITLIZER(constraint->m_constraint_data.slide_joint.anchor_a.x, constraint->m_constraint_data.slide_joint.anchor_a.y);
        ui_vector_2 anchor_b = UI_VECTOR_2_INITLIZER(constraint->m_constraint_data.slide_joint.anchor_b.x, constraint->m_constraint_data.slide_joint.anchor_b.y);

        if (transform_a) anchor_a = ui_sprite_2d_transform_adj_local_pos(transform_a, anchor_a, flip_type);
        if (transform_b) anchor_b = ui_sprite_2d_transform_adj_local_pos(transform_b, anchor_b, flip_type);
        
        if (cpSlideJointInit(
                &constraint->m_slide,
                constraint->m_body_a ? &constraint->m_body_a->m_body : cpSpaceGetStaticBody(space),
                constraint->m_body_b ? &constraint->m_body_b->m_body : cpSpaceGetStaticBody(space),
                cpv(anchor_a.x, anchor_a.y), cpv(anchor_b.x, anchor_b.y),
                constraint->m_constraint_data.slide_joint.min, constraint->m_constraint_data.slide_joint.max)
            == NULL)
        {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_constraint_add_to_space: init slide constraint fail!");
            return -1;
        }

        break;
    }
    case chipmunk_constraint_type_groove_joint: {
        ui_vector_2 groove_a = UI_VECTOR_2_INITLIZER(constraint->m_constraint_data.groove_joint.groove_a.x, constraint->m_constraint_data.groove_joint.groove_a.y);
        ui_vector_2 groove_b = UI_VECTOR_2_INITLIZER(constraint->m_constraint_data.groove_joint.groove_b.x, constraint->m_constraint_data.groove_joint.groove_b.y);
        ui_vector_2 anchor_b = UI_VECTOR_2_INITLIZER(constraint->m_constraint_data.groove_joint.anchor_b.x, constraint->m_constraint_data.groove_joint.anchor_b.y);
        
        if (transform_a) groove_a = ui_sprite_2d_transform_adj_local_pos(transform_a, groove_a, flip_type);
        if (transform_b) groove_b = ui_sprite_2d_transform_adj_local_pos(transform_b, groove_b, flip_type);
        if (transform_b) anchor_b = ui_sprite_2d_transform_adj_local_pos(transform_b, anchor_b, flip_type);
        
        if (cpGrooveJointInit(
                &constraint->m_groove,
                constraint->m_body_a ? &constraint->m_body_a->m_body : cpSpaceGetStaticBody(space),
                constraint->m_body_b ? &constraint->m_body_b->m_body : cpSpaceGetStaticBody(space),
                cpv(groove_a.x, groove_a.y), cpv(groove_b.x, groove_b.y),
                cpv(anchor_b.x, anchor_b.y))
            == NULL)
        {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_constraint_add_to_space: init groove constraint fail!");
            return -1;
        }

        break;
    }
    case chipmunk_constraint_type_gear_joint: {
        if (cpGearJointInit(
                &constraint->m_gear,
                constraint->m_body_a ? &constraint->m_body_a->m_body : cpSpaceGetStaticBody(space),
                constraint->m_body_b ? &constraint->m_body_b->m_body : cpSpaceGetStaticBody(space),
                constraint->m_constraint_data.gear_joint.phase, constraint->m_constraint_data.gear_joint.ratio)
            == NULL)
        {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_constraint_add_to_space: init gear constraint fail!");
            return -1;
        }
        
        break;
    }
    case chipmunk_constraint_type_ratchet_joint: {
        if (cpRatchetJointInit(
                &constraint->m_ratchet,
                constraint->m_body_a ? &constraint->m_body_a->m_body : cpSpaceGetStaticBody(space),
                constraint->m_body_b ? &constraint->m_body_b->m_body : cpSpaceGetStaticBody(space),
                constraint->m_constraint_data.ratchet_joint.phase, constraint->m_constraint_data.ratchet_joint.ratio)
            == NULL)
        {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_constraint_add_to_space: init ratchet constraint fail!");
            return -1;
        }
        break;
    }
    case chipmunk_constraint_type_rotary_limit_joint: {
        if (cpRotaryLimitJointInit(
                &constraint->m_rotary_limit,
                constraint->m_body_a ? &constraint->m_body_a->m_body : cpSpaceGetStaticBody(space),
                constraint->m_body_b ? &constraint->m_body_b->m_body : cpSpaceGetStaticBody(space),
                constraint->m_constraint_data.rotary_limit_joint.min, constraint->m_constraint_data.rotary_limit_joint.max)
            == NULL)
        {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_constraint_add_to_space: init rotary_limit constraint fail!");
            return -1;
        }
        break;
    }
    case chipmunk_constraint_type_fix_rotation_joint: {
        cpBody * body_a = constraint->m_body_a ? &constraint->m_body_a->m_body : cpSpaceGetStaticBody(space);
        cpBody * body_b = constraint->m_body_b ? &constraint->m_body_b->m_body : cpSpaceGetStaticBody(space);
        float fix_rotation = cpBodyGetAngle(body_b) - cpBodyGetAngle(body_a);
        if (cpRotaryLimitJointInit(&constraint->m_rotary_limit, body_a, body_b, fix_rotation, fix_rotation) == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_constraint_add_to_space: init fix rotation constraint fail!");
            return -1;
        }
        break;
    }
    case chipmunk_constraint_type_simple_motor: {
        if (cpSimpleMotorInit(
                &constraint->m_simple_motor,
                constraint->m_body_a ? &constraint->m_body_a->m_body : cpSpaceGetStaticBody(space),
                constraint->m_body_b ? &constraint->m_body_b->m_body : cpSpaceGetStaticBody(space),
                constraint->m_constraint_data.simple_motor.rate)
            == NULL)
        {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_constraint_add_to_space: init simple_motor constraint fail!");
            return -1;
        }

        break;
    }
    default:
        CPE_ERROR(
            module->m_em, "ui_sprite_chipmunk_obj_constraint_add_to_space: unknown constraint type %d!",
            constraint->m_constraint_type);
        return -1;
    }

    if (constraint->m_error_bias) {
        cpConstraintSetErrorBias(&constraint->m_rotary_limit.constraint, constraint->m_error_bias);
    }

	if (constraint->m_max_force) {
		cpConstraintSetMaxForce(&constraint->m_rotary_limit.constraint, constraint->m_max_force);
	}

	if (constraint->m_max_bias) {
		cpConstraintSetMaxBias(&constraint->m_rotary_limit.constraint, constraint->m_max_bias);
	}

    cpSpaceAddConstraint(space, &constraint->m_rotary_limit.constraint);
    constraint->m_is_in_space = 1;

    return 0;
}

void ui_sprite_chipmunk_obj_constraint_remove_from_space(ui_sprite_chipmunk_obj_constraint_t obj_constraint)  {
    cpConstraint * constraint;

    assert(obj_constraint->m_is_in_space);

    constraint = &obj_constraint->m_rotary_limit.constraint;
    
    if (constraint->space) {
        cpSpaceRemoveConstraint(constraint->space, constraint);
        assert(constraint->space == NULL);
    }

    cpConstraintDestroy(constraint);

    obj_constraint->m_is_in_space = 0;
}

#ifdef __cplusplus
}
#endif

