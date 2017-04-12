#ifndef UI_SPRITE_SPINE_FOLLOW_PARTS_H
#define UI_SPRITE_SPINE_FOLLOW_PARTS_H
#include "ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_FOLLOW_PARTS_NAME;

ui_sprite_spine_follow_parts_t ui_sprite_spine_follow_parts_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_spine_follow_parts_free(ui_sprite_spine_follow_parts_t follow_parts);

#ifdef __cplusplus
}
#endif

#endif
