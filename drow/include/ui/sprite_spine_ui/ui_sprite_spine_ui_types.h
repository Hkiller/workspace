#ifndef UI_SPRITE_SPINE_UI_TYPES_H
#define UI_SPRITE_SPINE_UI_TYPES_H
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "ui/sprite_ui/ui_sprite_ui_types.h"
#include "ui/sprite_spine/ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_spine_ui_module * ui_sprite_spine_ui_module_t;

/*animations*/
typedef struct ui_sprite_spine_ui_anim_resize * ui_sprite_spine_ui_anim_resize_t;
typedef struct ui_sprite_spine_ui_anim_toggle * ui_sprite_spine_ui_anim_toggle_t;    
typedef struct ui_sprite_spine_ui_anim_button * ui_sprite_spine_ui_anim_button_t;
typedef struct ui_sprite_spine_ui_anim_bind * ui_sprite_spine_ui_anim_bind_t;        

/*actions*/    
typedef struct ui_sprite_spine_ui_action_resize_follow * ui_sprite_spine_ui_action_resize_follow_t;

#ifdef __cplusplus
}
#endif

#endif
