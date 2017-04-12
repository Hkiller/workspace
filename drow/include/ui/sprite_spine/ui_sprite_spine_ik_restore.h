#ifndef UI_SPRITE_SPINE_IK_RESTORE_H
#define UI_SPRITE_SPINE_IK_RESTORE_H
#include "ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_IK_RESTORE_NAME;

ui_sprite_spine_ik_restore_t ui_sprite_spine_ik_restore_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_spine_ik_restore_free(ui_sprite_spine_ik_restore_t ik_restore);

#ifdef __cplusplus
}
#endif

#endif
