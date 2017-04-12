#ifndef UI_SPRITE_RENDER_WITH_OBJ_H
#define UI_SPRITE_RENDER_WITH_OBJ_H
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_RENDER_WITH_OBJ_NAME;

ui_sprite_render_with_obj_t ui_sprite_render_with_obj_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_render_with_obj_free(ui_sprite_render_with_obj_t with_obj);

int ui_sprite_render_with_obj_set_obj_name(ui_sprite_render_with_obj_t with_obj, const char * obj_name);
int ui_sprite_render_with_obj_set_obj_res(ui_sprite_render_with_obj_t with_obj, const char * obj_res);    

#ifdef __cplusplus
}
#endif

#endif
