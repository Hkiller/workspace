#ifndef UI_SPRITE_UI_TYPES_H
#define UI_SPRITE_UI_TYPES_H
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "plugin/ui/plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_ui_module * ui_sprite_ui_module_t;
typedef struct ui_sprite_ui_env * ui_sprite_ui_env_t;
typedef struct ui_sprite_ui_popup_def * ui_sprite_ui_popup_def_t;
typedef struct ui_sprite_ui_phase * ui_sprite_ui_phase_t;
typedef struct ui_sprite_ui_state * ui_sprite_ui_state_t;
typedef struct ui_sprite_ui_navigation * ui_sprite_ui_navigation_t;

typedef struct ui_sprite_ui_action_navigation * ui_sprite_ui_action_navigation_t;
typedef struct ui_sprite_ui_action_show_page * ui_sprite_ui_action_show_page_t;
typedef struct ui_sprite_ui_action_show_popup * ui_sprite_ui_action_show_popup_t;    
typedef struct ui_sprite_ui_action_play_anim * ui_sprite_ui_action_play_anim_t;    
typedef struct ui_sprite_ui_action_send_event * ui_sprite_ui_action_send_event_t;
typedef struct ui_sprite_ui_action_scope_value * ui_sprite_ui_action_scope_value_t;
typedef struct ui_sprite_ui_action_show_template * ui_sprite_ui_action_show_template_t;
typedef struct ui_sprite_ui_action_entity_follow_control * ui_sprite_ui_action_entity_follow_control_t;    
typedef struct ui_sprite_ui_action_phase_switch * ui_sprite_ui_action_phase_switch_t;
typedef struct ui_sprite_ui_action_phase_back * ui_sprite_ui_action_phase_back_t;    
typedef struct ui_sprite_ui_action_guard_package * ui_sprite_ui_action_guard_package_t;
    
typedef struct ui_sprite_ui_action_control_anim_bulk * ui_sprite_ui_action_control_anim_bulk_t;
typedef struct ui_sprite_ui_action_control_anim_bulk_record * ui_sprite_ui_action_control_anim_bulk_record_t;
typedef struct ui_sprite_ui_action_control_anim * ui_sprite_ui_action_control_anim_t;
    
#ifdef __cplusplus
}
#endif

#endif
