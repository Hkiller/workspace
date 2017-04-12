#ifndef UI_SPRITE_PARTICLE_CONTROLED_OBJ_I_H
#define UI_SPRITE_PARTICLE_CONTROLED_OBJ_I_H
#include "ui/sprite_particle/ui_sprite_particle_controled_obj.h"
#include "ui_sprite_particle_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_particle_controled_obj {
    ui_sprite_particle_module_t m_module;
    ui_sprite_particle_gen_entity_slot_t m_slot;
    uint8_t m_is_binding;
    uint8_t m_accept_scale;
    uint8_t m_accept_angle;
    uint8_t m_remove_particle;
};

int ui_sprite_particle_controled_obj_regist(ui_sprite_particle_module_t module);
void ui_sprite_particle_controled_obj_unregist(ui_sprite_particle_module_t module);

void ui_sprite_particle_controled_obj_set_slot(ui_sprite_particle_controled_obj_t obj, ui_sprite_particle_gen_entity_slot_t slot);
    
#ifdef __cplusplus
}
#endif

#endif
