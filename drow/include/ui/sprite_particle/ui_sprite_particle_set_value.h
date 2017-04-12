#ifndef UI_SPRITE_PARTICLE_SET_VALUE_H
#define UI_SPRITE_PARTICLE_SET_VALUE_H
#include "ui_sprite_particle_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_PARTICLE_SET_VALUE_NAME;

ui_sprite_particle_set_value_t ui_sprite_particle_set_value_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_particle_set_value_free(ui_sprite_particle_set_value_t send_evt);

#ifdef __cplusplus
}
#endif

#endif
