#ifndef UI_SPRITE_CHIPMUNK_WITH_GRAVITY_H
#define UI_SPRITE_CHIPMUNK_WITH_GRAVITY_H
#include "ui_sprite_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CHIPMUNK_WITH_GRAVITY_NAME;

ui_sprite_chipmunk_with_gravity_t ui_sprite_chipmunk_with_gravity_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_chipmunk_with_gravity_free(ui_sprite_chipmunk_with_gravity_t send_evt);

#ifdef __cplusplus
}
#endif

#endif
