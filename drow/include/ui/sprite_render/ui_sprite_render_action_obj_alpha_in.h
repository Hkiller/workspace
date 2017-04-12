#ifndef UI_SPRITE_RENDER_ACTION_OBJ_ALPHA_IN_H
#define UI_SPRITE_RENDER_ACTION_OBJ_ALPHA_IN_H
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_RENDER_ACTION_OBJ_ALPHA_IN;

ui_sprite_render_action_obj_alpha_in_t ui_sprite_render_action_obj_alpha_in_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_render_action_obj_alpha_in_free(ui_sprite_render_action_obj_alpha_in_t action_obj_alpha_in);

#ifdef __cplusplus
}
#endif

#endif
