#ifndef UI_SPRITE_RENDER_ACTION_OBJ_ALPHA_OUT_H
#define UI_SPRITE_RENDER_ACTION_OBJ_ALPHA_OUT_H
#include "ui_sprite_render_types.h"
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_RENDER_ACTION_OBJ_ALPHA_OUT;

ui_sprite_render_action_obj_alpha_out_t ui_sprite_render_action_obj_alpha_out_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_render_action_obj_alpha_out_free(ui_sprite_render_action_obj_alpha_out_t action_obj_alpha_out);

#ifdef __cplusplus
}
#endif

#endif
