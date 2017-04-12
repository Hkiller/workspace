#ifndef UI_SPRITE_CTRL_TURNTABLE_H
#define UI_SPRITE_CTRL_TURNTABLE_H
#include "gd/app/app_types.h"
#include "ui_sprite_ctrl_types.h"
#include "protocol/ui/sprite_ctrl/ui_sprite_ctrl_turntable.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CTRL_TURNTABLE_NAME;

ui_sprite_ctrl_turntable_t ui_sprite_ctrl_turntable_create(ui_sprite_entity_t entity);
ui_sprite_ctrl_turntable_t ui_sprite_ctrl_turntable_find(ui_sprite_entity_t entity);
void ui_sprite_ctrl_turntable_free(ui_sprite_ctrl_turntable_t ctrl);

int ui_sprite_ctrl_turntable_add_member(ui_sprite_ctrl_turntable_t turntable, ui_sprite_ctrl_turntable_member_t member);
void ui_sprite_ctrl_turntable_remove_member(ui_sprite_ctrl_turntable_t turntable, ui_sprite_ctrl_turntable_member_t member);

void ui_sprite_ctrl_turntable_set_focuse_member(ui_sprite_ctrl_turntable_t turntable, ui_sprite_ctrl_turntable_member_t member);

UI_SPRITE_CTRL_TURNTABLE_DEF const * ui_sprite_ctrl_turntable_def(ui_sprite_ctrl_turntable_t turntable);
int ui_sprite_ctrl_turntable_set_def(ui_sprite_ctrl_turntable_t turntable, UI_SPRITE_CTRL_TURNTABLE_DEF const * def);

#ifdef __cplusplus
}
#endif

#endif
