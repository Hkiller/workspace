#ifndef RENDER_SPRITE_RENDER_UTILS_H
#define RENDER_SPRITE_RENDER_UTILS_H
#include "ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_render_anim_t ui_sprite_render_anim_find_on_entity_by_id(ui_sprite_entity_t entity, uint32_t anim_id);
ui_sprite_render_anim_t ui_sprite_render_anim_find_on_entity_by_name(ui_sprite_entity_t entity, const char * anim_name);
    
ui_runtime_render_obj_ref_t
ui_sprite_render_find_entity_render_obj(
    ui_sprite_render_module_t module, ui_sprite_entity_t entity, const char * name);

ui_runtime_render_obj_t
ui_sprite_render_find_render_obj(
    ui_sprite_render_module_t module, ui_sprite_entity_t entity, const char * name);

ui_runtime_render_obj_ref_t
ui_sprite_render_find_render_obj_create_ref(
    ui_sprite_render_module_t module, ui_sprite_entity_t entity, const char * name);
    
ui_runtime_render_obj_ref_t
ui_sprite_render_find_entity_render_obj_with_type(
    ui_sprite_render_module_t module, ui_sprite_entity_t entity, const char * name, uint8_t obj_type);
    
#ifdef __cplusplus
}
#endif

#endif
