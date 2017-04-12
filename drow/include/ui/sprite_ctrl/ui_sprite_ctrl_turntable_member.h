#ifndef UI_SPRITE_CTRL_TURNTABLE_MEMBER_H
#define UI_SPRITE_CTRL_TURNTABLE_MEMBER_H
#include "gd/app/app_types.h"
#include "ui_sprite_ctrl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * UI_SPRITE_CTRL_TURNTABLE_MEMBER_NAME;

ui_sprite_ctrl_turntable_member_t ui_sprite_ctrl_turntable_member_create(ui_sprite_entity_t entity);
ui_sprite_ctrl_turntable_member_t ui_sprite_ctrl_turntable_member_find(ui_sprite_entity_t entity);
void ui_sprite_ctrl_turntable_member_free(ui_sprite_ctrl_turntable_member_t ctrl);

int ui_sprite_ctrl_turntable_member_set_on_select(ui_sprite_ctrl_turntable_member_t member, const char * on_select);
const char * ui_sprite_ctrl_turntable_member_on_select(ui_sprite_ctrl_turntable_member_t member);
int ui_sprite_ctrl_turntable_member_set_on_unselect(ui_sprite_ctrl_turntable_member_t member, const char * on_unselect);
const char * ui_sprite_ctrl_turntable_member_on_unselect(ui_sprite_ctrl_turntable_member_t member);

#ifdef __cplusplus
}
#endif

#endif
