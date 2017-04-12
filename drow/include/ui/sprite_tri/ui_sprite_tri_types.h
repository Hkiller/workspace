#ifndef UI_SPRITE_TRI_TYPES_H
#define UI_SPRITE_TRI_TYPES_H
#include "ui/sprite/ui_sprite_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_sprite_tri_action_trigger {
    ui_sprite_tri_action_on_active,
    ui_sprite_tri_action_on_deactive,
    ui_sprite_tri_action_in_active
} ui_sprite_tri_action_trigger_t;

typedef struct ui_sprite_tri_module * ui_sprite_tri_module_t;
typedef struct ui_sprite_tri_obj * ui_sprite_tri_obj_t;
typedef struct ui_sprite_tri_rule * ui_sprite_tri_rule_t;
typedef struct ui_sprite_tri_action * ui_sprite_tri_action_t;
typedef struct ui_sprite_tri_action_meta * ui_sprite_tri_action_meta_t;
typedef struct ui_sprite_tri_condition * ui_sprite_tri_condition_t;
typedef struct ui_sprite_tri_condition_meta * ui_sprite_tri_condition_meta_t;
typedef struct ui_sprite_tri_trigger * ui_sprite_tri_trigger_t;

typedef struct ui_sprite_tri_action_remove_self * ui_sprite_tri_action_remove_self_t;
typedef struct ui_sprite_tri_action_send_event * ui_sprite_tri_action_send_event_t;    

#ifdef __cplusplus
}
#endif

#endif
