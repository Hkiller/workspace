#ifndef UI_SPRITE_TRI_OBJ_H
#define UI_SPRITE_TRI_OBJ_H
#include "ui_sprite_tri_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_TRI_OBJ_NAME;

ui_sprite_tri_obj_t ui_sprite_tri_obj_create(ui_sprite_entity_t entity);
ui_sprite_tri_obj_t ui_sprite_tri_obj_find(ui_sprite_entity_t entity);
void ui_sprite_tri_obj_free(ui_sprite_tri_obj_t tri_obj);

ui_sprite_tri_module_t ui_sprite_tri_obj_module(ui_sprite_tri_obj_t tri_obj);

#ifdef __cplusplus
}
#endif

#endif
