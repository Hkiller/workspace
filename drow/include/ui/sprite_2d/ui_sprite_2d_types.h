#ifndef UI_SPRITE_2D_TYPES_H
#define UI_SPRITE_2D_TYPES_H
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_rect.h"
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "ui/sprite_cfg/ui_sprite_cfg_types.h"
#include "protocol/ui/sprite_2d/ui_sprite_2d_data.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_2d_module * ui_sprite_2d_module_t;
typedef struct ui_sprite_2d_transform * ui_sprite_2d_transform_t;
typedef struct ui_sprite_2d_part * ui_sprite_2d_part_t;
typedef struct ui_sprite_2d_part_it * ui_sprite_2d_part_it_t;
typedef struct ui_sprite_2d_part_binding * ui_sprite_2d_part_binding_t;
typedef struct ui_sprite_2d_part_attr * ui_sprite_2d_part_attr_t;
typedef struct ui_sprite_2d_part_attr_it * ui_sprite_2d_part_attr_it_t;

/*actions*/
typedef struct ui_sprite_2d_move * ui_sprite_2d_move_t;
typedef struct ui_sprite_2d_rotate * ui_sprite_2d_rotate_t;    
typedef struct ui_sprite_2d_scale * ui_sprite_2d_scale_t;
typedef struct ui_sprite_2d_flip * ui_sprite_2d_flip_t;
typedef struct ui_sprite_2d_track_flip * ui_sprite_2d_track_flip_t;
typedef struct ui_sprite_2d_track_angle * ui_sprite_2d_track_angle_t;
typedef struct ui_sprite_2d_wait_switchback * ui_sprite_2d_wait_switchback_t;
typedef struct ui_sprite_2d_wait_stop * ui_sprite_2d_wait_stop_t;
typedef struct ui_sprite_2d_search * ui_sprite_2d_search_t;
typedef struct ui_sprite_2d_wait_move_distance * ui_sprite_2d_wait_move_distance_t;
typedef struct ui_sprite_2d_action_part_follow * ui_sprite_2d_action_part_follow_t;

#ifdef __cplusplus
}
#endif

#endif


