#ifndef UI_SPRITE_RENDER_OBJ_WORLD_H
#define UI_SPRITE_RENDER_OBJ_WORLD_H
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_render_env_t ui_sprite_render_obj_world_env(ui_sprite_render_obj_world_t world_obj);
void ui_sprite_render_obj_world_set_env(ui_sprite_render_obj_world_t world_obj, ui_sprite_render_env_t env);
    
uint8_t ui_sprite_render_obj_world_control_tick(ui_sprite_render_obj_world_t world_obj);
void ui_sprite_render_obj_world_set_control_tick(ui_sprite_render_obj_world_t world_obj, uint8_t control_tick);

uint8_t ui_sprite_render_obj_world_sync_transform(ui_sprite_render_obj_world_t world_obj);
void ui_sprite_render_obj_world_set_sync_transform(ui_sprite_render_obj_world_t world_obj, uint8_t sync_transform);
    
ui_sprite_world_t ui_sprite_render_obj_world_world(ui_sprite_render_obj_world_t world_obj);

#ifdef __cplusplus
}
#endif

#endif
