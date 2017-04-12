#ifndef UI_SPRITE_SPINE_BIND_PARTS_H
#define UI_SPRITE_SPINE_BIND_PARTS_H
#include "ui_sprite_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_SPINE_BIND_PARTS_NAME;

ui_sprite_spine_bind_parts_t ui_sprite_spine_bind_parts_create(ui_sprite_fsm_state_t fsm_state, const char * name);
void ui_sprite_spine_bind_parts_free(ui_sprite_spine_bind_parts_t bind_parts);

#ifdef __cplusplus
}
#endif

#endif
