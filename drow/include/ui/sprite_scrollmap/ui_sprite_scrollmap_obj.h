#ifndef UI_SPRITE_SCROLLMAP_OBJ_H
#define UI_SPRITE_SCROLLMAP_OBJ_H
#include "ui_sprite_scrollmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SCROLLMAP_OBJ_NAME;

ui_sprite_scrollmap_obj_t ui_sprite_scrollmap_obj_create(ui_sprite_entity_t entity);
ui_sprite_scrollmap_obj_t ui_sprite_scrollmap_obj_find(ui_sprite_entity_t entity);
void ui_sprite_scrollmap_obj_free(ui_sprite_scrollmap_obj_t scrollmap_obj);

uint8_t ui_sprite_scrollmap_obj_is_move_suspend(ui_sprite_scrollmap_obj_t scrollmap_obj);
void ui_sprite_scrollmap_obj_set_move_suspend(ui_sprite_scrollmap_obj_t scrollmap_obj, uint8_t is_suspend);
    
#ifdef __cplusplus
}
#endif

#endif
