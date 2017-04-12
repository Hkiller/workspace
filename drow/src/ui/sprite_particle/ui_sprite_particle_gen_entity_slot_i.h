#ifndef UI_SPRITE_CHIPMUNK_CHIPMUNK_BODY_I_H
#define UI_SPRITE_CHIPMUNK_CHIPMUNK_BODY_I_H
#include "ui_sprite_particle_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_particle_gen_entity_slot {
    ui_sprite_particle_gen_entity_t m_gen_entity;
    ui_sprite_particle_controled_obj_t m_controled_obj;
};

int ui_sprite_particle_gen_entity_slot_init(void * ctx, plugin_particle_obj_plugin_data_t data);
void ui_sprite_particle_gen_entity_slot_fini(void * ctx, plugin_particle_obj_plugin_data_t data);
void ui_sprite_particle_gen_entity_slot_update(void * ctx, plugin_particle_obj_plugin_data_t data);

void ui_sprite_particle_gen_entity_slot_free(ui_sprite_particle_gen_entity_slot_t slot);
    
#ifdef __cplusplus
}
#endif

#endif
