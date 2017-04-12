#ifndef UI_SPRITE_SPINE_TYPES_H
#define UI_SPRITE_SPINE_TYPES_H
#include "plugin/spine/plugin_spine_types.h"
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "ui/sprite_render/ui_sprite_render_types.h"
#include "ui/sprite_tri/ui_sprite_tri_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_spine_module * ui_sprite_spine_module_t;
typedef struct ui_sprite_spine_controled_obj * ui_sprite_spine_controled_obj_t;
typedef struct ui_sprite_spine_play_anim * ui_sprite_spine_play_anim_t;
typedef struct ui_sprite_spine_schedule_state * ui_sprite_spine_schedule_state_t;
typedef struct ui_sprite_spine_apply_transition * ui_sprite_spine_apply_transition_t;
typedef struct ui_sprite_spine_guard_transition * ui_sprite_spine_guard_transition_t;
typedef struct ui_sprite_spine_set_state * ui_sprite_spine_set_state_t;
typedef struct ui_sprite_spine_bind_parts * ui_sprite_spine_bind_parts_t;
typedef struct ui_sprite_spine_control_entity * ui_sprite_spine_control_entity_t;    
typedef struct ui_sprite_spine_follow_parts * ui_sprite_spine_follow_parts_t;
typedef struct ui_sprite_spine_ik_restore * ui_sprite_spine_ik_restore_t;    
typedef struct ui_sprite_spine_with_obj * ui_sprite_spine_with_obj_t;
typedef struct ui_sprite_spine_move_entity * ui_sprite_spine_move_entity_t;

typedef struct ui_sprite_spine_tri_on_part_state * ui_sprite_spine_tri_on_part_state_t;
typedef struct ui_sprite_spine_tri_apply_transition * ui_sprite_spine_tri_apply_transition_t;
typedef struct ui_sprite_spine_tri_set_timescale * ui_sprite_spine_tri_set_timescale_t;
    
#ifdef __cplusplus
}
#endif

#endif
