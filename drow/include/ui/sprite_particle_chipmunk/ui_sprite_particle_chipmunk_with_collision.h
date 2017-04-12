#ifndef UI_SPRITE_PARTICLE_CHIPMUNK_WITH_COLLISION_H
#define UI_SPRITE_PARTICLE_CHIPMUNK_WITH_COLLISION_H
#include "ui_sprite_particle_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_PARTICLE_WITH_COLLISION_NAME;

ui_sprite_particle_chipmunk_with_collision_t
ui_sprite_particle_chipmunk_with_collision_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_particle_chipmunk_with_collision_free(ui_sprite_particle_chipmunk_with_collision_t send_evt);

#ifdef __cplusplus
}
#endif

#endif
