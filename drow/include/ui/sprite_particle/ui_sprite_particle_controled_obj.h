#ifndef UI_SPRITE_PARTICLE_CONTROLED_OBJ_H
#define UI_SPRITE_PARTICLE_CONTROLED_OBJ_H
#include "ui_sprite_particle_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_PARTICLE_CONTROLED_OBJ_NAME;

ui_sprite_particle_controled_obj_t ui_sprite_particle_controled_obj_create(ui_sprite_entity_t entity);
ui_sprite_particle_controled_obj_t ui_sprite_particle_controled_obj_find(ui_sprite_entity_t entity);
void ui_sprite_particle_controled_obj_free(ui_sprite_particle_controled_obj_t particle_controled_obj);

uint8_t ui_sprite_particle_controled_obj_is_binding(ui_sprite_particle_controled_obj_t obj);
void ui_sprite_particle_controled_obj_set_binding(ui_sprite_particle_controled_obj_t obj, uint8_t is_binding);
    
#ifdef __cplusplus
}
#endif

#endif
