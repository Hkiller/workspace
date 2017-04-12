#ifndef UI_SPRITE_SPINE_UTILS_H
#define UI_SPRITE_SPINE_UTILS_H
#include "plugin/spine/plugin_spine_obj.h"
#include "ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_spine_obj_t ui_sprite_spine_find_obj(
    ui_sprite_render_module_t module, ui_sprite_entity_t entity, const char * name, error_monitor_t em);

plugin_spine_obj_part_t ui_sprite_spine_find_obj_part(
    ui_sprite_render_module_t module, ui_sprite_entity_t entity, const char * name, error_monitor_t em);

plugin_spine_obj_t ui_sprite_spine_find_obj_on_entity(
    ui_sprite_entity_t entity, const char * name, error_monitor_t em);
    
#ifdef __cplusplus
}
#endif

#endif
