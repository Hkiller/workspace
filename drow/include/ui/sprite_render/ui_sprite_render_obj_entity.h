#ifndef UI_SPRITE_RENDER_OBJ_ENTITY_H
#define UI_SPRITE_RENDER_OBJ_ENTITY_H
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

const char * ui_sprite_render_obj_entity_world_name(ui_sprite_render_obj_entity_t entity_obj);
void ui_sprite_render_obj_entity_set_world_name(ui_sprite_render_obj_entity_t entity_obj, const char * world_name);
ui_sprite_world_t ui_sprite_render_obj_entity_world(ui_sprite_render_obj_entity_t entity_obj);    

const char * ui_sprite_render_obj_entity_entity_name(ui_sprite_render_obj_entity_t entity_obj);
void ui_sprite_render_obj_entity_set_entity_name(ui_sprite_render_obj_entity_t entity_obj, const char * entity_name);
ui_sprite_entity_t ui_sprite_render_obj_entity_entity(ui_sprite_render_obj_entity_t entity_obj);

#ifdef __cplusplus
}
#endif

#endif
