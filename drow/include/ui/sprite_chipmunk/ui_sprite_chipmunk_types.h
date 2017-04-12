#ifndef UI_SPRITE_CHIPMUNK_TYPES_H
#define UI_SPRITE_CHIPMUNK_TYPES_H
#include "protocol/ui/sprite_chipmunk/ui_sprite_chipmunk_data.h"
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_2d/ui_sprite_2d_types.h"
#include "plugin/chipmunk/plugin_chipmunk_types.h"
#include "ui/sprite_render/ui_sprite_render_types.h"
#include "ui/sprite_tri/ui_sprite_tri_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_sprite_chipmunk_runing_mode {
    ui_sprite_chipmunk_runing_mode_passive = UI_SPRITE_CHIPMUNK_OBJ_RUNING_MODE_PASSIVE,
    ui_sprite_chipmunk_runing_mode_active = UI_SPRITE_CHIPMUNK_OBJ_RUNING_MODE_ACTIVE
} ui_sprite_chipmunk_runing_mode_t;

typedef enum ui_sprite_chipmunk_unit {
    ui_sprite_chipmunk_unit_unknown = 0,
    ui_sprite_chipmunk_unit_logic = 1,
    ui_sprite_chipmunk_unit_pixel = 2,
} ui_sprite_chipmunk_unit_t;

typedef struct ui_sprite_chipmunk_module * ui_sprite_chipmunk_module_t;
typedef struct ui_sprite_chipmunk_env * ui_sprite_chipmunk_env_t;
typedef struct ui_sprite_chipmunk_obj * ui_sprite_chipmunk_obj_t;
typedef struct ui_sprite_chipmunk_obj_updator * ui_sprite_chipmunk_obj_updator_t;
typedef struct ui_sprite_chipmunk_obj_body * ui_sprite_chipmunk_obj_body_t;
typedef struct ui_sprite_chipmunk_obj_body_it * ui_sprite_chipmunk_obj_body_it_t;
typedef struct ui_sprite_chipmunk_obj_shape_group * ui_sprite_chipmunk_obj_shape_group_t;
typedef struct ui_sprite_chipmunk_obj_shape * ui_sprite_chipmunk_obj_shape_t;
typedef struct ui_sprite_chipmunk_obj_shape_node_buf * ui_sprite_chipmunk_obj_shape_node_buf_t;
typedef struct ui_sprite_chipmunk_obj_constraint * ui_sprite_chipmunk_obj_constraint_t;
typedef struct ui_sprite_chipmunk_obj_constraint_it * ui_sprite_chipmunk_obj_constraint_it_t;

typedef struct ui_sprite_chipmunk_with_time_scale * ui_sprite_chipmunk_with_time_scale_t;
typedef struct ui_sprite_chipmunk_with_collision * ui_sprite_chipmunk_with_collision_t;
typedef struct ui_sprite_chipmunk_with_boundary * ui_sprite_chipmunk_with_boundary_t;
typedef struct ui_sprite_chipmunk_with_group * ui_sprite_chipmunk_with_group_t;
typedef struct ui_sprite_chipmunk_with_gravity * ui_sprite_chipmunk_with_gravity_t;
typedef struct ui_sprite_chipmunk_with_damping * ui_sprite_chipmunk_with_damping_t;
typedef struct ui_sprite_chipmunk_with_runing_mode * ui_sprite_chipmunk_with_runing_mode_t;
typedef struct ui_sprite_chipmunk_with_addition_accel * ui_sprite_chipmunk_with_addition_accel_t;
typedef struct ui_sprite_chipmunk_with_constraint * ui_sprite_chipmunk_with_constraint_t;    
typedef struct ui_sprite_chipmunk_with_attractor * ui_sprite_chipmunk_with_attractor_t;    
typedef struct ui_sprite_chipmunk_on_collision * ui_sprite_chipmunk_on_collision_t;
typedef struct ui_sprite_chipmunk_on_click * ui_sprite_chipmunk_on_click_t;
typedef struct ui_sprite_chipmunk_wait_collision * ui_sprite_chipmunk_wait_collision_t;
typedef struct ui_sprite_chipmunk_manipulator * ui_sprite_chipmunk_manipulator_t;
typedef struct ui_sprite_chipmunk_track_angle * ui_sprite_chipmunk_track_angle_t;
typedef struct ui_sprite_chipmunk_apply_velocity * ui_sprite_chipmunk_apply_velocity_t;
typedef struct ui_sprite_chipmunk_move_to_entity * ui_sprite_chipmunk_move_to_entity_t;
typedef struct ui_sprite_chipmunk_send_event_to_collision * ui_sprite_chipmunk_send_event_to_collision_t;
    
typedef struct ui_sprite_chipmunk_touch_move * ui_sprite_chipmunk_touch_move_t;

/*tri*/
typedef struct ui_sprite_chipmunk_tri_scope * ui_sprite_chipmunk_tri_scope_t;
typedef struct ui_sprite_chipmunk_tri_scope_render * ui_sprite_chipmunk_tri_scope_render_t;

typedef struct ui_sprite_chipmunk_tri_have_entity * ui_sprite_chipmunk_tri_have_entity_t;

/*for with_collision*/
typedef struct ui_sprite_chipmunk_with_collision_src * ui_sprite_chipmunk_with_collision_src_t;

typedef void (*ui_sprite_chipmunk_obj_shape_visit_fun_t)(ui_sprite_chipmunk_obj_shape_t shape, void * ctx);

typedef void (*ui_sprite_chipmunk_obj_updateor_update_fun_t)(
    ui_sprite_chipmunk_obj_updator_t updator, ui_sprite_chipmunk_obj_body_t body,
    UI_SPRITE_CHIPMUNK_PAIR * acc, float * damping);
typedef void (*ui_sprite_chipmunk_obj_updateor_clean_fun_t)(ui_sprite_chipmunk_obj_updator_t updator);

typedef void (*ui_sprite_chipmunk_obj_body_visit_fun_t)(ui_sprite_chipmunk_env_t env, ui_sprite_chipmunk_obj_body_t body, void * ctx);
    
#ifdef __cplusplus
}
#endif

#endif
