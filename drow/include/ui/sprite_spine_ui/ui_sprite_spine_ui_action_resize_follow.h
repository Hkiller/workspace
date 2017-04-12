#ifndef UI_SPRITE_SPINE_UI_ACTION_RESIZE_H
#define UI_SPRITE_SPINE_UI_ACTION_RESIZE_H
#include "ui_sprite_spine_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_UI_ACTION_RESIZE_FOLLOW_NAME;

ui_sprite_spine_ui_action_resize_follow_t ui_sprite_spine_ui_action_resize_follow_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_spine_ui_action_resize_follow_free(ui_sprite_spine_ui_action_resize_follow_t resize_follow);

int ui_sprite_spine_ui_action_resize_follow_set_control(ui_sprite_spine_ui_action_resize_follow_t resize_follow, const char * control);
int ui_sprite_spine_ui_action_resize_follow_set_res(ui_sprite_spine_ui_action_resize_follow_t resize_follow, const char * res);    

#ifdef __cplusplus
}
#endif

#endif
