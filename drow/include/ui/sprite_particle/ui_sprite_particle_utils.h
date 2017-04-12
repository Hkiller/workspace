#ifndef UI_SPRITE_PARTICLE_UTILS_H
#define UI_SPRITE_PARTICLE_UTILS_H
#include "plugin/particle/plugin_particle_obj.h"
#include "ui_sprite_particle_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_particle_obj_t ui_sprite_particle_find_obj(
    ui_sprite_render_module_t module, ui_sprite_entity_t entity, const char * name, error_monitor_t em);

plugin_particle_obj_t
ui_sprite_particle_find_obj_on_entity(
    ui_sprite_entity_t entity, const char * name, error_monitor_t em);
    
#ifdef __cplusplus
}
#endif

#endif
