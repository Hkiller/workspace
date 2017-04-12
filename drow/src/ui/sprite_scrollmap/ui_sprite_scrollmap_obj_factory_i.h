#ifndef UI_SPRITE_SCROLLMAP_OBJ_FACTORY_I_H
#define UI_SPRITE_SCROLLMAP_OBJ_FACTORY_I_H
#include "ui_sprite_scrollmap_obj_i.h"

#ifdef __cplusplus
extern "C" {
#endif

const char * ui_sprite_scrollmap_obj_name(void * ctx, plugin_scrollmap_obj_t obj);
int ui_sprite_scrollmap_obj_on_init(void * ctx, plugin_scrollmap_obj_t obj, const char * obj_type, const char * args);
void ui_sprite_scrollmap_obj_on_update(void * ctx, plugin_scrollmap_obj_t obj);
void ui_sprite_scrollmap_obj_on_event(void * ctx, plugin_scrollmap_obj_t obj, const char * event);
void ui_sprite_scrollmap_obj_on_destory(void * ctx, plugin_scrollmap_obj_t obj);
    
#ifdef __cplusplus
}
#endif

#endif
