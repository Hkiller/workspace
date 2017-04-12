#ifndef UI_SPRITE_RENDER_ACTION_ACTION_OBJ_BIND_VALUE_H
#define UI_SPRITE_RENDER_ACTION_ACTION_OBJ_BIND_VALUE_H
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_RENDER_ACTION_OBJ_BIND_VALUE;

ui_sprite_render_action_obj_bind_value_t ui_sprite_render_action_obj_bind_value_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_render_action_obj_bind_value_free(ui_sprite_render_action_obj_bind_value_t action_obj_bind_value);

#ifdef __cplusplus
}
#endif

#endif
