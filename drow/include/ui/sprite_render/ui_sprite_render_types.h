#ifndef RENDER_SPRITE_RENDER_TYPES_H
#define RENDER_SPRITE_RENDER_TYPES_H
#include "render/runtime/ui_runtime_types.h"
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_render_module * ui_sprite_render_module_t;
typedef struct ui_sprite_render_env * ui_sprite_render_env_t;
typedef struct ui_sprite_render_layer * ui_sprite_render_layer_t;
typedef struct ui_sprite_render_layer_it * ui_sprite_render_layer_it_t;
typedef struct ui_sprite_render_sch * ui_sprite_render_sch_t;
typedef struct ui_sprite_render_group * ui_sprite_render_group_t;
typedef struct ui_sprite_render_group_it * ui_sprite_render_group_it_t;
typedef struct ui_sprite_render_def * ui_sprite_render_def_t;
typedef struct ui_sprite_render_def_it * ui_sprite_render_def_it_t;
typedef struct ui_sprite_render_anim * ui_sprite_render_anim_t;
typedef struct ui_sprite_render_anim_it * ui_sprite_render_anim_it_t;

typedef struct ui_sprite_render_show_animation * ui_sprite_render_show_animation_t;
typedef struct ui_sprite_render_with_obj * ui_sprite_render_with_obj_t;    
typedef struct ui_sprite_render_change_second_color * ui_sprite_render_change_second_color_t;
typedef struct ui_sprite_render_suspend * ui_sprite_render_suspend_t;    
typedef struct ui_sprite_render_resume * ui_sprite_render_resume_t;    
typedef struct ui_sprite_render_lock_on_screen * ui_sprite_render_lock_on_screen_t;
typedef struct ui_sprite_render_action_adj_priority * ui_sprite_render_action_adj_priority_t;
typedef struct ui_sprite_render_action_obj_bind_value * ui_sprite_render_action_obj_bind_value_t;    
typedef struct ui_sprite_render_action_obj_alpha_out * ui_sprite_render_action_obj_alpha_out_t;
typedef struct ui_sprite_render_action_obj_alpha_in * ui_sprite_render_action_obj_alpha_in_t;

typedef struct ui_sprite_render_obj_world * ui_sprite_render_obj_world_t;
typedef struct ui_sprite_render_obj_entity * ui_sprite_render_obj_entity_t;

typedef int (*ui_sprite_render_obj_create_fun_t)(
    ui_runtime_render_obj_ref_t * r, void * ctx, ui_sprite_world_t world, uint32_t entity_id, const char * res);

typedef int (*ui_sprite_render_env_touch_process_fun_t)(
    void * ctx, ui_sprite_render_env_t render_env,
    uint32_t track_id, ui_runtime_touch_event_t evt, ui_vector_2_t screen_pt, ui_vector_2_t logic_pt);

typedef void (*ui_sprite_render_env_transform_monitor_fun_t)(void * ctx, ui_sprite_render_env_t render_env);
    
#ifdef __cplusplus
}
#endif

#endif
