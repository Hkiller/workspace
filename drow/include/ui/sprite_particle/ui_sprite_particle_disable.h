#ifndef UI_SPRITE_PARTICLE_DISABLE_H
#define UI_SPRITE_PARTICLE_DISABLE_H
#include "ui_sprite_particle_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_PARTICLE_DISABLE_NAME;

ui_sprite_particle_disable_t ui_sprite_particle_disable_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_particle_disable_free(ui_sprite_particle_disable_t particle_disable_emitter);

#ifdef __cplusplus
}
#endif

#endif
