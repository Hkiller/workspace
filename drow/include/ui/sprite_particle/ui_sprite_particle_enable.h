#ifndef UI_SPRITE_PARTICLE_ENABLE_H
#define UI_SPRITE_PARTICLE_ENABLE_H
#include "ui_sprite_particle_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_PARTICLE_ENABLE_NAME;

ui_sprite_particle_enable_t ui_sprite_particle_enable_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_particle_enable_free(ui_sprite_particle_enable_t particle_enable_emitter);

#ifdef __cplusplus
}
#endif

#endif
