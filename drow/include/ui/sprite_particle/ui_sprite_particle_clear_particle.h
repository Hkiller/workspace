#ifndef UI_SPRITE_PARTICLE_CLEAR_PARTICLE_H
#define UI_SPRITE_PARTICLE_CLEAR_PARTICLE_H
#include "ui_sprite_particle_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_PARTICLE_CLEAR_PARTICLE_NAME;

ui_sprite_particle_clear_particle_t ui_sprite_particle_clear_particle_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_particle_clear_particle_free(ui_sprite_particle_clear_particle_t particle_clear_particle_emitter);

#ifdef __cplusplus
}
#endif

#endif
