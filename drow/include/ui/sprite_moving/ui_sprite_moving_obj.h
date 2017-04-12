#ifndef UI_SPRITE_MOVING_OBJ_H
#define UI_SPRITE_MOVING_OBJ_H
#include "cpe/cfg/cfg_types.h"
#include "ui_sprite_moving_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_MOVING_OBJ_NAME;

ui_sprite_moving_obj_t ui_sprite_moving_obj_create(ui_sprite_entity_t entity);
ui_sprite_moving_obj_t ui_sprite_moving_obj_find(ui_sprite_entity_t entity);
void ui_sprite_moving_obj_free(ui_sprite_moving_obj_t moving_obj);

uint8_t ui_sprite_moving_obj_is_suspend(ui_sprite_moving_obj_t obj);
void ui_sprite_moving_obj_set_is_suspend(ui_sprite_moving_obj_t obj, uint8_t is_suspend);

float ui_sprite_moving_obj_time_scale(ui_sprite_moving_obj_t obj);
void ui_sprite_moving_obj_set_tile_scale(ui_sprite_moving_obj_t obj, float time_scale);
    
typedef void (*ui_sprite_moving_obj_node_destory_fun_t)(void * ctx, plugin_moving_node_t moving_node);

int ui_sprite_moving_obj_push_node(
    ui_sprite_moving_obj_t obj, plugin_moving_node_t moving_node,
    void * ctx, ui_sprite_moving_obj_node_destory_fun_t destory);
    
int ui_sprite_moving_obj_pop_node(ui_sprite_moving_obj_t obj, plugin_moving_node_t moving_node);
void ui_sprite_moving_obj_remove_node_by_ctx(ui_sprite_moving_obj_t obj, void * ctx);
    
int ui_sprite_moving_obj_destory_node(ui_sprite_moving_obj_t obj, plugin_moving_node_t moving_node);
void ui_sprite_moving_obj_destory_node_by_ctx(ui_sprite_moving_obj_t obj, void * ctx);
    
#ifdef __cplusplus
}
#endif

#endif
