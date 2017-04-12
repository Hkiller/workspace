#ifndef UI_SPRITE_PARTICLE_BIND_EMITTE_PART_H
#define UI_SPRITE_PARTICLE_BIND_EMITTE_PART_H
#include "ui_sprite_particle_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_PARTICLE_BIND_EMITTER_PART_NAME;

ui_sprite_particle_bind_emitter_part_t ui_sprite_particle_bind_emitter_part_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_particle_bind_emitter_part_free(ui_sprite_particle_bind_emitter_part_t particle_bind_emitter_part);

#ifdef __cplusplus
}
#endif

#endif
