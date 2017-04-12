#ifndef UI_SPRITE_UI_ACTION_CONTROL_ANIM_H
#define UI_SPRITE_UI_ACTION_CONTROL_ANIM_H
#include "ui_sprite_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_UI_ACTION_CONTROL_ANIM_NAME;

ui_sprite_ui_action_control_anim_t ui_sprite_ui_action_control_anim_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_ui_action_control_anim_free(ui_sprite_ui_action_control_anim_t control_anim);

int ui_sprite_ui_action_control_anim_set_control(ui_sprite_ui_action_control_anim_t action_control_anim, const char * control);
const char * ui_sprite_ui_action_control_anim_control(ui_sprite_ui_action_control_anim_t action_control_anim);

int ui_sprite_ui_action_control_anim_set_anim(ui_sprite_ui_action_control_anim_t action_control_anim, const char * anim);
const char * ui_sprite_ui_action_control_anim_anim(ui_sprite_ui_action_control_anim_t action_control_anim);

int ui_sprite_ui_action_control_anim_set_init(ui_sprite_ui_action_control_anim_t action_control_anim, const char * init);
const char * ui_sprite_ui_action_control_anim_init(ui_sprite_ui_action_control_anim_t action_control_anim);
    
#ifdef __cplusplus
}
#endif

#endif
