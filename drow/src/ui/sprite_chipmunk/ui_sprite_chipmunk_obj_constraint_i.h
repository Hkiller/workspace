#ifndef UI_SPRITE_CHIPMUNK_OBJ_CONSTRAINT_I_H
#define UI_SPRITE_CHIPMUNK_OBJ_CONSTRAINT_I_H
#include "chipmunk/chipmunk_private.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_obj_constraint.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_load_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_obj_constraint {
    ui_sprite_chipmunk_obj_body_t m_body_a;
    TAILQ_ENTRY(ui_sprite_chipmunk_obj_constraint) m_next_for_body_a;
    ui_sprite_chipmunk_obj_body_t m_body_b;
    TAILQ_ENTRY(ui_sprite_chipmunk_obj_constraint) m_next_for_body_b;
    float m_fix_per_step;
    void * m_creator;
    uint8_t m_is_runtime;
    uint8_t m_is_in_space;
    float m_error_bias;
	float m_max_force;
	float m_max_bias;
    char m_name[64];
    uint8_t m_constraint_type;
    CHIPMUNK_CONSTRAINT_DATA m_constraint_data;
    union {
        cpGearJoint m_gear;
        cpGrooveJoint m_groove;
        cpPinJoint m_pin;
        cpPivotJoint m_pivot;
        cpRatchetJoint m_ratchet;
        cpRotaryLimitJoint m_rotary_limit;
        cpSlideJoint m_slide;
		cpSimpleMotor m_simple_motor;
    };
};

int ui_sprite_chipmunk_obj_constraint_add_to_space(ui_sprite_chipmunk_obj_constraint_t constraint, cpSpace * space);

void ui_sprite_chipmunk_obj_constraint_remove_from_space(ui_sprite_chipmunk_obj_constraint_t constraint);

#ifdef __cplusplus
}
#endif

#endif
